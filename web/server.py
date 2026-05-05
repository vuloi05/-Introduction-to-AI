"""
Gomoku Web Server — FastAPI bridge to C++ engine.
Supports: Human vs AI, AI vs AI, Human vs Human.
"""

import os
import subprocess
import time
import threading
from pathlib import Path

from fastapi import FastAPI, HTTPException
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel
from typing import Optional

BASE_DIR = Path(__file__).resolve().parent
PROJECT_DIR = BASE_DIR.parent
ENGINE_PATH = PROJECT_DIR / "build" / "Gomoku.exe"
STATIC_DIR = BASE_DIR / "static"
BOARD_SIZE = 16


class GameSession:
    def __init__(self):
        self.process = None
        self.board = [[0] * BOARD_SIZE for _ in range(BOARD_SIZE)]
        self.moves = []
        self.game_over = False
        self.winner = None
        self.mode = 'hvai'
        self.player_color = 'black'
        self.current_turn = 'black'
        self.lock = threading.Lock()

    def stop(self):
        if self.process and self.process.poll() is None:
            try:
                self.process.stdin.close()
                self.process.terminate()
                self.process.wait(timeout=2)
            except Exception:
                try:
                    self.process.kill()
                except Exception:
                    pass
        self.process = None

    def _reset(self):
        self.stop()
        self.board = [[0] * BOARD_SIZE for _ in range(BOARD_SIZE)]
        self.moves = []
        self.game_over = False
        self.winner = None
        self.current_turn = 'black'

    def _launch_engine(self, cmd):
        if not ENGINE_PATH.exists():
            raise FileNotFoundError(f"Engine not found: {ENGINE_PATH}")
        self.process = subprocess.Popen(
            cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, text=True, bufsize=1,
        )

    # ---- Human vs AI -------------------------------------------------------
    def start_hvai(self, player_color, level, depth, time_limit, threads):
        self._reset()
        self.mode = 'hvai'
        self.player_color = player_color
        cmd = [str(ENGINE_PATH), '-m', 'quiet', '-d', str(depth), '-l', level]
        if time_limit > 0:
            cmd += ['-t', str(time_limit)]
        if threads > 0:
            cmd += ['-T', str(threads)]
        if player_color == 'black':
            cmd += ['-b', 'human', '-w', 'engine']
        else:
            cmd += ['-b', 'engine', '-w', 'human']
        self._launch_engine(cmd)

        first_move = None
        if player_color == 'white':
            first_move = self._read_engine_move()
            if first_move:
                self.board[first_move['row']][first_move['col']] = 1
                self.moves.append({**first_move, 'stone': 'black'})
                self.current_turn = 'white'
        return first_move

    def make_human_move(self, row, col):
        if self.game_over:
            raise ValueError('Game is already over')
        if self.board[row][col] != 0:
            raise ValueError('Cell is occupied')

        human_stone = 1 if self.player_color == 'black' else 2
        self.board[row][col] = human_stone
        self.moves.append({'row': row, 'col': col, 'stone': self.player_color})
        self.current_turn = 'white' if self.player_color == 'black' else 'black'

        self.process.stdin.write(f"{row} {col}\n")
        self.process.stdin.flush()

        if self._check_five(row, col, human_stone):
            self.game_over = True
            self.winner = self.player_color
            self._try_read_line()
            return {'aiMove': None, 'gameOver': True, 'winner': self.winner, 'engineTime': 0}

        start_t = time.time()
        ai_move = self._read_engine_move()
        elapsed = time.time() - start_t

        if ai_move is None:
            return {'aiMove': None, 'gameOver': True, 'winner': self.winner, 'engineTime': elapsed}

        ai_stone = 2 if self.player_color == 'black' else 1
        ai_color = 'white' if self.player_color == 'black' else 'black'
        r, c = ai_move['row'], ai_move['col']
        self.board[r][c] = ai_stone
        self.moves.append({'row': r, 'col': c, 'stone': ai_color})
        self.current_turn = self.player_color

        if self._check_five(r, c, ai_stone):
            self.game_over = True
            self.winner = ai_color
            self._try_read_line()

        return {'aiMove': ai_move, 'gameOver': self.game_over,
                'winner': self.winner, 'engineTime': round(elapsed, 3)}

    # ---- AI vs AI ----------------------------------------------------------
    def start_aivai(self, black_level, black_depth, white_level, white_depth, time_limit):
        self._reset()
        self.mode = 'aivai'
        cmd = [str(ENGINE_PATH), '-b', 'engine', '-w', 'engine', '-m', 'quiet',
               '--black-level', black_level, '--black-depth', str(black_depth),
               '--white-level', white_level, '--white-depth', str(white_depth)]
        if time_limit > 0:
            cmd += ['-t', str(time_limit)]
        self._launch_engine(cmd)

    def ai_step(self):
        with self.lock:
            if self.game_over:
                return {'move': None, 'gameOver': True, 'winner': self.winner}

            start_t = time.time()
            raw = self._read_engine_move()
            elapsed = time.time() - start_t

            if raw is None:
                return {'move': None, 'gameOver': True, 'winner': self.winner, 'engineTime': round(elapsed, 3)}

            stone_val = 1 if len(self.moves) % 2 == 0 else 2
            stone_name = 'black' if stone_val == 1 else 'white'
            r, c = raw['row'], raw['col']
            self.board[r][c] = stone_val
            self.moves.append({'row': r, 'col': c, 'stone': stone_name})
            self.current_turn = 'white' if stone_name == 'black' else 'black'

            if self._check_five(r, c, stone_val):
                self.game_over = True
                self.winner = stone_name
                self._try_read_line()

            return {'move': {'row': r, 'col': c}, 'stone': stone_name,
                    'gameOver': self.game_over, 'winner': self.winner,
                    'engineTime': round(elapsed, 3)}

    # ---- Human vs Human ----------------------------------------------------
    def start_hvh(self):
        self._reset()
        self.mode = 'hvh'

    def make_hvh_move(self, row, col):
        if self.game_over:
            raise ValueError('Game is already over')
        if self.board[row][col] != 0:
            raise ValueError('Cell is occupied')

        stone_val = 1 if self.current_turn == 'black' else 2
        self.board[row][col] = stone_val
        self.moves.append({'row': row, 'col': col, 'stone': self.current_turn})

        if self._check_five(row, col, stone_val):
            self.game_over = True
            self.winner = self.current_turn
        else:
            self.current_turn = 'white' if self.current_turn == 'black' else 'black'

        return {'gameOver': self.game_over, 'winner': self.winner,
                'currentTurn': self.current_turn}

    # ---- Engine I/O --------------------------------------------------------
    def _read_engine_move(self):
        try:
            line = self.process.stdout.readline().strip()
        except Exception:
            self.game_over = True
            self.winner = 'draw'
            return None
        if not line:
            self.game_over = True
            return None
        if line.startswith('winner'):
            self.game_over = True
            self.winner = 'black' if line.split()[1] == 'X' else 'white'
            return None
        if line == 'draw':
            self.game_over = True
            self.winner = 'draw'
            return None
        parts = line.split()
        return {'row': int(parts[0]), 'col': int(parts[1])}

    def _try_read_line(self):
        try:
            self.process.stdout.readline()
        except Exception:
            pass

    def _check_five(self, row, col, stone):
        for dr, dc in [(0, 1), (1, 0), (1, 1), (1, -1)]:
            count = 1
            for sign in (1, -1):
                r, c = row + dr * sign, col + dc * sign
                while 0 <= r < BOARD_SIZE and 0 <= c < BOARD_SIZE and self.board[r][c] == stone:
                    count += 1
                    r += dr * sign
                    c += dc * sign
            if count >= 5:
                return True
        return False


# ---------------------------------------------------------------------------
# FastAPI
# ---------------------------------------------------------------------------
app = FastAPI(title="Gomoku Web GUI")
game = GameSession()


class NewGameRequest(BaseModel):
    mode: str = 'hvai'
    playerColor: str = 'black'
    level: str = 'vcf'
    depth: int = 4
    timeLimit: float = 0
    threads: int = -1
    blackLevel: str = 'vcf'
    blackDepth: int = 4
    whiteLevel: str = 'vcf'
    whiteDepth: int = 4


class MoveRequest(BaseModel):
    row: int
    col: int


@app.post("/api/new-game")
def new_game(req: NewGameRequest):
    try:
        if req.mode == 'hvai':
            first_move = game.start_hvai(req.playerColor, req.level, req.depth,
                                          req.timeLimit, req.threads)
            return {'success': True, 'mode': 'hvai', 'firstMove': first_move,
                    'board': game.board, 'playerColor': game.player_color}
        elif req.mode == 'aivai':
            game.start_aivai(req.blackLevel, req.blackDepth,
                             req.whiteLevel, req.whiteDepth, req.timeLimit)
            return {'success': True, 'mode': 'aivai', 'board': game.board}
        elif req.mode == 'hvh':
            game.start_hvh()
            return {'success': True, 'mode': 'hvh', 'board': game.board,
                    'currentTurn': 'black'}
    except FileNotFoundError as e:
        raise HTTPException(500, str(e))


@app.post("/api/move")
def make_move(req: MoveRequest):
    try:
        if game.mode == 'hvai':
            result = game.make_human_move(req.row, req.col)
        elif game.mode == 'hvh':
            result = game.make_hvh_move(req.row, req.col)
        else:
            raise ValueError("Use /api/ai-step for AI vs AI mode")
    except ValueError as e:
        raise HTTPException(400, str(e))
    return {'success': True, **result, 'board': game.board, 'moves': game.moves}


@app.post("/api/ai-step")
def ai_step():
    if game.mode != 'aivai':
        raise HTTPException(400, "Not in AI vs AI mode")
    result = game.ai_step()
    return {'success': True, **result, 'board': game.board, 'moves': game.moves}


@app.post("/api/resign")
def resign():
    game.game_over = True
    if game.mode == 'hvai':
        game.winner = 'white' if game.player_color == 'black' else 'black'
    else:
        game.winner = 'draw'
    game.stop()
    return {'success': True, 'winner': game.winner}


app.mount("/static", StaticFiles(directory=str(STATIC_DIR)), name="static")


@app.get("/")
def serve_index():
    return FileResponse(str(STATIC_DIR / "index.html"))


if __name__ == "__main__":
    import uvicorn
    print(f"Engine: {ENGINE_PATH}")
    print(f"Server: http://localhost:8080")
    uvicorn.run(app, host="127.0.0.1", port=8080)
