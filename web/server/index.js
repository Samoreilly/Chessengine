const express = require('express');
const { WebSocketServer } = require('ws');
const { spawn } = require('child_process');
const path = require('path');
const http = require('http');

const app = express();
const server = http.createServer(app);
const wss = new WebSocketServer({ server });

const ENGINE_PATH = path.resolve(__dirname, '../../');
const ENGINE_BINARY = path.join(ENGINE_PATH, 'ChessEngine');

// Serve static frontend files (production build)
app.use(express.static(path.join(__dirname, '../client/dist')));

wss.on('connection', (ws) => {
    console.log('[WSS] Client connected');

    // Spawn engine instance for this client
    let engine;
    try {
        engine = spawn(ENGINE_BINARY, ['--api']);
    } catch (err) {
        console.error('[WSS] Failed to spawn engine:', err);
        ws.send(JSON.stringify({ status: 'error', message: 'Engine failed to start' }));
        ws.close();
        return;
    }

    let stdoutBuffer = '';

    engine.stdout.on('data', (data) => {
        // Buffer incoming data and split by newlines to get complete JSON lines
        stdoutBuffer += data.toString();

        const lines = stdoutBuffer.split('\n');
        // Keep last (potentially incomplete) chunk in buffer
        stdoutBuffer = lines.pop() || '';

        for (const line of lines) {
            const trimmed = line.trim();
            if (!trimmed) continue;

            console.log(`[Engine →] ${trimmed}`);

            // Validate it's parseable JSON before forwarding
            try {
                JSON.parse(trimmed);
                ws.send(trimmed);
            } catch (e) {
                // Non-JSON output from engine (debug prints, etc.) — skip it
                console.warn(`[Engine] Non-JSON output skipped: ${trimmed}`);
            }
        }
    });

    engine.stderr.on('data', (data) => {
        console.error(`[Engine stderr] ${data}`);
    });

    engine.on('error', (err) => {
        console.error('[Engine] Spawn error:', err);
        try {
            ws.send(JSON.stringify({ status: 'error', message: 'Engine process error' }));
        } catch (_) { /* ws might be closed */ }
    });

    engine.on('close', (code) => {
        console.log(`[Engine] Exited with code ${code}`);
        try {
            ws.send(JSON.stringify({ status: 'engine_closed', code }));
            ws.close();
        } catch (_) { /* ws might already be closed */ }
    });

    ws.on('message', (message) => {
        const msg = message.toString().trim();
        if (!msg) return;
        console.log(`[Client →] ${msg}`);

        // Forward to engine stdin
        try {
            engine.stdin.write(msg + '\n');
        } catch (err) {
            console.error('[Engine] Failed to write to stdin:', err);
        }
    });

    ws.on('close', () => {
        console.log('[WSS] Client disconnected');
        try {
            engine.kill();
        } catch (_) { /* already dead */ }
    });
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
    console.log(`[Server] Running on http://localhost:${PORT}`);
});
