#!/usr/bin/env bash
# Non-root Debian 13 cloud setup. All files stay under /workspace/toolchain.
set -euo pipefail
repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
kit=/workspace/toolchain
mkdir -p "$kit/apt"/{lists/partial,archives/partial,etc/apt.conf.d,etc/sources.list.d,etc/preferences.d} "$kit/sysroot"
cat > "$kit/apt/etc/sources.list" <<'SOURCES'
deb [signed-by=/usr/share/keyrings/debian-archive-keyring.gpg] https://deb.debian.org/debian trixie main
SOURCES
cat > "$kit/apt/config" <<'CONFIG'
Dir::Etc "/workspace/toolchain/apt/etc";
Dir::Etc::sourcelist "sources.list";
Dir::Etc::sourceparts "sources.list.d";
Dir::Etc::main "apt.conf";
Dir::Etc::parts "apt.conf.d";
Dir::State::lists "/workspace/toolchain/apt/lists";
Dir::Cache::archives "/workspace/toolchain/apt/archives";
Debug::NoLocking "true";
APT::Sandbox::User "agent";
CONFIG
export APT_CONFIG="$kit/apt/config"
/usr/bin/apt-get update
/usr/bin/apt-get install --download-only -y cmake ninja-build libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libfreetype-dev libgl1-mesa-dev xvfb xauth
for package in "$kit"/apt/archives/*.deb; do dpkg-deb -x "$package" "$kit/sysroot"; done
# Development-library symlinks refer to runtime packages supplied by the base image.
python3 - <<'PY'
from pathlib import Path
root=Path('/workspace/toolchain/sysroot/usr/lib/x86_64-linux-gnu')
for file in root.iterdir():
    if file.is_symlink() and not file.exists():
        target=Path('/usr/lib/x86_64-linux-gnu')/file.readlink().name
        if target.exists():
            file.unlink()
            file.symlink_to(target)
PY
export PATH="$kit/sysroot/usr/bin:$PATH"
export LD_LIBRARY_PATH="$kit/sysroot/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
cd "$repo_dir"
cmake -S . -B build/cloud -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/cloud-local.cmake
cmake --build build/cloud --parallel 4
ctest --test-dir build/cloud --output-on-failure
