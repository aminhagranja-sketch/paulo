# Source this file from Bash in the isolated cloud environment.
export PATH="/workspace/toolchain/sysroot/usr/bin:$PATH"
export LD_LIBRARY_PATH="/workspace/toolchain/sysroot/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export XDG_CACHE_HOME=/workspace/paulo/build/cache
export XDG_DATA_HOME=/workspace/paulo/.local/share
mkdir -p "$XDG_CACHE_HOME" "$XDG_DATA_HOME"
