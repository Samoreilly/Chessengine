import { useState, useEffect, useRef, useCallback } from 'react';
import { Chessboard } from 'react-chessboard';
import { Chess } from 'chess.js';

// ─── Constants ───────────────────────────────────────────────────────────────
const WS_URL = `ws://${window.location.hostname}:3000`;
const ENGINE_DEPTH = 6;
const RECONNECT_DELAY_MS = 2500;

// ─── App ─────────────────────────────────────────────────────────────────────
function App() {
  // --- Screen state ---
  const [screen, setScreen] = useState('menu'); // 'menu' | 'game'
  const [playerColor, setPlayerColor] = useState('w'); // 'w' | 'b'
  const [gameMode, setGameMode] = useState('engine'); // 'engine' | 'pvp'

  // --- Game state ---
  const [fen, setFen] = useState('start');
  const [isThinking, setIsThinking] = useState(false);
  const [engineEval, setEngineEval] = useState(0);
  const [bestMoveStr, setBestMoveStr] = useState(null);
  const [topLines, setTopLines] = useState([]); // top 3 engine lines for the player
  const [isAnalyzing, setIsAnalyzing] = useState(false); // true when computing hints
  const [moveHistory, setMoveHistory] = useState([]);
  const [gameStatus, setGameStatus] = useState('playing');
  const [statusMessage, setStatusMessage] = useState('');
  const [wsConnected, setWsConnected] = useState(false);
  const [lastMoveSquares, setLastMoveSquares] = useState({});

  // --- Refs (always current, never stale) ---
  const gameRef = useRef(new Chess());
  const wsRef = useRef(null);
  const reconnectTimerRef = useRef(null);
  const playerColorRef = useRef('w');
  const moveHistoryRef = useRef([]);
  const gameStatusRef = useRef('playing');
  const screenRef = useRef('menu');
  const searchModeRef = useRef('engine'); // 'engine' = engine picking a move, 'hint' = suggestion for player
  const gameModeRef = useRef('engine'); // 'engine' | 'pvp'

  // Keep refs in sync
  useEffect(() => { playerColorRef.current = playerColor; }, [playerColor]);
  useEffect(() => { gameStatusRef.current = gameStatus; }, [gameStatus]);
  useEffect(() => { screenRef.current = screen; }, [screen]);
  useEffect(() => { gameModeRef.current = gameMode; }, [gameMode]);

  // ─── Sync chess.js game → React state ──────────────────────────────────────
  const syncGameToState = useCallback(() => {
    const g = gameRef.current;
    setFen(g.fen());

    if (g.isCheckmate()) {
      const winner = g.turn() === 'w' ? 'Black' : 'White';
      setGameStatus('checkmate');
      gameStatusRef.current = 'checkmate';
      setStatusMessage(`Checkmate! ${winner} wins.`);
    } else if (g.isStalemate()) {
      setGameStatus('stalemate');
      gameStatusRef.current = 'stalemate';
      setStatusMessage('Stalemate — draw.');
    } else if (g.isDraw()) {
      setGameStatus('draw');
      gameStatusRef.current = 'draw';
      setStatusMessage('Draw.');
    } else if (g.isCheck()) {
      setStatusMessage(g.turn() === 'w' ? 'White is in check!' : 'Black is in check!');
    } else {
      setStatusMessage('');
    }
  }, []);

  // ─── Send over WebSocket ──────────────────────────────────────────────────
  // ─── Direct WebSocket send (no useCallback, no stale closures) ────────────
  function wsSend(msg) {
    const ws = wsRef.current;
    if (ws && ws.readyState === 1) { // 1 = OPEN
      console.log('[WS →]', msg);
      ws.send(msg);
      return true;
    }
    console.warn('[WS] Cannot send, readyState:', ws?.readyState);
    return false;
  }

  // ─── Handle a parsed engine JSON message ──────────────────────────────────
  // Using a ref-based handler to avoid stale closures entirely
  const handleEngineMessageRef = useRef(null);

  handleEngineMessageRef.current = (msg) => {
    console.log('[Engine →]', msg);

    // ── new game ──
    if (msg.status === 'new_game_started') {
      const freshGame = new Chess();
      gameRef.current = freshGame;
      moveHistoryRef.current = [];
      setMoveHistory([]);
      setEngineEval(0);
      setBestMoveStr(null);
      setTopLines([]);
      setIsAnalyzing(false);
      setIsThinking(false);
      setGameStatus('playing');
      gameStatusRef.current = 'playing';
      setStatusMessage('');
      setLastMoveSquares({});
      setFen(freshGame.fen());

      // If player is black, engine (white) moves first
      if (playerColorRef.current === 'b') {
        console.log('[App] Player is black, engine moves first');
        searchModeRef.current = 'engine';
        setTimeout(() => {
          setIsThinking(true);
          const ws = wsRef.current;
          if (ws && ws.readyState === 1) {
            ws.send('search ' + ENGINE_DEPTH);
          }
        }, 300);
      } else {
        // Player is white — get a hint for their first move
        searchModeRef.current = 'hint';
        setTimeout(() => {
          setIsAnalyzing(true);
          const ws = wsRef.current;
          if (ws && ws.readyState === 1) {
            ws.send('search ' + ENGINE_DEPTH);
          }
        }, 300);
      }
      return;
    }

    // ── move accepted ──
    if (msg.status === 'move_ok') {
      console.log('[App] Move accepted by engine');

      // In PvP, we just want to analyze the new position
      if (gameModeRef.current === 'pvp') {
        console.log('[App] PvP turn, analyzing position');
        searchModeRef.current = 'hint';
        setIsAnalyzing(true);
        const ws = wsRef.current;
        if (ws && ws.readyState === 1) {
          ws.send('search ' + ENGINE_DEPTH);
        }
        return;
      }

      const g = gameRef.current;
      if (g.isGameOver()) {
        console.log('[App] Game is over, not searching');
        return;
      }
      const engineColor = playerColorRef.current === 'w' ? 'b' : 'w';
      const ws = wsRef.current;
      if (g.turn() === engineColor) {
        // It's the engine's turn — search for its move
        console.log('[App] Engine turn, searching for move');
        searchModeRef.current = 'engine';
        setIsThinking(true);
        if (ws && ws.readyState === 1) {
          ws.send('search ' + ENGINE_DEPTH);
        }
      } else {
        // It's the player's turn — search for a hint
        console.log('[App] Player turn, searching for hint');
        searchModeRef.current = 'hint';
        setIsAnalyzing(true);
        if (ws && ws.readyState === 1) {
          ws.send('search ' + ENGINE_DEPTH);
        }
      }
      return;
    }

    // ── illegal move ──
    if (msg.status === 'illegal_move') {
      console.warn('[Engine] Rejected move as illegal');
      gameRef.current.undo();
      syncGameToState();
      setStatusMessage('Engine rejected the move.');
      return;
    }

    // ── error ──
    if (msg.status === 'error') {
      console.warn('[Engine] Error:', msg.message);
      return;
    }

    // ── search result ──
    if (msg.eval !== undefined) {
      setEngineEval(msg.eval);
      setIsThinking(false);
      setIsAnalyzing(false);

      const mode = searchModeRef.current;
      const engineColor = playerColorRef.current === 'w' ? 'b' : 'w';

      if (mode === 'hint') {
        // This was a hint search for the player — store top lines
        console.log('[App] Hint received:', msg.topMoves?.length, 'lines');
        setTopLines(msg.topMoves || []);
        setBestMoveStr(msg.bestmove || null);
        return;
      }

      // Engine move search
      setBestMoveStr(msg.bestmove || null);

      if (msg.bestmove && gameRef.current.turn() === engineColor && gameStatusRef.current === 'playing') {
        const from = msg.bestmove.slice(0, 2);
        const to = msg.bestmove.slice(2, 4);
        const promotion = msg.bestmove.length > 4 ? msg.bestmove[4] : undefined;

        const moveObj = { from, to };
        if (promotion) moveObj.promotion = promotion;

        try {
          const result = gameRef.current.move(moveObj);
          if (result) {
            const entry = { san: result.san, color: engineColor };
            moveHistoryRef.current = [...moveHistoryRef.current, entry];
            setMoveHistory([...moveHistoryRef.current]);
            setLastMoveSquares({
              [from]: { backgroundColor: 'rgba(255, 255, 0, 0.3)' },
              [to]: { backgroundColor: 'rgba(255, 255, 0, 0.3)' },
            });

            // Tell the engine to apply this move on its board too
            if (wsRef.current && wsRef.current.readyState === 1) {
              wsRef.current.send('move ' + msg.bestmove);
            }
            syncGameToState();
          } else {
            console.error('[Engine] bestmove was not legal:', msg.bestmove);
          }
        } catch (e) {
          console.error('[Engine] Failed to apply bestmove:', msg.bestmove, e);
        }
      }
      return;
    }
  };

  // ─── Process incoming WS message ───────────────────────────────────────────
  // Each WS message from our server is already a validated, complete JSON line.
  // No cross-message buffering needed.
  const processRawData = useCallback((raw) => {
    // Handle potential multi-line messages (rare but possible)
    const lines = raw.split('\n');
    for (const line of lines) {
      const trimmed = line.trim();
      if (!trimmed) continue;
      try {
        const parsed = JSON.parse(trimmed);
        handleEngineMessageRef.current?.(parsed);
      } catch (e) {
        console.warn('[WS] Non-JSON from server:', trimmed);
      }
    }
  }, []);

  // ─── WebSocket lifecycle ──────────────────────────────────────────────────
  useEffect(() => {
    function connect() {
      if (wsRef.current) {
        wsRef.current.onopen = null;
        wsRef.current.onmessage = null;
        wsRef.current.onerror = null;
        wsRef.current.onclose = null;
        if (wsRef.current.readyState < 2) { // 2 = CLOSING
          wsRef.current.close();
        }
      }

      console.log('[WS] Connecting to', WS_URL);
      const ws = new WebSocket(WS_URL);
      wsRef.current = ws;

      ws.onopen = () => {
        console.log('[WS] Connected');
        setWsConnected(true);
        // Only send newgame if we're already in-game
        if (screenRef.current === 'game') {
          ws.send('newgame');
        }
      };

      ws.onmessage = (event) => {
        processRawData(event.data);
      };

      ws.onerror = (err) => console.error('[WS] Error:', err);

      ws.onclose = () => {
        console.log('[WS] Closed');
        setWsConnected(false);
        setIsThinking(false);
        if (reconnectTimerRef.current) clearTimeout(reconnectTimerRef.current);
        reconnectTimerRef.current = setTimeout(connect, RECONNECT_DELAY_MS);
      };
    }

    connect();

    return () => {
      if (reconnectTimerRef.current) clearTimeout(reconnectTimerRef.current);
      if (wsRef.current) {
        wsRef.current.onclose = null;
        wsRef.current.close();
      }
    };
  }, [processRawData]);

  // ─── Start a game ─────────────────────────────────────────────────────────
  function startGame(color, mode = 'engine') {
    setPlayerColor(color);
    playerColorRef.current = color;
    setGameMode(mode);
    gameModeRef.current = mode;
    setScreen('game');
    screenRef.current = 'game';

    // Reset local game
    const freshGame = new Chess();
    gameRef.current = freshGame;
    moveHistoryRef.current = [];
    setMoveHistory([]);
    setEngineEval(0);
    setBestMoveStr(null);
    setTopLines([]);
    setIsAnalyzing(false);
    setIsThinking(false);
    setGameStatus('playing');
    gameStatusRef.current = 'playing';
    setStatusMessage('');
    setLastMoveSquares({});
    setFen(freshGame.fen());

    setFen(freshGame.fen());

    // Tell engine to reset
    wsSend('newgame');
  }

  // ─── New game (from in-game) ──────────────────────────────────────────────
  function handleNewGame() {
    setScreen('menu');
    screenRef.current = 'menu';
    setIsThinking(false);
  }

  // ─── Human drops a piece ──────────────────────────────────────────────────
  function onDrop({ sourceSquare, targetSquare, piece }) {
    if (!targetSquare) return false;
    if (gameStatusRef.current !== 'playing') return false;
    // In engine mode, only allow the player's color; in PvP, allow both
    if (gameModeRef.current === 'engine' && gameRef.current.turn() !== playerColorRef.current) return false;

    const pieceType = piece?.pieceType || piece || '';
    const currentTurn = gameRef.current.turn();

    // Determine promotion (white pawn to rank 8, black pawn to rank 1)
    const isPromotion =
      (currentTurn === 'w' && (pieceType === 'wP' || pieceType === 'P') && targetSquare[1] === '8') ||
      (currentTurn === 'b' && (pieceType === 'bP' || pieceType === 'p') && targetSquare[1] === '1');

    const moveObj = {
      from: sourceSquare,
      to: targetSquare,
      promotion: isPromotion ? 'q' : undefined,
    };

    try {
      const result = gameRef.current.move(moveObj);
      if (!result) return false;

      // Clear hints since player just moved
      setTopLines([]);

      // Record in history
      const entry = { san: result.san, color: currentTurn };
      moveHistoryRef.current = [...moveHistoryRef.current, entry];
      setMoveHistory([...moveHistoryRef.current]);

      // Highlight last move
      setLastMoveSquares({
        [sourceSquare]: { backgroundColor: 'rgba(255, 255, 0, 0.3)' },
        [targetSquare]: { backgroundColor: 'rgba(255, 255, 0, 0.3)' },
      });

      syncGameToState();

      // Always tell the engine about the move (state sync for analysis)
      const moveStr = `${sourceSquare}${targetSquare}${isPromotion ? 'q' : ''}`;
      wsSend(`move ${moveStr}`);

      return true;
    } catch (e) {
      return false;
    }
  }

  // ─── Helpers ──────────────────────────────────────────────────────────────
  const evalDisplay = (engineEval / 100).toFixed(2);
  const evalForBar = playerColor === 'w' ? engineEval : -engineEval;
  const evalBarPercent = Math.min(Math.max(50 + (evalForBar / 10), 2), 98);

  const movePairs = [];
  for (let i = 0; i < moveHistory.length; i += 2) {
    movePairs.push({
      num: Math.floor(i / 2) + 1,
      white: moveHistory[i]?.san || '',
      black: moveHistory[i + 1]?.san || '',
    });
  }

  const currentTurnColor = gameRef.current.turn();
  const isPlayerTurn = gameMode === 'pvp' || currentTurnColor === playerColor;
  const isPvP = gameMode === 'pvp';
  const topLabel = isPvP ? 'Black' : (playerColor === 'w' ? 'Engine (Black)' : 'Engine (White)');
  const bottomLabel = isPvP ? 'White' : (playerColor === 'w' ? 'You (White)' : 'You (Black)');

  // ─── Menu Screen ──────────────────────────────────────────────────────────
  if (screen === 'menu') {
    return (
      <div style={styles.menuContainer}>
        <div style={styles.menuCard}>
          <div style={styles.menuIcon}>♔</div>
          <h1 style={styles.menuTitle}>Chess Engine</h1>
          <p style={styles.menuSubtitle}>Play against a custom C++ chess engine or a friend</p>

          {/* PvP Button */}
          <button
            onClick={() => startGame('w', 'pvp')}
            style={styles.menuBtnPvP}
            onMouseOver={(e) => {
              e.currentTarget.style.transform = 'translateY(-3px) scale(1.03)';
              e.currentTarget.style.boxShadow = '0 8px 30px rgba(139, 92, 246, 0.2)';
            }}
            onMouseOut={(e) => {
              e.currentTarget.style.transform = 'translateY(0) scale(1)';
              e.currentTarget.style.boxShadow = '0 4px 16px rgba(0,0,0,0.3)';
            }}
            id="play-pvp"
          >
            <span style={{ fontSize: '24px' }}>⚔️</span>
            <span style={styles.menuBtnLabel}>Player vs Player</span>
            <span style={styles.menuBtnHint}>Play on the same board</span>
          </button>

          <div style={styles.menuDivider} />

          <p style={styles.menuChoose}>Play vs Engine</p>

          <div style={styles.menuBtnRow}>
            <button
              onClick={() => startGame('w')}
              style={styles.menuBtnWhite}
              onMouseOver={(e) => {
                e.currentTarget.style.transform = 'translateY(-3px) scale(1.03)';
                e.currentTarget.style.boxShadow = '0 8px 30px rgba(255,255,255,0.15)';
              }}
              onMouseOut={(e) => {
                e.currentTarget.style.transform = 'translateY(0) scale(1)';
                e.currentTarget.style.boxShadow = '0 4px 16px rgba(0,0,0,0.3)';
              }}
              id="play-as-white"
            >
              <span style={styles.menuBtnIcon}>♔</span>
              <span style={styles.menuBtnLabel}>Play as White</span>
              <span style={styles.menuBtnHint}>You move first</span>
            </button>

            <button
              onClick={() => startGame('b')}
              style={styles.menuBtnBlack}
              onMouseOver={(e) => {
                e.currentTarget.style.transform = 'translateY(-3px) scale(1.03)';
                e.currentTarget.style.boxShadow = '0 8px 30px rgba(100,100,255,0.15)';
              }}
              onMouseOut={(e) => {
                e.currentTarget.style.transform = 'translateY(0) scale(1)';
                e.currentTarget.style.boxShadow = '0 4px 16px rgba(0,0,0,0.3)';
              }}
              id="play-as-black"
            >
              <span style={styles.menuBtnIcon}>♚</span>
              <span style={styles.menuBtnLabel}>Play as Black</span>
              <span style={styles.menuBtnHint}>Engine moves first</span>
            </button>
          </div>

          {/* Connection status */}
          <div style={styles.menuConn}>
            <span style={{
              ...styles.connDot,
              backgroundColor: wsConnected ? '#4ade80' : '#f87171',
              boxShadow: wsConnected
                ? '0 0 8px rgba(74, 222, 128, 0.6)'
                : '0 0 8px rgba(248, 113, 113, 0.6)',
            }} />
            <span style={styles.connText}>
              {wsConnected ? 'Engine ready' : 'Connecting…'}
            </span>
          </div>
        </div>
      </div>
    );
  }

  // ─── Game Screen ──────────────────────────────────────────────────────────
  return (
    <div style={styles.container}>
      {/* ── Sidebar ── */}
      <div style={styles.sidebar}>
        <div style={styles.sidebarInner}>
          {/* Title */}
          <div style={styles.titleBlock}>
            <span style={styles.titleIcon}>♔</span>
            <h1 style={styles.title}>Chess Engine</h1>
          </div>

          {/* Connection */}
          <div style={styles.connectionRow}>
            <span style={{
              ...styles.connDot,
              backgroundColor: wsConnected ? '#4ade80' : '#f87171',
              boxShadow: wsConnected
                ? '0 0 8px rgba(74, 222, 128, 0.6)'
                : '0 0 8px rgba(248, 113, 113, 0.6)',
            }} />
            <span style={styles.connText}>
              {wsConnected ? 'Engine connected' : 'Reconnecting…'}
            </span>
          </div>

          {/* Turn / Status */}
          <div style={styles.card}>
            <div style={styles.cardLabel}>Status</div>
            <div style={{
              ...styles.statusValue,
              color: isThinking ? '#a78bfa'
                : gameStatus !== 'playing' ? '#fbbf24'
                  : (currentTurnColor === 'w') ? '#e4e4e7' : '#60a5fa',
            }}>
              {isThinking
                ? (isPvP ? '⏳ Analyzing…' : '⏳ Engine thinking…')
                : gameStatus === 'checkmate' ? '♚ Checkmate'
                  : gameStatus === 'stalemate' ? '½ Stalemate'
                    : gameStatus === 'draw' ? '½ Draw'
                      : isPvP ? (currentTurnColor === 'w' ? '● White\'s turn' : '● Black\'s turn')
                        : isPlayerTurn ? '● Your turn' : '● Engine\'s turn'}
            </div>
            {statusMessage && <div style={styles.statusMsg}>{statusMessage}</div>}
          </div>

          {/* Evaluation */}
          <div style={styles.card}>
            <div style={styles.cardLabel}>Evaluation</div>
            <div style={styles.evalBarOuter}>
              <div style={{
                ...styles.evalBarInner,
                width: `${evalBarPercent}%`,
              }} />
              <div style={styles.evalText}>
                {evalDisplay > 0 ? '+' : ''}{evalDisplay}
              </div>
            </div>
            <div style={styles.evalSubtext}>
              {evalForBar > 50 ? 'White is winning' : evalForBar < -50 ? 'Black is winning' : 'Position is equal'}
            </div>
          </div>

          {/* Top 3 Analysis Lines */}
          <div style={{
            ...styles.card,
            border: topLines.length > 0 && isPlayerTurn ? '1px solid rgba(74, 222, 128, 0.3)' : '1px solid #27272a',
            background: topLines.length > 0 && isPlayerTurn
              ? 'linear-gradient(135deg, rgba(74, 222, 128, 0.04), rgba(24,24,27,1))'
              : '#1f1f23',
          }}>
            <div style={styles.cardLabel}>
              {isAnalyzing ? '⏳ Analyzing…' : '💡 Best Lines'}
            </div>
            {isAnalyzing && (
              <div style={{ color: '#71717a', fontSize: '13px', fontStyle: 'italic', padding: '4px 0' }}>Computing…</div>
            )}
            {!isAnalyzing && topLines.length === 0 && (
              <div style={{ color: '#52525b', fontSize: '13px', fontStyle: 'italic', padding: '4px 0' }}>Waiting for analysis</div>
            )}
            {!isAnalyzing && topLines.map((entry, idx) => {
              const evalCp = entry.score;
              const evalDisp = (evalCp / 100).toFixed(1);
              const isPositive = (playerColor === 'w') ? evalCp >= 0 : evalCp <= 0;
              return (
                <div
                  key={idx}
                  style={{
                    display: 'flex',
                    alignItems: 'flex-start',
                    gap: '8px',
                    padding: '8px 10px',
                    marginTop: idx > 0 ? '4px' : '0',
                    borderRadius: '6px',
                    backgroundColor: idx === 0 ? 'rgba(74, 222, 128, 0.08)' : 'rgba(39, 39, 42, 0.5)',
                    border: idx === 0 ? '1px solid rgba(74, 222, 128, 0.15)' : '1px solid transparent',
                    transition: 'all 0.2s ease',
                  }}
                >
                  {/* Rank badge */}
                  <span style={{
                    width: '22px',
                    height: '22px',
                    borderRadius: '50%',
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'center',
                    fontSize: '11px',
                    fontWeight: 700,
                    flexShrink: 0,
                    backgroundColor: idx === 0 ? 'rgba(74, 222, 128, 0.2)' : 'rgba(63, 63, 70, 0.6)',
                    color: idx === 0 ? '#4ade80' : '#71717a',
                  }}>{idx + 1}</span>

                  {/* Evaluation chip */}
                  <span style={{
                    fontSize: '12px',
                    fontWeight: 700,
                    fontFamily: "'JetBrains Mono', 'Fira Code', monospace",
                    color: isPositive ? '#4ade80' : '#f87171',
                    backgroundColor: isPositive ? 'rgba(74, 222, 128, 0.1)' : 'rgba(248, 113, 113, 0.1)',
                    padding: '2px 6px',
                    borderRadius: '4px',
                    flexShrink: 0,
                    minWidth: '42px',
                    textAlign: 'center',
                  }}>
                    {evalCp >= 0 ? '+' : ''}{evalDisp}
                  </span>

                  {/* Move line */}
                  <span style={{
                    fontSize: '11px',
                    fontFamily: "'JetBrains Mono', 'Fira Code', monospace",
                    color: '#a1a1aa',
                    lineHeight: '1.5',
                    wordBreak: 'break-word',
                  }}>
                    {entry.line.join(' → ')}
                  </span>
                </div>
              );
            })}
          </div>

          {/* Move history */}
          <div style={{ ...styles.card, flex: 1, overflow: 'hidden', display: 'flex', flexDirection: 'column' }}>
            <div style={styles.cardLabel}>Move History</div>
            <div style={styles.moveListHeader}>
              <span style={styles.moveHeaderNum}>#</span>
              <span style={styles.moveHeaderCol}>White</span>
              <span style={styles.moveHeaderCol}>Black</span>
            </div>
            <div style={styles.moveList} id="move-list">
              {movePairs.length === 0 && (
                <div style={styles.emptyMoves}>No moves yet</div>
              )}
              {movePairs.map((pair) => (
                <div key={pair.num} style={styles.moveRow}>
                  <span style={styles.moveNum}>{pair.num}.</span>
                  <span style={styles.moveWhite}>{pair.white}</span>
                  <span style={styles.moveBlack}>{pair.black}</span>
                </div>
              ))}
            </div>
          </div>
        </div>

        {/* Bottom button */}
        <button
          onClick={handleNewGame}
          style={styles.newGameBtn}
          onMouseOver={(e) => {
            e.currentTarget.style.background = 'linear-gradient(135deg, #6366f1, #8b5cf6)';
            e.currentTarget.style.transform = 'translateY(-1px)';
          }}
          onMouseOut={(e) => {
            e.currentTarget.style.background = 'linear-gradient(135deg, #4f46e5, #7c3aed)';
            e.currentTarget.style.transform = 'translateY(0)';
          }}
          id="new-game-button"
        >
          ↻ New Game
        </button>
      </div>

      {/* ── Board area ── */}
      <div style={styles.boardArea}>
        {/* Top player (opponent or Black in PvP) */}
        <div style={styles.playerBar}>
          <span style={styles.playerDot}>{isPvP ? '⚫' : (playerColor === 'w' ? '⚫' : '⚪')}</span>
          <span style={styles.playerName}>{topLabel}</span>
          {isThinking && !isPvP && <div style={styles.thinkingDots}>
            <span style={styles.dot1} />
            <span style={styles.dot2} />
            <span style={styles.dot3} />
          </div>}
          {isPvP && currentTurnColor === 'b' && gameStatus === 'playing' && (
            <span style={styles.yourTurnBadge}>Turn</span>
          )}
        </div>

        <div style={styles.boardWrapper} id="chess-board">
          <Chessboard
            options={{
              position: fen,
              onPieceDrop: onDrop,
              boardOrientation: isPvP ? 'white' : (playerColor === 'w' ? 'white' : 'black'),
              animationDurationInMs: 250,
              allowDragging: gameStatus === 'playing' && !isThinking,
              darkSquareStyle: { backgroundColor: '#779952' },
              lightSquareStyle: { backgroundColor: '#e9edcc' },
              dropSquareStyle: { boxShadow: 'inset 0 0 1px 6px rgba(99, 102, 241, 0.6)' },
              squareStyles: lastMoveSquares,
              boardStyle: {
                borderRadius: '6px',
                boxShadow: '0 12px 48px rgba(0, 0, 0, 0.55)',
              },
            }}
          />
        </div>

        {/* Bottom player (you or White in PvP) */}
        <div style={styles.playerBar}>
          <span style={styles.playerDot}>{isPvP ? '⚪' : (playerColor === 'w' ? '⚪' : '⚫')}</span>
          <span style={styles.playerName}>{bottomLabel}</span>
          {isPvP && currentTurnColor === 'w' && gameStatus === 'playing' && (
            <span style={styles.yourTurnBadge}>Turn</span>
          )}
          {!isPvP && isPlayerTurn && gameStatus === 'playing' && !isThinking && (
            <span style={styles.yourTurnBadge}>Your turn</span>
          )}
        </div>
      </div>
    </div>
  );
}

// ─── Styles ──────────────────────────────────────────────────────────────────
const styles = {
  // ── Menu ──
  menuContainer: {
    display: 'flex',
    height: '100vh',
    width: '100vw',
    alignItems: 'center',
    justifyContent: 'center',
    background: 'radial-gradient(ellipse at center, #1a1a2e 0%, #0f0f11 70%)',
    fontFamily: "'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif",
  },
  menuCard: {
    textAlign: 'center',
    padding: '48px 56px',
    borderRadius: '20px',
    backgroundColor: 'rgba(24, 24, 27, 0.85)',
    border: '1px solid #27272a',
    backdropFilter: 'blur(16px)',
    boxShadow: '0 24px 80px rgba(0, 0, 0, 0.5)',
    maxWidth: '520px',
    width: '100%',
  },
  menuIcon: {
    fontSize: '56px',
    marginBottom: '8px',
    filter: 'drop-shadow(0 0 12px rgba(99, 102, 241, 0.4))',
  },
  menuTitle: {
    margin: '0 0 8px',
    fontSize: '32px',
    fontWeight: 700,
    letterSpacing: '-0.5px',
    background: 'linear-gradient(135deg, #e4e4e7, #a1a1aa)',
    WebkitBackgroundClip: 'text',
    WebkitTextFillColor: 'transparent',
  },
  menuSubtitle: {
    margin: 0,
    fontSize: '15px',
    color: '#71717a',
  },
  menuDivider: {
    height: '1px',
    background: 'linear-gradient(90deg, transparent, #3f3f46, transparent)',
    margin: '28px 0',
  },
  menuChoose: {
    margin: '0 0 20px',
    fontSize: '14px',
    fontWeight: 600,
    color: '#a1a1aa',
    textTransform: 'uppercase',
    letterSpacing: '1.5px',
  },
  menuBtnRow: {
    display: 'flex',
    gap: '16px',
    justifyContent: 'center',
  },
  menuBtnWhite: {
    flex: 1,
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    gap: '6px',
    padding: '24px 20px',
    background: 'linear-gradient(160deg, #fafaf9, #d4d4d8)',
    color: '#18181b',
    border: 'none',
    borderRadius: '14px',
    cursor: 'pointer',
    transition: 'all 0.25s cubic-bezier(0.23, 1, 0.32, 1)',
    boxShadow: '0 4px 16px rgba(0,0,0,0.3)',
  },
  menuBtnBlack: {
    flex: 1,
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    gap: '6px',
    padding: '24px 20px',
    background: 'linear-gradient(160deg, #27272a, #18181b)',
    color: '#e4e4e7',
    border: '1px solid #3f3f46',
    borderRadius: '14px',
    cursor: 'pointer',
    transition: 'all 0.25s cubic-bezier(0.23, 1, 0.32, 1)',
    boxShadow: '0 4px 16px rgba(0,0,0,0.3)',
  },
  menuBtnPvP: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    gap: '12px',
    width: '100%',
    padding: '18px 24px',
    marginTop: '24px',
    background: 'linear-gradient(135deg, #4f46e5, #7c3aed)',
    color: '#fff',
    border: 'none',
    borderRadius: '14px',
    cursor: 'pointer',
    transition: 'all 0.25s cubic-bezier(0.23, 1, 0.32, 1)',
    boxShadow: '0 4px 16px rgba(0,0,0,0.3)',
    fontSize: '16px',
  },
  menuBtnIcon: {
    fontSize: '36px',
  },
  menuBtnLabel: {
    fontSize: '16px',
    fontWeight: 700,
  },
  menuBtnHint: {
    fontSize: '12px',
    opacity: 0.6,
  },
  menuConn: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    gap: '8px',
    marginTop: '28px',
  },

  // ── Game layout ──
  container: {
    display: 'flex',
    height: '100vh',
    width: '100vw',
    backgroundColor: '#0f0f11',
    color: '#e4e4e7',
    fontFamily: "'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif",
    overflow: 'hidden',
  },

  // ── Sidebar ──
  sidebar: {
    width: '320px',
    minWidth: '320px',
    backgroundColor: '#18181b',
    borderRight: '1px solid #27272a',
    display: 'flex',
    flexDirection: 'column',
    padding: '24px 18px 18px',
    overflowY: 'auto',
  },
  sidebarInner: {
    flex: 1,
    display: 'flex',
    flexDirection: 'column',
    gap: '14px',
    overflow: 'hidden',
  },
  titleBlock: {
    display: 'flex',
    alignItems: 'center',
    gap: '10px',
    marginBottom: '4px',
  },
  titleIcon: {
    fontSize: '28px',
    filter: 'drop-shadow(0 0 6px rgba(99, 102, 241, 0.5))',
  },
  title: {
    margin: 0,
    fontSize: '22px',
    fontWeight: 700,
    letterSpacing: '-0.3px',
    background: 'linear-gradient(135deg, #e4e4e7, #a1a1aa)',
    WebkitBackgroundClip: 'text',
    WebkitTextFillColor: 'transparent',
  },
  connectionRow: {
    display: 'flex',
    alignItems: 'center',
    gap: '8px',
    padding: '6px 0',
  },
  connDot: {
    width: '8px',
    height: '8px',
    borderRadius: '50%',
    display: 'inline-block',
    transition: 'background-color 0.3s, box-shadow 0.3s',
  },
  connText: {
    fontSize: '12px',
    color: '#71717a',
    fontWeight: 500,
  },

  // Cards
  card: {
    padding: '14px',
    borderRadius: '10px',
    backgroundColor: '#1f1f23',
    border: '1px solid #27272a',
  },
  cardLabel: {
    fontSize: '11px',
    fontWeight: 600,
    textTransform: 'uppercase',
    letterSpacing: '0.8px',
    color: '#71717a',
    marginBottom: '8px',
  },
  statusValue: {
    fontSize: '15px',
    fontWeight: 600,
    transition: 'color 0.3s',
  },
  statusMsg: {
    fontSize: '12px',
    color: '#a1a1aa',
    marginTop: '6px',
  },
  evalBarOuter: {
    height: '36px',
    backgroundColor: '#27272a',
    borderRadius: '6px',
    position: 'relative',
    overflow: 'hidden',
    display: 'flex',
    alignItems: 'center',
  },
  evalBarInner: {
    height: '100%',
    background: 'linear-gradient(90deg, #fafaf9, #d4d4d8)',
    transition: 'width 0.6s cubic-bezier(0.23, 1, 0.32, 1)',
    borderRadius: '6px 0 0 6px',
  },
  evalText: {
    position: 'absolute',
    width: '100%',
    textAlign: 'center',
    fontWeight: 700,
    fontSize: '14px',
    fontFamily: "'JetBrains Mono', 'Fira Code', monospace",
    color: '#e4e4e7',
    mixBlendMode: 'difference',
    zIndex: 1,
  },
  evalSubtext: {
    fontSize: '11px',
    color: '#71717a',
    marginTop: '6px',
    textAlign: 'center',
  },
  bestMove: {
    fontSize: '26px',
    fontWeight: 700,
    fontFamily: "'JetBrains Mono', 'Fira Code', monospace",
    color: '#818cf8',
    letterSpacing: '1px',
  },

  // Move history
  moveListHeader: {
    display: 'flex',
    gap: '4px',
    padding: '0 6px 4px',
    borderBottom: '1px solid #27272a',
    marginBottom: '4px',
  },
  moveHeaderNum: {
    width: '28px',
    textAlign: 'right',
    fontSize: '10px',
    color: '#52525b',
    fontWeight: 700,
    textTransform: 'uppercase',
  },
  moveHeaderCol: {
    width: '56px',
    fontSize: '10px',
    color: '#52525b',
    fontWeight: 700,
    textTransform: 'uppercase',
  },
  moveList: {
    flex: 1,
    overflowY: 'auto',
    paddingRight: '4px',
  },
  emptyMoves: {
    fontSize: '13px',
    color: '#52525b',
    fontStyle: 'italic',
    padding: '8px 6px',
  },
  moveRow: {
    display: 'flex',
    alignItems: 'center',
    gap: '4px',
    padding: '4px 6px',
    borderRadius: '4px',
    fontSize: '13px',
    fontFamily: "'JetBrains Mono', 'Fira Code', monospace",
  },
  moveNum: {
    color: '#52525b',
    width: '28px',
    textAlign: 'right',
    flexShrink: 0,
    fontWeight: 600,
  },
  moveWhite: {
    width: '56px',
    color: '#e4e4e7',
    fontWeight: 500,
  },
  moveBlack: {
    width: '56px',
    color: '#a1a1aa',
    fontWeight: 500,
  },
  newGameBtn: {
    marginTop: '14px',
    padding: '14px',
    background: 'linear-gradient(135deg, #4f46e5, #7c3aed)',
    color: '#fff',
    border: 'none',
    borderRadius: '10px',
    fontSize: '16px',
    fontWeight: 700,
    cursor: 'pointer',
    transition: 'all 0.2s ease',
    letterSpacing: '0.3px',
    boxShadow: '0 4px 14px rgba(79, 70, 229, 0.35)',
  },

  // ── Board area ──
  boardArea: {
    flex: 1,
    display: 'flex',
    flexDirection: 'column',
    justifyContent: 'center',
    alignItems: 'center',
    padding: '32px',
    gap: '12px',
    background: 'radial-gradient(ellipse at center, #1a1a2e 0%, #0f0f11 70%)',
  },
  boardWrapper: {
    width: 'min(72vh, 680px)',
    aspectRatio: '1 / 1',
  },
  playerBar: {
    display: 'flex',
    alignItems: 'center',
    gap: '8px',
    padding: '8px 18px',
    borderRadius: '10px',
    backgroundColor: 'rgba(39, 39, 42, 0.6)',
    backdropFilter: 'blur(8px)',
    minWidth: '240px',
  },
  playerDot: {
    fontSize: '16px',
  },
  playerName: {
    fontSize: '14px',
    fontWeight: 600,
    color: '#a1a1aa',
  },
  yourTurnBadge: {
    marginLeft: 'auto',
    fontSize: '11px',
    fontWeight: 700,
    color: '#4ade80',
    backgroundColor: 'rgba(74, 222, 128, 0.1)',
    padding: '2px 10px',
    borderRadius: '20px',
    border: '1px solid rgba(74, 222, 128, 0.2)',
  },
  thinkingDots: {
    marginLeft: 'auto',
    display: 'flex',
    gap: '4px',
    alignItems: 'center',
  },
  dot1: {
    width: '6px',
    height: '6px',
    borderRadius: '50%',
    backgroundColor: '#a78bfa',
    animation: 'pulse 1.4s ease-in-out infinite',
  },
  dot2: {
    width: '6px',
    height: '6px',
    borderRadius: '50%',
    backgroundColor: '#a78bfa',
    animation: 'pulse 1.4s ease-in-out 0.2s infinite',
  },
  dot3: {
    width: '6px',
    height: '6px',
    borderRadius: '50%',
    backgroundColor: '#a78bfa',
    animation: 'pulse 1.4s ease-in-out 0.4s infinite',
  },
};

export default App;
