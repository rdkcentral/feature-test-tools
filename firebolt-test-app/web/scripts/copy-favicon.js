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
const source = path.join(rootDir, 'static', 'icon.png');
const rootTarget = path.join(rootDir, 'favicon.png');
const buildTarget = path.join(rootDir, 'build', 'favicon.png');

function copyIfAvailable(target) {
  const targetDir = path.dirname(target);
  if (!fs.existsSync(targetDir)) {
    return false;
  }

  fs.copyFileSync(source, target);
  return true;
}

if (!fs.existsSync(source)) {
  console.error('Missing source icon:' + source);
  process.exit(1);
}

copyIfAvailable(rootTarget);
copyIfAvailable(buildTarget);
