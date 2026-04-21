/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
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

const test = require('node:test');
const assert = require('node:assert/strict');
const path = require('path');
const fs = require('fs');
const { WebSocket } = require('ws');
const {
  waitForHealth,
  launchServer,
  openWs,
  waitForWsClose
} = require('./test_utils');

async function waitForDecodedPcmFile(
  outDir,
  timeoutMs = 5000,
  intervalMs = 100,
  stableChecksRequired = 2
) {
  const deadline = Date.now() + timeoutMs;
  let lastSize = null;
  let stableChecks = 0;

  while (Date.now() < deadline) {
    const files = fs.readdirSync(outDir);
    const decoded = files.find((name) => name.includes('adpcm-decoded-s16le.pcm'));

    if (decoded) {
      const fullPath = path.join(outDir, decoded);
      const stats = fs.statSync(fullPath);
      const size = stats.size;

      if (size === lastSize) {
        stableChecks += 1;
        if (stableChecks >= stableChecksRequired) {
          // Confirm one more interval of no growth to avoid burst-write plateaus.
          await new Promise((resolve) => setTimeout(resolve, intervalMs));
          const confirmStats = fs.statSync(fullPath);
          if (confirmStats.size === size) {
            return fullPath;
          }
          lastSize = confirmStats.size;
          stableChecks = 1;
        }
      } else {
        lastSize = size;
        stableChecks = 1;
      }
    }

    await new Promise((resolve) => setTimeout(resolve, intervalMs));
  }

  throw new Error('Timed out waiting for decoded PCM output file to become available and stable in size');
}

async function waitForCaptureFile(outDir, expectedSize, timeoutMs = 5000, intervalMs = 50) {
  const deadline = Date.now() + timeoutMs;

  while (Date.now() < deadline) {
    const files = fs.readdirSync(outDir);
    const capture = files.find((name) => /audio-.*\.pcm$/.test(name) && !name.includes('decoded'));

    if (capture) {
      const filePath = path.join(outDir, capture);
      const bytes = fs.readFileSync(filePath);
      if (typeof expectedSize !== 'number' || bytes.length === expectedSize) {
        return { capture, bytes };
      }
    }

    await new Promise((resolve) => setTimeout(resolve, intervalMs));
  }

  throw new Error(
    `Timed out after ${timeoutMs}ms waiting for capture file in ${outDir}` +
      (typeof expectedSize === 'number' ? ` with size ${expectedSize}` : '')
  );
}

function onceWithTimeout(emitter, event, timeoutMs, message) {
  return new Promise((resolve, reject) => {
    let settled = false;

    const cleanup = () => {
      if (typeof emitter.removeListener === 'function') {
        emitter.removeListener(event, handler);
      } else if (typeof emitter.off === 'function') {
        emitter.off(event, handler);
      }
    };

    const handler = (...args) => {
      if (settled) {
        return;
      }
      settled = true;
      clearTimeout(timeoutId);
      cleanup();
      resolve(args);
    };

    const timeoutId = setTimeout(() => {
      if (settled) {
        return;
      }
      settled = true;
      cleanup();

      if (emitter instanceof WebSocket) {
        try {
          if (typeof emitter.terminate === 'function') {
            emitter.terminate();
          } else if (typeof emitter.close === 'function') {
            emitter.close();
          }
        } catch (_) {
          // Ignore best-effort socket cleanup errors.
        }
      }

      reject(
        new Error(message || `Timeout of ${timeoutMs}ms exceeded while waiting for '${event}' event`)
      );
    }, timeoutMs);

    if (typeof emitter.on === 'function') {
      emitter.on(event, handler);
    } else if (typeof emitter.addListener === 'function') {
      emitter.addListener(event, handler);
    } else {
      clearTimeout(timeoutId);
      reject(new Error('Emitter does not support event subscription methods'));
    }
  });
}

test('health endpoint returns expected JSON payload', async (t) => {
  const server = await launchServer();
  t.after(async () => {
    try {
      await server.stop();
    } finally {
      fs.rmSync(server.outDir, { recursive: true, force: true });
    }
  });

  await waitForHealth(server.port);
  const res = await fetch(`http://127.0.0.1:${server.port}/health`);
  const body = await res.json();

  assert.equal(res.status, 200);
  assert.deepEqual(body, { ok: true, service: 'aows-test-endpoint' });
});

test('non-upgrade HTTP request gets 426 response', async (t) => {
  const server = await launchServer();
  t.after(async () => {
    try {
      await server.stop();
    } finally {
      fs.rmSync(server.outDir, { recursive: true, force: true });
    }
  });

  await waitForHealth(server.port);
  const res = await fetch(`http://127.0.0.1:${server.port}/`);
  const txt = await res.text();

  assert.equal(res.status, 426);
  assert.match(txt, /Upgrade Required: websocket/);
});

test('websocket binary frame is captured to output file', async (t) => {
  const server = await launchServer();
  t.after(async () => {
    try {
      await server.stop();
    } finally {
      fs.rmSync(server.outDir, { recursive: true, force: true });
    }
  });

  await waitForHealth(server.port);
  const ws = await openWs(`ws://127.0.0.1:${server.port}/`);

  const payload = Buffer.from([1, 2, 3, 4, 5, 6, 7, 8]);
  await new Promise((resolve, reject) => {
    ws.send(payload, (err) => {
      if (err) {
        reject(err);
        return;
      }
      resolve();
    });
  });
  ws.close(1000, 'done');
  await waitForWsClose(ws);

  let captureResult;
  try {
    captureResult = await waitForCaptureFile(server.outDir, payload.length);
  } catch (err) {
    assert.fail(`${err.message}, logs:\n${server.getLogs()}`);
  }

  const { capture, bytes } = captureResult;
  assert.ok(capture, `expected capture file, logs:\n${server.getLogs()}`);
  assert.deepEqual(bytes, payload);
});

test('diagnostic ping and stats replies are returned when enabled', async (t) => {
  const server = await launchServer({ AOWS_SEND_DIAGNOSTICS: 'true' });
  t.after(async () => {
    try {
      await server.stop();
    } finally {
      fs.rmSync(server.outDir, { recursive: true, force: true });
    }
  });

  await waitForHealth(server.port);
  const ws = new WebSocket(`ws://127.0.0.1:${server.port}/`);
  ws.on('error', (err) => {
    assert.fail(`Unexpected WebSocket error in diagnostics test: ${err.message}`);
  });
  const firstMessage = onceWithTimeout(
    ws,
    'message',
    5000,
    "Timed out waiting for initial 'session_open' message on diagnostics WebSocket"
  );
  await onceWithTimeout(
    ws,
    'open',
    5000,
    'Timed out waiting for diagnostics WebSocket to open'
  );
  const [sessionRaw] = await firstMessage;
  const sessionMsg = JSON.parse(sessionRaw.toString('utf8'));
  assert.equal(sessionMsg.event, 'session_open');

  ws.send(JSON.stringify({ event: 'ping' }));
  const [pongRaw] = await onceWithTimeout(
    ws,
    'message',
    5000,
    "Timed out waiting for 'pong' diagnostics message"
  );
  const pong = JSON.parse(pongRaw.toString('utf8'));
  assert.equal(pong.event, 'pong');

  ws.send(Buffer.from([9, 9, 9]));
  ws.send(JSON.stringify({ event: 'stats' }));
  const [statsRaw] = await onceWithTimeout(
    ws,
    'message',
    5000,
    "Timed out waiting for 'stats' diagnostics message"
  );
  const stats = JSON.parse(statsRaw.toString('utf8'));
  assert.equal(stats.event, 'stats');
  assert.equal(stats.bytes, 3);
  assert.equal(stats.frames, 1);

  ws.close(1000, 'done');
  await waitForWsClose(ws);
});

test('aligned ADPCM frame produces decoded PCM output when decode is enabled', async (t) => {
  const server = await launchServer({ AOWS_DECODE_ADPCM: 'true' });
  t.after(async () => {
    try {
      await server.stop();
    } finally {
      fs.rmSync(server.outDir, { recursive: true, force: true });
    }
  });

  await waitForHealth(server.port);
  const ws = await openWs(`ws://127.0.0.1:${server.port}/`);

  const frame = Buffer.alloc(95, 0);
  frame[1] = 10;
  frame[2] = 0x00;
  frame[3] = 0x00;
  await new Promise((resolve, reject) => {
    ws.send(frame, (err) => {
      if (err) {
        reject(err);
        return;
      }
      resolve();
    });
  });
  ws.close(1000, 'done');
  await waitForWsClose(ws);

  let decodedPath;
  try {
    decodedPath = await waitForDecodedPcmFile(server.outDir);
  } catch (err) {
    assert.fail(`${err.message}, logs:\n${server.getLogs()}`);
  }

  const decodedBytes = fs.readFileSync(decodedPath);
  assert.equal(decodedBytes.length, (95 - 4) * 4);
});
