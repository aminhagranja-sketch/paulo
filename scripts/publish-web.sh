#!/usr/bin/env bash
# Invoke only when publication to this GitHub repository is authorized.
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."
if [[ -n "$(git status --porcelain -- web)" ]]; then
    echo 'Commit and review web/ changes before publishing.' >&2
    exit 1
fi
pages_tree="$(git rev-parse HEAD:web)"
parents=()
if [[ -n "$(git ls-remote origin refs/heads/gh-pages)" ]]; then
    git fetch origin refs/heads/gh-pages
    parents=(-p "$(git rev-parse FETCH_HEAD)")
fi
pages_commit="$(git commit-tree "$pages_tree" "${parents[@]}" -m 'Publish tested mobile game from web/')"
git push origin "$pages_commit":refs/heads/gh-pages
echo "Published static bundle at gh-pages commit $pages_commit"
echo 'If Pages is disabled: GitHub Settings > Pages > Deploy from a branch > gh-pages / root > Save.'
