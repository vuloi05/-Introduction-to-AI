/* =====================================================================
   Gomoku Web GUI — Application Logic
   Handles board rendering, socket.io communication, and game flow.
   ===================================================================== */

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
const BOARD_SIZE = 16;
const CELL_SIZE  = parseInt(getComputedStyle(document.documentElement).getPropertyValue('--cell-size')) || 38;
const PADDING    = parseInt(getComputedStyle(document.documentElement).getPropertyValue('--board-padding')) || 24;
const STONE_SIZE = parseInt(getComputedStyle(document.documentElement).getPropertyValue('--stone-size')) || 32;

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------
let board       = [];           // 16x16: 0=empty, 1=black, 2=white
let moveCount   = 0;
let currentTurn = 'black';      // 'black' or 'white'
let gameActive  = false;
let gameMode    = 'pvp';        // 'pvp' = human vs engine, 'ava' = AI vs AI
let lastMove    = null;
let socket      = null;
let settings    = { depth: 4, level: 'vcf', time: 0 };

// ---------------------------------------------------------------------------
// DOM refs
// ---------------------------------------------------------------------------
const $welcome     = document.getElementById('screen-welcome');
const $game        = document.getElementById('screen-game');
const $canvas      = document.getElementById('board-canvas');
const $stonesLayer = document.getElementById('stones-layer');
const $clickLayer  = document.getElementById('click-layer');
const $overlay     = document.getElementById('game-over-overlay');

const $turnIndicator = document.getElementById('turn-indicator');
const $turnStone     = document.getElementById('turn-stone');
const $turnLabel     = document.getElementById('turn-label');
const $turnDetail    = document.getElementById('turn-detail');
const $thinkingBar   = document.getElementById('thinking-bar');

const $statTime   = document.getElementById('stat-time');
const $statMoves  = document.getElementById('stat-moves');
const $statDepth  = document.getElementById('stat-depth');
const $statLevel  = document.getElementById('stat-level');
const $moveHist   = document.getElementById('move-history');

const $winnerIcon   = document.getElementById('winner-icon');
const $winnerText   = document.getElementById('winner-text');
const $winnerDetail = document.getElementById('winner-detail');

// Settings controls
const $depthSlider = document.getElementById('setting-depth');
const $depthValue  = document.getElementById('depth-value');
const $levelSelect = document.getElementById('setting-level');
const $timeSlider  = document.getElementById('setting-time');
const $timeValue   = document.getElementById('time-value');

// ---------------------------------------------------------------------------
// Settings UI
// ---------------------------------------------------------------------------
$depthSlider.addEventListener('input', () => {
    $depthValue.textContent = $depthSlider.value;
});

$timeSlider.addEventListener('input', () => {
    const v = parseInt($timeSlider.value);
    $timeValue.textContent = v === 0 ? 'Off' : v + 's';
});

// ---------------------------------------------------------------------------
// Screen management
// ---------------------------------------------------------------------------
function showScreen(name) {
    $welcome.classList.remove('active');
    $game.classList.remove('active');
    if (name === 'welcome') $welcome.classList.add('active');
    if (name === 'game')    $game.classList.add('active');
}

// ---------------------------------------------------------------------------
// Board: Canvas rendering (grid lines on wood texture)
// ---------------------------------------------------------------------------
const woodImg = new Image();
woodImg.src = 'wood_texture.png';

function computeSizes() {
    const style = getComputedStyle(document.documentElement);
    const cell = parseInt(style.getPropertyValue('--cell-size')) || 38;
    const pad  = parseInt(style.getPropertyValue('--board-padding')) || 24;
    const canvasW = pad * 2 + cell * (BOARD_SIZE - 1);
    return { cell, pad, canvasW };
}

function drawBoard() {
    const { cell, pad, canvasW } = computeSizes();
    const dpr = window.devicePixelRatio || 1;

    $canvas.width  = canvasW * dpr;
    $canvas.height = canvasW * dpr;
    $canvas.style.width  = canvasW + 'px';
    $canvas.style.height = canvasW + 'px';

    // Also size the container
    const container = document.getElementById('board-container');
    container.style.width  = canvasW + 'px';
    container.style.height = canvasW + 'px';

    const ctx = $canvas.getContext('2d');
    ctx.scale(dpr, dpr);

    // Draw wood texture
    if (woodImg.complete && woodImg.naturalWidth > 0) {
        ctx.drawImage(woodImg, 0, 0, canvasW, canvasW);
    } else {
        // Fallback color
        ctx.fillStyle = '#c4a35a';
        ctx.fillRect(0, 0, canvasW, canvasW);
    }

    // Grid lines
    ctx.strokeStyle = 'rgba(30, 15, 5, 0.55)';
    ctx.lineWidth = 1;

    for (let i = 0; i < BOARD_SIZE; i++) {
        const pos = pad + i * cell;
        // Horizontal
        ctx.beginPath();
        ctx.moveTo(pad, pos);
        ctx.lineTo(pad + (BOARD_SIZE - 1) * cell, pos);
        ctx.stroke();
        // Vertical
        ctx.beginPath();
        ctx.moveTo(pos, pad);
        ctx.lineTo(pos, pad + (BOARD_SIZE - 1) * cell);
        ctx.stroke();
    }

    // Star points (common Go/Gomoku star points for 16x16)
    const starPoints = [
        [3, 3], [3, 12], [12, 3], [12, 12], [7, 7], [7, 8], [8, 7], [8, 8]
    ];
    ctx.fillStyle = 'rgba(30, 15, 5, 0.6)';
    for (const [r, c] of starPoints) {
        const x = pad + c * cell;
        const y = pad + r * cell;
        ctx.beginPath();
        ctx.arc(x, y, 3.5, 0, Math.PI * 2);
        ctx.fill();
    }
}

woodImg.onload = () => { drawBoard(); };

// ---------------------------------------------------------------------------
// Board: Click layer (intersection targets)
// ---------------------------------------------------------------------------
function buildClickLayer() {
    const { cell, pad, canvasW } = computeSizes();
    $clickLayer.innerHTML = '';
    $clickLayer.style.width  = canvasW + 'px';
    $clickLayer.style.height = canvasW + 'px';

    for (let r = 0; r < BOARD_SIZE; r++) {
        for (let c = 0; c < BOARD_SIZE; c++) {
            const div = document.createElement('div');
            div.className = 'intersection';
            div.dataset.row = r;
            div.dataset.col = c;
            div.style.left = (pad + c * cell) + 'px';
            div.style.top  = (pad + r * cell) + 'px';

            // Hover preview
            const hover = document.createElement('div');
            hover.className = 'hover-stone hover-black';
            div.appendChild(hover);

            div.addEventListener('click', () => onCellClick(r, c));
            $clickLayer.appendChild(div);
        }
    }
}

function updateClickLayer() {
    const intersections = $clickLayer.querySelectorAll('.intersection');
    intersections.forEach(el => {
        const r = parseInt(el.dataset.row);
        const c = parseInt(el.dataset.col);
        const hover = el.querySelector('.hover-stone');

        if (board[r][c] !== 0 || !gameActive) {
            el.classList.add('disabled');
            if (hover) hover.style.display = 'none';
        } else {
            el.classList.remove('disabled');
            if (hover) {
                hover.style.display = '';
                hover.className = 'hover-stone ' + (currentTurn === 'black' ? 'hover-black' : 'hover-white');
            }
        }

        // Disable clicks during engine's turn in PvP mode
        if (gameMode === 'pvp' && currentTurn === 'white') {
            el.classList.add('disabled');
            if (hover) hover.style.display = 'none';
        }
        // Disable all clicks in AvA mode
        if (gameMode === 'ava') {
            el.classList.add('disabled');
            if (hover) hover.style.display = 'none';
        }
    });
}

// ---------------------------------------------------------------------------
// Board: Stone rendering
// ---------------------------------------------------------------------------
function clearStones() {
    $stonesLayer.innerHTML = '';
}

function placeStone(row, col, color) {
    const { cell, pad } = computeSizes();
    const stoneSize = parseInt(getComputedStyle(document.documentElement).getPropertyValue('--stone-size')) || 32;

    const stone = document.createElement('div');
    stone.className = `stone stone-${color}`;
    stone.style.left = (pad + col * cell) + 'px';
    stone.style.top  = (pad + row * cell) + 'px';
    stone.id = `stone-${row}-${col}`;
    $stonesLayer.appendChild(stone);

    // Remove old last-move marker
    const oldMarker = $stonesLayer.querySelector('.last-move-marker');
    if (oldMarker) oldMarker.remove();

    // Add new last-move marker
    const marker = document.createElement('div');
    marker.className = `last-move-marker last-marker-on-${color}`;
    marker.style.left = (pad + col * cell) + 'px';
    marker.style.top  = (pad + row * cell) + 'px';
    $stonesLayer.appendChild(marker);
}

// ---------------------------------------------------------------------------
// Board: State management
// ---------------------------------------------------------------------------
function initBoard() {
    board = Array.from({ length: BOARD_SIZE }, () => Array(BOARD_SIZE).fill(0));
    moveCount   = 0;
    currentTurn = 'black';
    lastMove    = null;
}

function applyMove(row, col, color) {
    const stoneVal = color === 'black' ? 1 : 2;
    board[row][col] = stoneVal;
    moveCount++;
    lastMove = { row, col };
    placeStone(row, col, color);
    addMoveToHistory(moveCount, color, row, col);
    $statMoves.textContent = moveCount;
}

// ---------------------------------------------------------------------------
// Move History
// ---------------------------------------------------------------------------
function addMoveToHistory(num, color, row, col) {
    const entry = document.createElement('div');
    entry.className = 'move-entry';
    entry.innerHTML = `
        <span class="move-num">${num}.</span>
        <span class="move-stone-icon ${color}"></span>
        <span class="move-coord">(${row}, ${col})</span>
    `;
    $moveHist.appendChild(entry);
    $moveHist.scrollTop = $moveHist.scrollHeight;
}

function clearHistory() {
    $moveHist.innerHTML = '';
}

// ---------------------------------------------------------------------------
// Turn / UI updates
// ---------------------------------------------------------------------------
function setTurn(color) {
    currentTurn = color;

    $turnStone.className = 'turn-stone ' + color;

    if (gameMode === 'ava') {
        $turnLabel.textContent = color === 'black' ? 'Black thinking...' : 'White thinking...';
        $turnDetail.textContent = 'AI vs AI mode';
        $turnIndicator.className = 'turn-indicator engine-turn';
        $thinkingBar.classList.remove('hidden');
    } else if (color === 'black') {
        $turnLabel.textContent = 'Your Turn';
        $turnDetail.textContent = 'Place a black stone';
        $turnIndicator.className = 'turn-indicator your-turn';
        $thinkingBar.classList.add('hidden');
    } else {
        $turnLabel.textContent = 'Engine Thinking...';
        $turnDetail.textContent = 'Computing best move';
        $turnIndicator.className = 'turn-indicator engine-turn';
        $thinkingBar.classList.remove('hidden');
    }

    updateClickLayer();
}

// ---------------------------------------------------------------------------
// Game flow
// ---------------------------------------------------------------------------
function startGame(mode) {
    gameMode = mode;
    settings.depth = parseInt($depthSlider.value);
    settings.level = $levelSelect.value;
    settings.time  = parseInt($timeSlider.value);

    // Init board
    initBoard();
    clearStones();
    clearHistory();
    drawBoard();
    buildClickLayer();

    // Update stats display
    $statDepth.textContent = settings.depth;
    $statLevel.textContent = settings.level.toUpperCase();
    $statTime.textContent  = '—';
    $statMoves.textContent = '0';

    // Hide overlay
    $overlay.classList.add('hidden');

    // Connect socket & start game
    connectSocket();

    // Show game screen
    showScreen('game');
    gameActive = true;

    setTurn('black');

    socket.emit('start_game', {
        depth: settings.depth,
        level: settings.level,
        time:  settings.time,
        mode:  mode
    });
}

function onCellClick(row, col) {
    if (!gameActive) return;
    if (gameMode === 'ava') return;
    if (currentTurn !== 'black') return;
    if (board[row][col] !== 0) return;

    // Place human stone
    applyMove(row, col, 'black');
    setTurn('white');

    // Send to server
    socket.emit('human_move', { row, col });
}

function showGameOver(winner, detail) {
    gameActive = false;
    $thinkingBar.classList.add('hidden');

    $winnerIcon.className = 'winner-icon';

    if (winner === 'X') {
        $winnerIcon.classList.add('black');
        if (gameMode === 'pvp') {
            $winnerText.textContent = 'You Win! 🎉';
            $winnerDetail.textContent = 'Congratulations! You beat the AI.';
        } else {
            $winnerText.textContent = 'Black Wins!';
            $winnerDetail.textContent = 'Black (first player) won the match.';
        }
    } else if (winner === 'O') {
        $winnerIcon.classList.add('white');
        if (gameMode === 'pvp') {
            $winnerText.textContent = 'Engine Wins!';
            $winnerDetail.textContent = 'The AI has defeated you. Try again!';
        } else {
            $winnerText.textContent = 'White Wins!';
            $winnerDetail.textContent = 'White (second player) won the match.';
        }
    } else {
        $winnerIcon.classList.add('draw-icon');
        $winnerText.textContent = 'Draw!';
        $winnerDetail.textContent = 'The board is full. No one wins.';
    }

    if (detail) {
        $winnerDetail.textContent += ' ' + detail;
    }

    $overlay.classList.remove('hidden');
    updateClickLayer();
}

function playAgain() {
    $overlay.classList.add('hidden');
    startGame(gameMode);
}

function backToMenu() {
    gameActive = false;
    $overlay.classList.add('hidden');
    if (socket) {
        socket.disconnect();
        socket = null;
    }
    showScreen('welcome');
}

function quitGame() {
    gameActive = false;
    if (socket) {
        socket.disconnect();
        socket = null;
    }
    showScreen('welcome');
}

// ---------------------------------------------------------------------------
// Socket.IO
// ---------------------------------------------------------------------------
function connectSocket() {
    if (socket && socket.connected) {
        return;
    }

    socket = io();

    socket.on('connect', () => {
        console.log('[Socket] Connected:', socket.id);
    });

    socket.on('game_started', (data) => {
        console.log('[Socket] Game started:', data);
    });

    socket.on('move_accepted', (data) => {
        console.log('[Socket] Move accepted:', data);
    });

    socket.on('engine_move', (data) => {
        console.log('[Socket] Engine move:', data);
        const { row, col, time: elapsed } = data;

        // Place engine's stone
        if (gameMode === 'ava') {
            // In AvA mode, alternate colors
            applyMove(row, col, currentTurn);
            $statTime.textContent = elapsed + 's';
            setTurn(currentTurn === 'black' ? 'white' : 'black');
        } else {
            applyMove(row, col, 'white');
            $statTime.textContent = elapsed + 's';
            setTurn('black');
        }
    });

    socket.on('game_over', (data) => {
        console.log('[Socket] Game over:', data);
        const timeStr = data.time ? ` (${data.time}s)` : '';
        showGameOver(data.winner, timeStr);
    });

    socket.on('error', (data) => {
        console.error('[Socket] Error:', data.message);
        alert('Error: ' + data.message);
    });

    socket.on('disconnect', () => {
        console.log('[Socket] Disconnected');
    });
}

// ---------------------------------------------------------------------------
// Init
// ---------------------------------------------------------------------------
window.addEventListener('resize', () => {
    if ($game.classList.contains('active')) {
        drawBoard();
        rebuildStonesFromState();
    }
});

function rebuildStonesFromState() {
    clearStones();
    buildClickLayer();
    let moveNum = 0;
    // Rebuild from move history isn't stored separately, but stones
    // We can re-place from board state
    for (let r = 0; r < BOARD_SIZE; r++) {
        for (let c = 0; c < BOARD_SIZE; c++) {
            if (board[r][c] === 1) placeStone(r, c, 'black');
            if (board[r][c] === 2) placeStone(r, c, 'white');
        }
    }
    updateClickLayer();
}

// Draw initial board on load
drawBoard();
