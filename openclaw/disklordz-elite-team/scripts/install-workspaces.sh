#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
STATE="${OPENCLAW_STATE_DIR:-$HOME/.openclaw}"

for id in sage code arch review patch forge trip plan flow sentinel relay; do
  dest="$STATE/workspace-$id"
  mkdir -p "$dest"
  if [[ -f "$ROOT/workspaces/$id/AGENTS.md" ]]; then
    cp "$ROOT/workspaces/$id/AGENTS.md" "$dest/AGENTS.md"
    echo "Installed $dest/AGENTS.md"
  fi
done

echo ""
echo "Register agents (run once):"
for id in sage code arch review patch forge trip plan flow sentinel; do
  echo "  openclaw agents add $id --workspace $STATE/workspace-$id --non-interactive"
done