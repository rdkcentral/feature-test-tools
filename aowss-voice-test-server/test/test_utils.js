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

const path = require('path');
const fs = require('fs');
const os = require('os');
const net = require('net');
const { spawn } = require('child_process');
const { once } = require('events');
const { WebSocket } = require('ws');

const SERVER_JS = path.resolve(__dirname, '..', 'aows_test_server.js');

async function waitForHealth(port, timeoutMs = 8000) {
  const deadline = Date.now() + timeoutMs;
  let lastErr = null;

  while (Date.now() < deadline) {
    const remainingMs = Math.max(1, deadline - Date.now());
    const attemptTimeoutMs = Math.min(1000, remainingMs);
    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort(), attemptTimeoutMs);

    try {
      const res = await fetch(`http://127.0.0.1:${port}/health`, { signal: controller.signal });
      if (res.ok) {
        try {
          await res.body?.cancel?.();
        } catch (_) {
          // Ignore cancellation errors for successful health responses.
        }
        return;
      }
      try {
        await res.body?.cancel?.();
      } catch (_) {
        // Ignore cancellation errors for failed health responses.
      }
      lastErr = new Error(`health status=${res.status}`);
    } catch (err) {
      lastErr = err;
    } finally {
      clearTimeout(timeoutId);
    }

    await new Promise((resolve) => setTimeout(resolve, 120));
  }

  throw lastErr || new Error('server did not become healthy');
}

async function findAvailablePortInRange(minPort, maxPort, maxAttempts = 50) {
  if (minPort >= maxPort) {
    throw new Error(`invalid port range: ${minPort}..${maxPort}`);
  }

  const rangeSize = maxPort - minPort;
  const attempts = Math.min(rangeSize, maxAttempts);
  const startOffset = Math.floor(Math.random() * rangeSize);

  for (let attempt = 0; attempt < attempts; attempt += 1) {
    const offset = (startOffset + attempt) % rangeSize;
    const port = minPort + offset;
    const server = net.createServer();

    try {
      await new Promise((resolve, reject) => {
        server.once('error', (err) => {
          if (server.listening) {
            server.close(() => reject(err));
          } else {
            reject(err);
          }
        });
        server.listen(port, '127.0.0.1', () => resolve());
      });

      await new Promise((resolve) => server.close(resolve));
      return port;
    } catch (err) {
      if (err && err.code === 'EADDRINUSE') {
        continue;
      }
      throw err;
    }
  }

  throw new Error(
    `could not find a free port in range ${minPort}..${maxPort - 1} after ${attempts} attempts`
  );
}

async function launchServer(extraEnv = {}) {
  const maxLaunchAttempts = 3;

  for (let launchAttempt = 0; launchAttempt < maxLaunchAttempts; launchAttempt += 1) {
    const port = await findAvailablePortInRange(20000, 40000);
    const outDir = fs.mkdtempSync(path.join(os.tmpdir(), 'aows-test-'));

    const env = {
      ...process.env,
      AOWS_HOST: '127.0.0.1',
      AOWS_PORT: String(port),
      AOWS_OUT_DIR: outDir,
      AOWS_PATH: '/',
      AOWS_IDLE_CLOSE_MS: '0',
      ...extraEnv
    };

    const child = spawn(process.execPath, [SERVER_JS], {
      env,
      stdio: ['ignore', 'pipe', 'pipe']
    });

    let logs = '';
    let childError = null;
    if (child.stdout) {
      child.stdout.on('data', (buf) => {
        logs += buf.toString('utf8');
      });
    }
    if (child.stderr) {
      child.stderr.on('data', (buf) => {
        logs += buf.toString('utf8');
      });
    }
    child.on('error', (err) => {
      childError = err;
      const msg = err && (err.stack || err.message) ? err.stack || err.message : String(err);
      logs += `\n[child_process error] ${msg}\n`;
    });

    try {
      await new Promise((resolve, reject) => {
        const cleanup = () => {
          child.removeListener('spawn', handleSpawn);
          child.removeListener('error', handleError);
        };

        const handleSpawn = () => {
          cleanup();
          resolve();
        };

        const handleError = (err) => {
          cleanup();
          try {
            if (outDir && fs.existsSync(outDir)) {
              fs.rmSync(outDir, { recursive: true, force: true });
            }
          } catch (_) {
            // Best-effort cleanup; preserve the original spawn error.
          }
          const errorObj = err instanceof Error ? err : new Error(String(err));
          reject(new Error(`failed to spawn test server: ${errorObj.message}`));
        };

        child.once('spawn', handleSpawn);
        child.once('error', handleError);
      });
    } catch (err) {
      const errText = String(err && err.message ? err.message : err);
      if (launchAttempt + 1 < maxLaunchAttempts && /EADDRINUSE/i.test(errText)) {
        continue;
      }
      throw err;
    }

    const earlyExit = await new Promise((resolve) => {
      let timeoutId;

      const cleanup = () => {
        child.removeListener('exit', handleExit);
        if (timeoutId != null) {
          clearTimeout(timeoutId);
          timeoutId = null;
        }
      };

      const handleExit = (code, signal) => {
        cleanup();
        resolve({ code, signal });
      };

      child.once('exit', handleExit);
      timeoutId = setTimeout(() => {
        cleanup();
        resolve(null);
      }, 250);
    });

    if (earlyExit) {
      const startupErr = new Error(
        `server process exited during startup (code=${earlyExit.code}, signal=${earlyExit.signal || 'none'})\nlogs:\n${logs}`
      );

      try {
        if (outDir && fs.existsSync(outDir)) {
          fs.rmSync(outDir, { recursive: true, force: true });
        }
      } catch (_) {
        // Best-effort cleanup for startup failures.
      }

      if (launchAttempt + 1 < maxLaunchAttempts && /EADDRINUSE/i.test(logs)) {
        continue;
      }

      throw startupErr;
    }

    const waitForExit = (timeoutMs) => {
      if (child.exitCode != null) return Promise.resolve(true);
      return Promise.race([
        once(child, 'exit').then(() => true),
        new Promise((resolve) => setTimeout(() => resolve(false), timeoutMs))
      ]);
    };

    const stop = async () => {
      if (childError) {
        throw new Error(`server process failed before stop\nlogs:\n${logs}`);
      }

      if (child.exitCode != null) return;
      try {
        child.kill('SIGTERM');
      } catch (err) {
        if (err.code !== 'ESRCH') throw err;
      }

      const exitedAfterTerm = await waitForExit(1500);
      if (exitedAfterTerm || child.exitCode != null) return;

      try {
        child.kill('SIGKILL');
      } catch (err) {
        if (err.code !== 'ESRCH') throw err;
      }
      const exitedAfterKill = await waitForExit(1500);
      if (!exitedAfterKill && child.exitCode == null) {
        throw new Error(`server process did not exit after SIGTERM/SIGKILL\nlogs:\n${logs}`);
      }
    };

    return { child, port, outDir, stop, getLogs: () => logs };
  }

  throw new Error('failed to launch test server after retries');
}

async function openWs(url) {
  const ws = new WebSocket(url);
  const timeoutMs = 8000;
  await new Promise((resolve, reject) => {
    let timeoutId;

    const cleanup = () => {
      ws.removeListener('open', handleOpen);
      ws.removeListener('error', handleError);
      ws.removeListener('close', handleClose);
      if (timeoutId != null) {
        clearTimeout(timeoutId);
        timeoutId = null;
      }
    };

    const handleOpen = () => {
      cleanup();
      resolve();
    };

    const handleError = (err) => {
      cleanup();
      reject(err instanceof Error ? err : new Error(String(err)));
    };

    const handleClose = () => {
      cleanup();
      reject(new Error('WebSocket closed before connection was opened'));
    };

    ws.on('open', handleOpen);
    ws.on('error', handleError);
    ws.on('close', handleClose);

    timeoutId = setTimeout(() => {
      cleanup();
      try {
        if (typeof ws.terminate === 'function') {
          ws.terminate();
        } else {
          ws.close();
        }
      } catch (_) {}
      reject(new Error('Timed out waiting for WebSocket to open'));
    }, timeoutMs);
  });

  ws.on('error', () => {});

  return ws;
}

function waitForWsClose(ws) {
  return once(ws, 'close');
}

module.exports = {
  waitForHealth,
  launchServer,
  openWs,
  waitForWsClose
};
