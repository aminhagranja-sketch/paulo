#!/usr/bin/env bash
set -euo pipefail
repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_dir"
if [[ -x /workspace/toolchain/sysroot/usr/share/emscripten/em++ ]]; then
    export EM_CONFIG=/workspace/toolchain/emscripten-config
    export EM_CACHE=/workspace/toolchain/sysroot/usr/share/emscripten/cache
    export LD_LIBRARY_PATH="/workspace/toolchain/sysroot/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
    emscripten_compiler=/workspace/toolchain/sysroot/usr/share/emscripten/em++
else
    emscripten_compiler=em++
fi
json_headers="$repo_dir/build/cloud/_deps/json-src/include"
if [[ ! -f "$json_headers/nlohmann/json.hpp" ]]; then
    echo 'Configure the C++ core first to fetch nlohmann/json, or set GRANJA_JSON_INCLUDE.' >&2
    json_headers="${GRANJA_JSON_INCLUDE:?Set GRANJA_JSON_INCLUDE to nlohmann/json include directory}"
fi
"$emscripten_compiler" src/World.cpp src/Simulation.cpp src/Save.cpp src/web/Bridge.cpp -Iinclude -I"$json_headers" -std=c++20 -O2 -fexceptions \
    -sALLOW_MEMORY_GROWTH=1 -sMODULARIZE=1 -sEXPORT_NAME=createGranja -sENVIRONMENT=web,node \
    '-sEXPORTED_RUNTIME_METHODS=["ccall","UTF8ToString"]' -o web/granja.js
mkdir -p web/assets
cp assets/sprites/chicken-*.png web/assets/
