/**
 * Gomoku Web GUI — Supports Human vs AI, AI vs AI, Human vs Human
 */

const BOARD_SIZE = 16;
const STAR_POINTS = [[3,3],[3,7],[3,11],[7,3],[7,7],[7,11],[11,3],[11,7],[11,11]];

// ---- State ----
let state = {
    board: Array.from({length: BOARD_SIZE}, () => Array(BOARD_SIZE).fill(0)),
    moves: [],
    mode: 'hvai',
    playerColor: 'black',
    currentTurn: 'black',
    gameActive: false,
    gameOver: false,
    winner: null,
    isThinking: false,
};

let autoPlayTimer = null;
let autoPlaying = false;

// ---- DOM ----
const $ = id => document.getElementById(id);
const $board = $('board');
const $statusBar = $('status-bar');
const $moveList = $('move-list');
const $overlay = $('game-over-overlay');

// ---- Board ----
function initBoard() {
    $board.innerHTML = '';
    for (let r = 0; r < BOARD_SIZE; r++) {
        for (let c = 0; c < BOARD_SIZE; c++) {
            const cell = document.createElement('div');
            cell.className = 'cell';
            cell.id = `cell-${r}-${c}`;
            if (STAR_POINTS.some(([sr,sc]) => sr===r && sc===c)) cell.classList.add('star-point');
            const preview = document.createElement('div');
            preview.className = 'hover-preview';
            cell.appendChild(preview);
            cell.addEventListener('click', () => onCellClick(r, c));
            $board.appendChild(cell);
        }
    }
    updatePreviews();
}

function placeStone(row, col, color) {
    state.board[row][col] = color === 'black' ? 1 : 2;
    const cell = $(`cell-${row}-${col}`);
    if (!cell) return;
    document.querySelectorAll('.stone.last-move').forEach(s => s.classList.remove('last-move'));
    const stone = document.createElement('div');
    stone.className = `stone ${color} last-move`;
    cell.appendChild(stone);
    cell.classList.add('occupied');
}

function updatePreviews() {
    const color = state.mode === 'hvh' ? state.currentTurn : state.playerColor;
    document.querySelectorAll('.hover-preview').forEach(p => {
        p.className = `hover-preview ${color}`;
    });
}

// ---- Mode switching ----
function onModeChange() {
    const mode = $('select-mode').value;
    $('settings-hvai').style.display = mode === 'hvai' ? '' : 'none';
    $('settings-aivai').style.display = mode === 'aivai' ? '' : 'none';
    $('settings-hvh').style.display = mode === 'hvh' ? '' : 'none';
    $('ai-controls').style.display = mode === 'aivai' ? 'flex' : 'none';
}

// ---- Game start ----
async function startNewGame() {
    stopAutoPlay();
    state.mode = $('select-mode').value;
    state.gameOver = false;
    state.gameActive = false;
    state.winner = null;
    state.isThinking = false;
    state.moves = [];
    state.board = Array.from({length: BOARD_SIZE}, () => Array(BOARD_SIZE).fill(0));
    state.currentTurn = 'black';

    hideOverlay();
    clearHistory();
    initBoard();

    const body = { mode: state.mode };

    if (state.mode === 'hvai') {
        state.playerColor = document.querySelector('.color-btn.active')?.dataset.color || 'black';
        Object.assign(body, {
            playerColor: state.playerColor,
            level: $('select-level').value,
            depth: +$('range-depth').value,
            timeLimit: +$('range-time').value,
        });
    } else if (state.mode === 'aivai') {
        Object.assign(body, {
            blackLevel: $('select-black-level').value,
            blackDepth: +$('range-black-depth').value,
            whiteLevel: $('select-white-level').value,
            whiteDepth: +$('range-white-depth').value,
        });
    }

    try {
        const res = await apiCall('/api/new-game', body);
        state.gameActive = true;
        $('btn-resign').disabled = false;
        $board.classList.remove('board-disabled');

        const modeNames = { hvai: 'Human vs AI', aivai: 'AI vs AI', hvh: 'Human vs Human' };
        $('info-mode').textContent = modeNames[state.mode];
        $('info-moves').textContent = '0';
        $('info-time').textContent = '—';

        if (state.mode === 'hvai') {
            if (res.firstMove) {
                placeStone(res.firstMove.row, res.firstMove.col, 'black');
                state.moves.push({...res.firstMove, stone: 'black'});
                addMoveToHistory(res.firstMove.row, res.firstMove.col, 'black');
                state.currentTurn = 'white';
            }
            setStatus('Your turn — click a cell to place your stone');
        } else if (state.mode === 'aivai') {
            $board.classList.add('board-disabled');
            $('btn-play-pause').disabled = false;
            $('btn-step').disabled = false;
            $('btn-play-pause').textContent = '▶ Play';
            setStatus('AI vs AI ready — press Play or Step');
        } else {
            setStatus('Black\'s turn — click to place');
            updatePreviews();
        }
    } catch (err) {
        setStatus(`Error: ${err.message}`);
    }
}

// ---- Cell click ----
async function onCellClick(row, col) {
    if (!state.gameActive || state.gameOver || state.isThinking) return;
    if (state.board[row][col] !== 0) return;
    if (state.mode === 'aivai') return;

    if (state.mode === 'hvh') {
        await makeHvhMove(row, col);
    } else {
        await makeHvaiMove(row, col);
    }
}

// ---- Human vs AI move ----
async function makeHvaiMove(row, col) {
    placeStone(row, col, state.playerColor);
    state.moves.push({row, col, stone: state.playerColor});
    addMoveToHistory(row, col, state.playerColor);

    state.isThinking = true;
    $board.classList.add('board-disabled');
    setStatus('<span class="dot thinking"></span> AI is thinking...');

    try {
        const res = await apiCall('/api/move', {row, col});
        state.isThinking = false;

        if (res.gameOver && !res.aiMove) {
            endGame(res.winner);
            return;
        }
        if (res.aiMove) {
            const aiColor = state.playerColor === 'black' ? 'white' : 'black';
            placeStone(res.aiMove.row, res.aiMove.col, aiColor);
            state.moves.push({...res.aiMove, stone: aiColor});
            addMoveToHistory(res.aiMove.row, res.aiMove.col, aiColor, res.engineTime);
            if (res.engineTime != null) $('info-time').textContent = `${res.engineTime}s`;
        }
        $('info-moves').textContent = state.moves.length;
        if (res.gameOver) { endGame(res.winner); return; }
        $board.classList.remove('board-disabled');
        setStatus('Your turn — click a cell to place your stone');
    } catch (err) {
        state.isThinking = false;
        $board.classList.remove('board-disabled');
        setStatus(`Error: ${err.message}`);
    }
}

// ---- Human vs Human move ----
async function makeHvhMove(row, col) {
    const stone = state.currentTurn;
    placeStone(row, col, stone);
    state.moves.push({row, col, stone});
    addMoveToHistory(row, col, stone);
    $('info-moves').textContent = state.moves.length;

    try {
        const res = await apiCall('/api/move', {row, col});
        if (res.gameOver) { endGame(res.winner); return; }
        state.currentTurn = res.currentTurn;
        updatePreviews();
        setStatus(`${state.currentTurn === 'black' ? 'Black' : 'White'}'s turn — click to place`);
    } catch (err) {
        setStatus(`Error: ${err.message}`);
    }
}

// ---- AI vs AI step ----
let stepping = false; // prevent concurrent steps

async function doAiStep() {
    if (state.gameOver || stepping) { stopAutoPlay(); return; }
    stepping = true;

    const nextStone = state.moves.length % 2 === 0 ? 'Black' : 'White';
    setStatus(`<span class="dot thinking"></span> ${nextStone} AI is thinking...`);

    try {
        const res = await apiCall('/api/ai-step', {});
        if (res.move) {
            placeStone(res.move.row, res.move.col, res.stone);
            state.moves.push({...res.move, stone: res.stone});
            addMoveToHistory(res.move.row, res.move.col, res.stone, res.engineTime);
            $('info-moves').textContent = state.moves.length;
            if (res.engineTime != null) $('info-time').textContent = `${res.engineTime}s`;
            setStatus(`Move ${state.moves.length} — ${res.stone === 'black' ? 'Black' : 'White'} plays (${res.move.row}, ${res.move.col})`);
        }
        if (res.gameOver) {
            endGame(res.winner);
            stopAutoPlay();
        }
    } catch (err) {
        stopAutoPlay();
        setStatus(`Error: ${err.message}`);
    } finally {
        stepping = false;
    }
}

async function autoPlayLoop() {
    while (autoPlaying && !state.gameOver) {
        await doAiStep();
        if (autoPlaying && !state.gameOver) {
            const delay = +$('range-speed').value;
            await new Promise(r => setTimeout(r, delay));
        }
    }
}

function startAutoPlay() {
    if (autoPlaying) return;
    autoPlaying = true;
    $('btn-play-pause').textContent = '⏸ Pause';
    autoPlayLoop();
}

function stopAutoPlay() {
    autoPlaying = false;
    $('btn-play-pause').textContent = '▶ Play';
}

function toggleAutoPlay() {
    if (autoPlaying) stopAutoPlay();
    else startAutoPlay();
}

// ---- End game ----
function endGame(winner) {
    state.gameOver = true;
    state.gameActive = false;
    $('btn-resign').disabled = true;
    $('btn-play-pause').disabled = true;
    $('btn-step').disabled = true;
    $board.classList.add('board-disabled');

    const $title = $('game-over-title');
    const $text = $('game-over-text');

    if (winner === 'draw') {
        $title.textContent = '🤝 Draw!';
        $text.textContent = 'The game ended in a draw.';
        $text.className = 'result-text result-draw';
    } else if (state.mode === 'aivai') {
        $title.textContent = `🏆 ${winner === 'black' ? 'Black' : 'White'} Wins!`;
        $text.textContent = `${winner === 'black' ? 'Black' : 'White'} AI wins in ${state.moves.length} moves.`;
        $text.className = 'result-text result-win';
    } else if (state.mode === 'hvh') {
        $title.textContent = `🎉 ${winner === 'black' ? 'Black' : 'White'} Wins!`;
        $text.textContent = `${winner === 'black' ? 'Black' : 'White'} player wins!`;
        $text.className = 'result-text result-win';
    } else if (winner === state.playerColor) {
        $title.textContent = '🎉 You Win!';
        $text.textContent = 'Congratulations!';
        $text.className = 'result-text result-win';
    } else {
        $title.textContent = '💀 You Lose';
        $text.textContent = 'The AI wins. Try again!';
        $text.className = 'result-text result-lose';
    }
    showOverlay();
}

// ---- History ----
function addMoveToHistory(row, col, stone, time = null) {
    if (state.moves.length <= 1) $moveList.innerHTML = '';
    const div = document.createElement('div');
    div.className = 'move-row';
    div.innerHTML = `
        <span class="move-num">${state.moves.length}</span>
        <span class="move-stone ${stone}"></span>
        <span class="move-coord">(${row}, ${col})</span>
        ${time != null ? `<span class="move-time">${time}s</span>` : ''}
    `;
    $moveList.appendChild(div);
    $moveList.scrollTop = $moveList.scrollHeight;
}

function clearHistory() {
    $moveList.innerHTML = '<div style="color:var(--text-muted);font-size:0.8rem;padding:8px">No moves yet</div>';
}

// ---- UI helpers ----
function setStatus(html) { $statusBar.innerHTML = html; }
function showOverlay() { $overlay.classList.add('visible'); }
function hideOverlay() { $overlay.classList.remove('visible'); }

async function apiCall(endpoint, body) {
    const res = await fetch(endpoint, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(body),
    });
    if (!res.ok) {
        const err = await res.json().catch(() => ({detail: 'Unknown error'}));
        throw new Error(err.detail || `HTTP ${res.status}`);
    }
    return res.json();
}

// ---- Event listeners ----
$('btn-new-game').addEventListener('click', startNewGame);
$('btn-resign').addEventListener('click', async () => {
    if (!state.gameActive) return;
    stopAutoPlay();
    await apiCall('/api/resign', {});
    endGame(state.mode === 'hvai' ? (state.playerColor === 'black' ? 'white' : 'black') : 'draw');
});
$('btn-play-again').addEventListener('click', () => { hideOverlay(); startNewGame(); });
$('btn-play-pause').addEventListener('click', toggleAutoPlay);
$('btn-step').addEventListener('click', doAiStep);

// Color picker
$('btn-color-black').addEventListener('click', () => {
    state.playerColor = 'black';
    $('btn-color-black').classList.add('active');
    $('btn-color-white').classList.remove('active');
    updatePreviews();
});
$('btn-color-white').addEventListener('click', () => {
    state.playerColor = 'white';
    $('btn-color-white').classList.add('active');
    $('btn-color-black').classList.remove('active');
    updatePreviews();
});

// Mode change
$('select-mode').addEventListener('change', onModeChange);

// Range sliders
$('range-depth').addEventListener('input', e => $('depth-value').textContent = e.target.value);
$('range-time').addEventListener('input', e => $('time-value').textContent = e.target.value);
$('range-black-depth').addEventListener('input', e => $('black-depth-value').textContent = e.target.value);
$('range-white-depth').addEventListener('input', e => $('white-depth-value').textContent = e.target.value);
$('range-speed').addEventListener('input', e => $('speed-value').textContent = e.target.value);

// ---- Init ----
initBoard();
onModeChange();
