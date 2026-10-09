#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 tools/sprite_pipeline.py pack --manifest assets/animations/review.json --size 1024
mkdir -p web/assets/animations
cp -R assets/atlases web/assets/
for category in chests buildings trees environment enemies/snake enemies/fox characters/chicks characters/chicken; do
    mkdir -p "web/assets/animations/$(dirname "$category")"
    cp "assets/animations/$category.json" "web/assets/animations/$category.json"
done
