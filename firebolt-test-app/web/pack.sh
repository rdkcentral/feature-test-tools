#!/bin/bash

# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2025 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -euo pipefail

START_DIR="$(cd "$(dirname "$0")" && pwd -P)"
BUNDLER_WORKDIR="${BUNDLER_WORKDIR:-$START_DIR}"
BUILD_DIR="${START_DIR}/build"
TAR_OUTPUT="${BUNDLER_WORKDIR}/fbttest.tgz"
INSTALL_PATH="usr/share/fbttest"

# ---------- deps ----------
[ ! -d "${START_DIR}/node_modules" ] && { echo "node_modules not found — running npm install..."; npm install; }

# ---------- build ----------
echo "Building..."
rm -rf "${BUILD_DIR}"
npm run build
[ ! -d "${BUILD_DIR}" ] && { echo "ERROR: build directory not created!"; exit 1; }

# ---------- patch startApp.js: XHR status=0 for file:// ----------
sed -i 's/xhr\.status === 200/xhr.status === 200 || (xhr.status === 0 \&\& xhr.responseText)/g' \
    "${BUILD_DIR}/startApp.js" \
    && echo "Patched startApp.js (file:// XHR)"

# ---------- patch appBundle.js: guard Transport.receive JSON.parse ----------
python - "${BUILD_DIR}/appBundle.js" <<'PYEOF'
import sys
path = sys.argv[1]
with open(path) as f:
    c = f.read()
old = 'Transport.receive(async (message) => {\n    const json = JSON.parse(message);'
new = 'Transport.receive(async (message) => {\n    let json; try { json = JSON.parse(message); } catch(e) { return; }'
patched = c.replace(old, new)
if patched == c:
    print("WARNING: Transport.receive patch target not found — skipping", flush=True)
else:
    with open(path, 'w') as f:
        f.write(patched)
    print("Patched appBundle.js (Transport.receive JSON.parse guard)", flush=True)
PYEOF

# ---------- package ----------
TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

mkdir -p "${TMPDIR}/${INSTALL_PATH}"
cp -r "${BUILD_DIR}/." "${TMPDIR}/${INSTALL_PATH}/"

mkdir -p "${BUNDLER_WORKDIR}"
tar -C "${TMPDIR}" -czf "${TAR_OUTPUT}" usr
echo "Package created: ${TAR_OUTPUT}"
