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
const { once } = require('events');
const { WebSocket } = require('ws');
const {
  waitForHealth,
  launchServer,
  openWs,
  waitForWsClose
} = require('./test_utils');

async function waitForFileByNameAndSize(
  outDir,
  matchName,
  expectedSize,
  timeoutMs = 5000,
  intervalMs = 50
) {
  const deadline = Date.now() + timeoutMs;

  while (Date.now() < deadline) {
    const files = fs.readdirSync(outDir);
    const matched = files.find((name) => matchName(name));

    if (matched) {
      const stat = fs.statSync(path.join(outDir, matched));
      if (typeof expectedSize !== 'number' || stat.size === expectedSize) {
        return { name: matched, size: stat.size };
      }
    }

    await new Promise((resolve) => setTimeout(resolve, intervalMs));
  }

  throw new Error(`Timed out waiting for matching file in ${outDir}`);
}

test('websocket upgrade is rejected for non-matching path filter', async (t) => {
  const server = await launchServer({ AOWS_PATH: '/voice' });
  t.after(async () => {
    try {
      await server.stop();
    } finally {
      fs.rmSync(server.outDir, { recursive: true, force: true });
    }
  });

  await waitForHealth(server.port);

  await new Promise((resolve, reject) => {
    const ws = new WebSocket(`ws://127.0.0.1:${server.port}/`);
    const timeout = setTimeout(() => {
      ws.terminate();
      reject(new Error('timed out waiting for websocket upgrade rejection'));
    }, 1000);

    const cleanup = () => {
      clearTimeout(timeout);
      ws.removeAllListeners();
    };

    const succeed = () => {
      cleanup();
      resolve();
    };

    ws.once('error', () => {
      succeed();
    });
    ws.once('unexpected-response', () => {
      succeed();
    });
    ws.once('close', () => {
      // Treat close without a successful open as a successful rejection.
      if (ws.readyState !== WebSocket.OPEN) {
        succeed();
      }
    });
    ws.once('open', () => {
      cleanup();
      ws.close(1000, 'unexpected');
      reject(new Error('expected websocket open to be rejected for non-matching path'));
    });
  });
});

test('unsupported JSON event returns error payload in diagnostics mode', async (t) => {
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
  const initialTimeoutMs = 5000;
  try {
    await Promise.race([
      once(ws, 'open'),
      new Promise((_, reject) =>
        setTimeout(() => reject(new Error('Timed out waiting for websocket open')), initialTimeoutMs)
      )
    ]);
  } catch (err) {
    assert.fail(`${err.message}, logs:\n${server.getLogs()}`);
  }

  try {
    await Promise.race([
      once(ws, 'message'),
      new Promise((_, reject) =>
        setTimeout(() => reject(new Error('Timed out waiting for initial message')), initialTimeoutMs)
      )
    ]);
  } catch (err) {
    assert.fail(`${err.message}, logs:\n${server.getLogs()}`);
  }

  ws.send(JSON.stringify({ event: 'unsupported_thing' }));

  let msg;
  // Loop until we receive the expected error event.
  // The protocol may emit other unsolicited messages first.
  const messageTimeoutMs = 5000;
  const deadline = Date.now() + messageTimeoutMs;
  // eslint-disable-next-line no-constant-condition
  while (true) {
    const remainingMs = deadline - Date.now();
    if (remainingMs <= 0) {
      assert.fail(`Timed out waiting for error event message, logs:\n${server.getLogs()}`);
    }

    let raw;
    try {
      [raw] = await Promise.race([
        once(ws, 'message'),
        new Promise((_, reject) =>
          setTimeout(() => reject(new Error('Timed out waiting for error event message')), remainingMs)
        )
      ]);
    } catch (err) {
      assert.fail(`${err.message}, logs:\n${server.getLogs()}`);
    }

    const parsed = JSON.parse(raw.toString('utf8'));
    if (parsed && parsed.event === 'error') {
      msg = parsed;
      break;
    }
  }

  assert.equal(msg.event, 'error');
  assert.equal(msg.message, 'unsupported_event:unsupported_thing');

  ws.close(1000, 'done');
  await waitForWsClose(ws);
});

test('misaligned ADPCM chunk writes empty decoded file', async (t) => {
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

  await new Promise((resolve, reject) => {
    ws.send(Buffer.alloc(10, 0xaa), (err) => {
      if (err) {
        reject(err);
        return;
      }
      resolve();
    });
  });
  ws.close(1000, 'done');
  await waitForWsClose(ws);

  let decodedResult;
  try {
    decodedResult = await waitForFileByNameAndSize(
      server.outDir,
      (name) => name.includes('adpcm-decoded-s16le.pcm'),
      0
    );
  } catch (err) {
    assert.fail(`${err.message}, logs:\n${server.getLogs()}`);
  }

  assert.ok(decodedResult.name, `expected decoded file, logs:\n${server.getLogs()}`);
  assert.equal(decodedResult.size, 0);
});
