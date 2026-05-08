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

const fs = require('fs');
const path = require('path');

const rootDir = path.resolve(__dirname, '..');
const buildDir = path.join(rootDir, 'build');
const target = path.join(buildDir, 'favicon.ico');

const candidates = [
  path.join(rootDir, 'static', 'favicon.ico'),
  path.join(rootDir, 'favicon.ico')
];

const source = candidates.find((p) => fs.existsSync(p));

if (!source) {
  console.warn('[copy-favicon-ico] No favicon.ico source found (checked static/ and root).');
  process.exit(0);
}

if (!fs.existsSync(buildDir)) {
  console.warn('[copy-favicon-ico] build/ directory not found; skipping favicon.ico copy.');
  process.exit(0);
}

fs.copyFileSync(source, target);
console.log(`[copy-favicon-ico] Copied ${path.relative(rootDir, source)} -> build/favicon.ico`);
