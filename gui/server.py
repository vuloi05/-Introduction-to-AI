"""
Gomoku Web GUI — Backend Server
Flask + Flask-SocketIO server that communicates with Gomoku.exe via subprocess.
"""

import os
import subprocess
import threading
import time

from flask import Flask, request, send_from_directory
from flask_socketio import SocketIO, emit

# ---------------------------------------------------------------------------
# App setup
# ---------------------------------------------------------------------------
app = Flask(__name__, static_folder=".", template_folder=".")
app.config["SECRET_KEY"] = "gomoku-secret"
socketio = SocketIO(app, cors_allowed_origins="*", async_mode="threading")

# Path to the engine executable (relative to this script)
ENGINE_PATH = os.path.normpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)),
                 "..", "Gomoku-main", "build", "Gomoku.exe")
)

# Per-session game state
games = {}            # sid -> subprocess.Popen
engine_timers = {}    # sid -> float (timestamp when human move sent)
reader_threads = {}   # sid -> threading.Thread


# ---------------------------------------------------------------------------
# Utility: reader thread
# ---------------------------------------------------------------------------
def _reader_thread(process, sid):
    """Continuously read lines from the engine's stdout and emit events."""
    try:
        while True:
            line = process.stdout.readline()
            if not line:
                break
            line = line.strip()
            if not line:
                continue

            # Game over: winner
            if line.startswith("winner"):
                parts = line.split()
                winner = parts[1] if len(parts) > 1 else "?"
                elapsed = time.time() - engine_timers.get(sid, time.time())
                socketio.emit("game_over",
                              {"winner": winner, "time": round(elapsed, 3)},
                              to=sid)
                break

            # Game over: draw
            if line == "draw":
                socketio.emit("game_over",
                              {"winner": "draw", "time": 0},
                              to=sid)
                break

            # Engine move: "row col"
            try:
                parts = line.split()
                row, col = int(parts[0]), int(parts[1])
                elapsed = time.time() - engine_timers.get(sid, time.time())
                socketio.emit("engine_move",
                              {"row": row, "col": col,
                               "time": round(elapsed, 3)},
                              to=sid)
            except (ValueError, IndexError):
                pass  # Ignore unexpected output

    except Exception as exc:
        socketio.emit("error", {"message": str(exc)}, to=sid)


def _kill_game(sid):
    """Terminate any existing game for the given session."""
    if sid in games:
        proc = games.pop(sid)
        try:
            proc.stdin.close()
        except Exception:
            pass
        try:
            proc.terminate()
            proc.wait(timeout=2)
        except Exception:
            try:
                proc.kill()
            except Exception:
                pass
    engine_timers.pop(sid, None)
    reader_threads.pop(sid, None)


# ---------------------------------------------------------------------------
# HTTP routes (serve static files)
# ---------------------------------------------------------------------------
@app.route("/")
def index():
    return send_from_directory(".", "index.html")


@app.route("/<path:filename>")
def serve_static(filename):
    return send_from_directory(".", filename)


# ---------------------------------------------------------------------------
# WebSocket events
# ---------------------------------------------------------------------------
@socketio.on("connect")
def on_connect():
    print(f"[+] Client connected: {request.sid}")


@socketio.on("disconnect")
def on_disconnect():
    print(f"[-] Client disconnected: {request.sid}")
    _kill_game(request.sid)


@socketio.on("start_game")
def on_start_game(data):
    sid = request.sid
    _kill_game(sid)

    depth = data.get("depth", 4)
    level = data.get("level", "vcf")
    time_limit = data.get("time", 0)
    mode = data.get("mode", "pvp")  # "pvp" = human vs engine, "ava" = AI vs AI

    if mode == "ava":
        args = [ENGINE_PATH, "-b", "engine", "-w", "engine",
                "-m", "quiet", "-d", str(depth), "-l", level]
    else:
        args = [ENGINE_PATH, "-b", "human", "-w", "engine",
                "-m", "quiet", "-d", str(depth), "-l", level]

    if time_limit and float(time_limit) > 0:
        args.extend(["-t", str(time_limit)])

    if not os.path.isfile(ENGINE_PATH):
        emit("error", {"message": f"Engine not found at: {ENGINE_PATH}"})
        return

    try:
        process = subprocess.Popen(
            args,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
        )
    except Exception as exc:
        emit("error", {"message": f"Failed to start engine: {exc}"})
        return

    games[sid] = process
    engine_timers[sid] = time.time()

    # Start reader thread
    t = threading.Thread(target=_reader_thread, args=(process, sid), daemon=True)
    t.start()
    reader_threads[sid] = t

    emit("game_started", {"mode": mode})
    print(f"[*] Game started for {sid}: depth={depth}, level={level}, mode={mode}")


@socketio.on("human_move")
def on_human_move(data):
    sid = request.sid

    if sid not in games:
        emit("error", {"message": "No active game. Start a new game first."})
        return

    process = games[sid]
    row, col = data["row"], data["col"]

    try:
        engine_timers[sid] = time.time()
        process.stdin.write(f"{row} {col}\n")
        process.stdin.flush()
        emit("move_accepted", {"row": row, "col": col})
    except (BrokenPipeError, OSError) as exc:
        emit("error", {"message": f"Engine process error: {exc}"})
        _kill_game(sid)


@socketio.on("new_game")
def on_new_game(data):
    _kill_game(request.sid)
    on_start_game(data)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    print(f"Engine path: {ENGINE_PATH}")
    print(f"Engine exists: {os.path.isfile(ENGINE_PATH)}")
    print("Starting Gomoku GUI server on http://localhost:5000")
    socketio.run(app, host="0.0.0.0", port=5000, debug=False, allow_unsafe_werkzeug=True)
