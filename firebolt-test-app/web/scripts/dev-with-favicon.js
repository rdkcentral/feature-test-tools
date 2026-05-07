/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
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
 **/

const { spawn } = require('child_process');
const fs = require('fs');
const path = require('path');

const rootDir = path.resolve(__dirname, '..');
const copyScript = path.join(__dirname, 'copy-favicon.js');
const npxCmd = process.platform === 'win32' ? 'npx.cmd' : 'npx';

function syncFavicon() {
  try {
    const child = spawn(process.execPath, [copyScript], {
      cwd: rootDir,
      stdio: 'ignore'
    });
    child.on('error', () => {
      // Keep dev server alive even if favicon sync fails.
    });
  } catch (err) {
    // Keep dev server alive even if favicon sync fails.
  }
}

syncFavicon();

const devChild = spawn(npxCmd, ['lng', 'dev'], {
  cwd: rootDir,
  stdio: 'inherit'
});

devChild.on('error', (err) => {
  console.error('[dev-with-favicon] Failed to start dev server:', err.message);
  clearInterval(interval);
  process.exit(1);
});

const interval = setInterval(() => {
  if (fs.existsSync(path.join(rootDir, 'build'))) {
    syncFavicon();
  }
}, 1000);

const shutdown = (signal) => {
  clearInterval(interval);
  if (!devChild.killed) {
    devChild.kill(signal);
  }
};

process.on('SIGINT', () => shutdown('SIGINT'));
process.on('SIGTERM', () => shutdown('SIGTERM'));


devChild.on('exit', (code) => {
  clearInterval(interval);
  process.exit(code ?? 0);
});
