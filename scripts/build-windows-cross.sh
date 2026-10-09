#!/usr/bin/env bash
# Optional MinGW build for the Debian cloud machine; Windows execution is separate.
set -euo pipefail
repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_dir"
if [[ ! -f /workspace/toolchain/apt/config ]]; then ./scripts/bootstrap-cloud.sh; fi
source scripts/cloud-env.sh
export APT_CONFIG=/workspace/toolchain/apt/config
/usr/bin/apt-get update
/usr/bin/apt-get install --download-only -y g++-mingw-w64-x86-64-posix
for package in /workspace/toolchain/apt/archives/*mingw*.deb; do dpkg-deb -x "$package" /workspace/toolchain/sysroot; done
extra=()
if [[ -d build/cloud/_deps/sfml-src && -d build/cloud/_deps/json-src ]]; then
    extra+=("-DFETCHCONTENT_SOURCE_DIR_SFML=$repo_dir/build/cloud/_deps/sfml-src" "-DFETCHCONTENT_SOURCE_DIR_JSON=$repo_dir/build/cloud/_deps/json-src")
fi
cmake -S . -B build/windows-cross -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-local.cmake -DCMAKE_BUILD_TYPE=Release "${extra[@]}"
cmake --build build/windows-cross --parallel 4
cmake --install build/windows-cross --prefix build/package-windows --component Runtime
python3 - <<'PY'
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
root=Path('build/package-windows')
with ZipFile('build/MeuGalinheiro-Windows.zip','w',ZIP_DEFLATED) as archive:
    for file in sorted(root.rglob('*')):
        if file.is_file(): archive.write(file,file.relative_to(root))
print('Windows package: build/MeuGalinheiro-Windows.zip (cross compiled; not executed here)')
PY
