#!/usr/bin/env bash
# Isolated Emscripten 3.1.69 toolchain from signed Debian 13 packages.
set -euo pipefail
repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_dir"
if [[ ! -f /workspace/toolchain/apt/config ]]; then ./scripts/bootstrap-cloud.sh; fi
export APT_CONFIG=/workspace/toolchain/apt/config
/usr/bin/apt-get update
/usr/bin/apt-get install --download-only --no-install-recommends -y emscripten
for pattern in emscripten binaryen clang-19 libclang-cpp19 libclang-common-19-dev lld-19 llvm-19 libpfm4 node-acorn; do
    for package in /workspace/toolchain/apt/archives/${pattern}_*.deb; do dpkg-deb -x "$package" /workspace/toolchain/sysroot; done
done
mkdir -p /workspace/toolchain/sysroot/usr/share/emscripten/node_modules
if [[ ! -e /workspace/toolchain/sysroot/usr/share/emscripten/node_modules/acorn ]]; then
    ln -s /workspace/toolchain/sysroot/usr/share/nodejs/acorn /workspace/toolchain/sysroot/usr/share/emscripten/node_modules/acorn
fi
python3 - <<'PY'
from pathlib import Path
import shutil
node=shutil.which('node')
if not node: raise SystemExit('Node.js is required')
config="LLVM_ROOT='/workspace/toolchain/sysroot/usr/bin'\nBINARYEN_ROOT='/workspace/toolchain/sysroot/usr'\nNODE_JS="+repr(node)+"\nFROZEN_CACHE=True\nLLVM_ADD_VERSION='19'\nCLANG_ADD_VERSION='19'\n"
Path('/workspace/toolchain/emscripten-config').write_text(config)
PY
./scripts/build-web.sh
