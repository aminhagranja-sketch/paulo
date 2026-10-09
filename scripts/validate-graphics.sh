#!/usr/bin/env bash
# Separate temporary display and save; never touches a player's real progress.
set -euo pipefail
repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_dir"
source scripts/cloud-env.sh
mkdir -p build/validation
validation_dir="$(mktemp -d "$repo_dir/build/validation/run.XXXXXX")"
Xvfb -displayfd 3 -screen 0 1920x1080x24 -nolisten tcp 3>"$validation_dir/display" >"$validation_dir/xvfb.log" 2>&1 &
xvfb_pid=$!
cleanup() {
    kill "$xvfb_pid" 2>/dev/null || true
    wait "$xvfb_pid" 2>/dev/null || true
}
trap cleanup EXIT
for attempt in {1..50}; do
    if [[ -s "$validation_dir/display" ]]; then break; fi
    if ! kill -0 "$xvfb_pid" 2>/dev/null; then cat "$validation_dir/xvfb.log"; exit 1; fi
    sleep 0.1
done
if [[ ! -s "$validation_dir/display" ]]; then cat "$validation_dir/xvfb.log"; exit 1; fi
export DISPLAY=":$(cat "$validation_dir/display")"
export LIBGL_ALWAYS_SOFTWARE=1
build/cloud/meu_galinheiro --smoke --save "$validation_dir/save.json"
echo "Screenshot e save de teste: $validation_dir"
