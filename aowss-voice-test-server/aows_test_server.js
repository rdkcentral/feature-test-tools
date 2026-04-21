#!/usr/bin/env node

/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Simple AOWS test endpoint for Control Manager.
 * - Accepts websocket (ws://) audio sessions.
 * - Writes binary audio frames to timestamped files.
 * - Sends basic text responses for diagnostics.
 */

const fs = require('fs');
const path = require('path');
const http = require('http');
const crypto = require('crypto');
const { WebSocketServer } = require('ws');

// Minimal .env loader keeps runtime dependency-free for embedded and lab hosts.
function loadEnv(filePath) {
  if (!fs.existsSync(filePath)) return;

  for (const raw of fs.readFileSync(filePath, 'utf8').split(/\r?\n/)) {
    const line = raw.trim();
    if (!line || line.startsWith('#')) continue;

    const eq = line.indexOf('=');
    if (eq < 1) continue;

    const key = line.slice(0, eq).trim();
    let value = line.slice(eq + 1).trim();
    if ((value.startsWith('"') && value.endsWith('"')) || (value.startsWith("'") && value.endsWith("'"))) {
      value = value.slice(1, -1);
    }
    if (!(key in process.env)) process.env[key] = value;
  }
}

function envBool(name, fallback = false) {
  const v = process.env[name];
  if (!v) return fallback;
  return /^(1|true|yes)$/i.test(v);
}

function envInt(name, fallback) {
  const raw = process.env[name];
  if (raw == null || raw === '') return fallback;
  const n = Number(raw);
  return Number.isFinite(n) ? n : fallback;
}

function utcStamp() {
  const d = new Date();
  const pad2 = (n) => String(n).padStart(2, '0');
  const pad3 = (n) => String(n).padStart(3, '0');
  return [
    d.getUTCFullYear(),
    pad2(d.getUTCMonth() + 1),
    pad2(d.getUTCDate())
  ].join('') + '-' + [
    pad2(d.getUTCHours()),
    pad2(d.getUTCMinutes()),
    pad2(d.getUTCSeconds())
  ].join('') + '-' + pad3(d.getUTCMilliseconds());
}

loadEnv(path.join(process.cwd(), '.env'));

// Runtime knobs let the same server work for RF4CE and BLE packet layouts.
const HOST = process.env.AOWS_HOST || '0.0.0.0';
const PORT = envInt('AOWS_PORT', 9880);
const OUT_DIR = process.env.AOWS_OUT_DIR || path.join(process.cwd(), 'aows_captures');
const PATH_FILTER = process.env.AOWS_PATH || '/';
const DECODE_ADPCM = envBool('AOWS_DECODE_ADPCM', false);
const SEND_DIAGNOSTICS = envBool('AOWS_SEND_DIAGNOSTICS', false);
const IDLE_CLOSE_MS = envInt('AOWS_IDLE_CLOSE_MS', 1000);
const NIBBLE_ORDER = (process.env.AOWS_ADPCM_NIBBLE_ORDER || 'lh').toLowerCase();
const ADPCM_FRAME_BYTES = envInt('AOWS_ADPCM_FRAME_BYTES', 95);
const ADPCM_HEADER_BYTES = envInt('AOWS_ADPCM_HEADER_BYTES', 4);
const ADPCM_OFFSET_STEP_INDEX = envInt('AOWS_ADPCM_OFFSET_STEP_INDEX', 1);
const ADPCM_OFFSET_PRED_LSB = envInt('AOWS_ADPCM_OFFSET_PRED_LSB', 2);
const ADPCM_OFFSET_PRED_MSB = envInt('AOWS_ADPCM_OFFSET_PRED_MSB', 3);
const DECODED_BYTES_PER_FRAME = (ADPCM_FRAME_BYTES - ADPCM_HEADER_BYTES) * 4;

let activeConnections = 0;
fs.mkdirSync(OUT_DIR, { recursive: true });

function logInfo(message) {
  console.log(`[${new Date().toISOString()}] ${message}`);
}

function logError(message) {
  console.error(`[${new Date().toISOString()}] ${message}`);
}

function makeFileName(remoteAddr, stamp = utcStamp()) {
  const safeAddr = (remoteAddr || 'unknown').replace(/[^a-zA-Z0-9_.-]/g, '_');
  const rand = crypto.randomBytes(2).toString('hex');
  return `audio-${stamp}-${safeAddr}-${rand}.pcm`;
}

// IMA/DVI ADPCM reference tables (public codec spec values).
// Reference: https://wiki.multimedia.cx/index.php/IMA_ADPCM
const IMA_INDEX_TABLE = [
  -1, -1, -1, -1, 2, 4, 6, 8,
  -1, -1, -1, -1, 2, 4, 6, 8
];

const IMA_STEP_TABLE = [
  7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
  19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
  50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
  130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
  337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
  876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
  2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
  5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
  15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
];

// IMA ADPCM nibble decode based on the algorithm described at:
// https://wiki.multimedia.cx/index.php/IMA_ADPCM
function decodeImaAdpcmNibbleStep(nibble, predictor, stepIndex) {
  const idx = Math.max(0, Math.min(88, stepIndex | 0));

  // Pull the lower 4-bit code and current quantizer step.
  const code = nibble & 0x0f;
  const step = IMA_STEP_TABLE[idx];

  // Split sign (3rd bit) and magnitude (bits 0-2).
  const sign = code & 0x08;
  const delta = code & 0x07;

  // Compute the predicted delta for this nibble.
  // Same result as bit-accumulation form: ((2 * delta + 1) * step) >> 3
  const diff = (((delta << 1) + 1) * step) >> 3;

  // Apply the sign bit and clamp to signed 16-bit PCM.
  let nextPredictor = predictor + (sign ? -diff : diff);
  if (nextPredictor > 32767) nextPredictor = 32767;
  if (nextPredictor < -32768) nextPredictor = -32768;

  // Advance step index from the code and clamp to table bounds.
  let nextStepIndex = idx + IMA_INDEX_TABLE[code];
  if (nextStepIndex < 0) nextStepIndex = 0;
  if (nextStepIndex > 88) nextStepIndex = 88;

  return { predictor: nextPredictor, stepIndex: nextStepIndex };
}

function decodeImaAdpcmAlignedToPcm16le(buffer) {
  // Decode only when payload boundaries are packet-aligned; otherwise keep raw bytes only.
  if (!Buffer.isBuffer(buffer) || buffer.length < ADPCM_FRAME_BYTES || (buffer.length % ADPCM_FRAME_BYTES) !== 0 || ADPCM_HEADER_BYTES >= ADPCM_FRAME_BYTES) {
    return null;
  }

  const frameCount = buffer.length / ADPCM_FRAME_BYTES;
  const out = Buffer.alloc(frameCount * DECODED_BYTES_PER_FRAME);
  let outOffset = 0;

  for (let frame = 0; frame < frameCount; frame++) {
    const start = frame * ADPCM_FRAME_BYTES;
    let stepIndex = buffer[start + ADPCM_OFFSET_STEP_INDEX];
    if (stepIndex > 88) stepIndex = 88;

    let predictor = (buffer[start + ADPCM_OFFSET_PRED_LSB] | (buffer[start + ADPCM_OFFSET_PRED_MSB] << 8));
    if (predictor & 0x8000) predictor -= 0x10000;

    for (let i = start + ADPCM_HEADER_BYTES; i < start + ADPCM_FRAME_BYTES; i++) {
      const b = buffer[i];
      const n1 = NIBBLE_ORDER === 'hl' ? (b >> 4) & 0x0f : b & 0x0f;
      const n2 = NIBBLE_ORDER === 'hl' ? b & 0x0f : (b >> 4) & 0x0f;

      let s = decodeImaAdpcmNibbleStep(n1, predictor, stepIndex);
      predictor = s.predictor;
      stepIndex = s.stepIndex;
      out.writeInt16LE(predictor, outOffset);
      outOffset += 2;

      s = decodeImaAdpcmNibbleStep(n2, predictor, stepIndex);
      predictor = s.predictor;
      stepIndex = s.stepIndex;
      out.writeInt16LE(predictor, outOffset);
      outOffset += 2;
    }
  }

  return out;
}

function tryParseJson(value) {
  try {
    return JSON.parse(value);
  } catch (err) {
    return null;
  }
}

function handleBinaryFrame(data, out, outDecoded, counters, ioFailed) {
  if (ioFailed.value) return;

  // Persist original bytes first so packet-loss or decode failures never lose evidence.
  if (!out.destroyed && out.writable) {
    out.write(data);
  }

  if (outDecoded) {
    const decoded = decodeImaAdpcmAlignedToPcm16le(data);
    // Non-aligned chunks are expected in some transports; keep capture and skip decode for that chunk.
    if (decoded && !outDecoded.destroyed && outDecoded.writable) outDecoded.write(decoded);
  }

  counters.bytes += data.length;
  counters.frames += 1;
}

function handleTextFrame(ws, txt, counters, replyEnabled) {
  const trimmed = txt.trim();
  if (!trimmed) {
    if (replyEnabled) ws.send(JSON.stringify({ event: 'ack', received: '', bytes: counters.bytes, frames: counters.frames }));
    return;
  }

  if (trimmed.toLowerCase() === 'stop') {
    ws.close(1000, 'OK');
    return;
  }

  const msg = tryParseJson(trimmed);
  if (msg && typeof msg.event === 'string') {
    switch (msg.event) {
      case 'stop':
        ws.close(1000, 'OK');
        return;
      case 'ping':
        if (replyEnabled) ws.send(JSON.stringify({ event: 'pong', ts_utc: new Date().toISOString() }));
        return;
      case 'stats':
        if (replyEnabled) ws.send(JSON.stringify({ event: 'stats', bytes: counters.bytes, frames: counters.frames }));
        return;
      default:
        if (replyEnabled) ws.send(JSON.stringify({ event: 'error', message: `unsupported_event:${msg.event}` }));
        return;
    }
  }

  // Backward-compatible control-plane reply for plain text messages.
  if (replyEnabled) ws.send(JSON.stringify({ event: 'ack', received: txt, bytes: counters.bytes, frames: counters.frames }));
}

const server = http.createServer((req, res) => {
  if (req.url === '/health') {
    res.writeHead(200, { 'content-type': 'application/json' });
    res.end(JSON.stringify({ ok: true, service: 'aows-test-endpoint' }));
    return;
  }

  // Non-upgrade requests receive an explicit response.
  res.writeHead(426, {
    'content-type': 'text/plain',
    'upgrade': 'websocket',
    'connection': 'Upgrade'
  });
  res.end('Upgrade Required: websocket\n');
});

const wss = new WebSocketServer({ noServer: true });

// HTTP handles health checks; websocket upgrades carry audio frames.
server.on('upgrade', (req, socket, head) => {
  if (PATH_FILTER !== '/' && req.url !== PATH_FILTER) {
    socket.write('HTTP/1.1 404 Not Found\r\n\r\n');
    socket.destroy();
    return;
  }

  wss.handleUpgrade(req, socket, head, (ws) => {
    wss.emit('connection', ws, req);
  });
});

wss.on('connection', (ws, req) => {
  const remoteAddr = req.socket.remoteAddress;
  const remotePort = req.socket.remotePort;
  const requestPath = req.url || '/';
  const captureStamp = utcStamp();
  const filename = makeFileName(remoteAddr, captureStamp);
  const fullPath = path.join(OUT_DIR, filename);
  // Derive a per-session tag to avoid filename collisions between concurrent connections.
  const remoteTag = (remoteAddr || 'unknown').replace(/[^0-9A-Za-z_.-]/g, '_');
  const sessionRand = crypto.randomBytes(4).toString('hex');
  const sessionTag = `${remoteTag}-${sessionRand}`;
  const out = fs.createWriteStream(fullPath, { flags: 'a' });
  const decodedPath = path.join(OUT_DIR, `audio-${captureStamp}-${sessionTag}-adpcm-decoded-s16le.pcm`);
  // Decoded output is optional and meant for quick listening/inspection of ADPCM sessions.
  const outDecoded = DECODE_ADPCM ? fs.createWriteStream(decodedPath, { flags: 'a' }) : null;
  const ioFailed = { value: false };
  let idleCloseTimer = null;

  function attachStreamErrorHandler(stream, filePath) {
    if (!stream) return;
    stream.on('error', (err) => {
      ioFailed.value = true;
      logError(`[fs-error] remote=${remoteAddr}:${remotePort} file=${filePath} err=${err.message}`);
      try {
        stream.destroy();
      } catch (_) {}
      if (ws.readyState === 1) {
        ws.close(1011, 'io_error');
      }
    });
  }

  attachStreamErrorHandler(out, fullPath);
  attachStreamErrorHandler(outDecoded, decodedPath);

  function armIdleCloseTimer() {
    if (IDLE_CLOSE_MS <= 0) return;
    if (idleCloseTimer) clearTimeout(idleCloseTimer);
    idleCloseTimer = setTimeout(() => {
      if (ws.readyState === 1) {
        ws.close(1000, 'idle_eos');
      }
    }, IDLE_CLOSE_MS);
  }

  const counters = { bytes: 0, frames: 0 };

  activeConnections += 1;
  logInfo(`[connect] remote=${remoteAddr}:${remotePort} path=${requestPath} active=${activeConnections}`);
  logInfo(`[open] remote=${remoteAddr} file=${fullPath}`);
  if (DECODE_ADPCM) {
    logInfo(`[decode] adpcm->pcm16le enabled file=${decodedPath} nibble_order=${NIBBLE_ORDER}`);
  }
  logInfo(`[session] reply_msgs=${SEND_DIAGNOSTICS ? 'enabled' : 'disabled'} idle_close_ms=${IDLE_CLOSE_MS}`);

  // Some sink-mode clients do not register a receive callback, so diagnostics are opt-in.
  if (SEND_DIAGNOSTICS) {
    ws.send(JSON.stringify({ event: 'session_open', file: filename, ts_utc: new Date().toISOString() }));
  }

  ws.on('message', (data, isBinary) => {
    if (isBinary) {
      handleBinaryFrame(data, out, outDecoded, counters, ioFailed);
      armIdleCloseTimer();
      return;
    }

    handleTextFrame(ws, data.toString('utf8'), counters, SEND_DIAGNOSTICS);
  });

  ws.on('error', (err) => {
    logError(`[ws-error] remote=${remoteAddr} err=${err.message}`);
  });

  ws.on('close', (code, reason) => {
    activeConnections = Math.max(0, activeConnections - 1);
    if (idleCloseTimer) clearTimeout(idleCloseTimer);
    out.end();
    if (outDecoded) outDecoded.end();

    logInfo(`[disconnect] remote=${remoteAddr}:${remotePort} path=${requestPath} active=${activeConnections}`);
    logInfo(`[close] remote=${remoteAddr} code=${code} reason=${reason.toString()} bytes=${counters.bytes} frames=${counters.frames} file=${fullPath}${outDecoded ? ` decoded=${decodedPath}` : ''}`);
  });
});

server.listen(PORT, HOST, () => {
  logInfo(`AOWS test endpoint listening on ws://${HOST}:${PORT}${PATH_FILTER}`);
  logInfo(`Saving audio to: ${OUT_DIR}`);
  logInfo(`Reply messages to clients: ${SEND_DIAGNOSTICS ? 'enabled' : 'disabled'}`);
  logInfo(`Session idle auto-close: ${IDLE_CLOSE_MS} ms (set 0 to disable)`);
  logInfo(`ADPCM frame/header config: frame=${ADPCM_FRAME_BYTES} header=${ADPCM_HEADER_BYTES} step_idx=${ADPCM_OFFSET_STEP_INDEX} pred_lsb=${ADPCM_OFFSET_PRED_LSB} pred_msb=${ADPCM_OFFSET_PRED_MSB}`);
  logInfo(`ADPCM decode to PCM16LE: ${DECODE_ADPCM ? 'enabled' : 'disabled'}`);
  logInfo('Health check: GET /health');
});
