#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
STATE="${OPENCLAW_STATE_DIR:-$HOME/.openclaw}"
SHARED="$STATE/workspace-shared/vault-docs"
mkdir -p "$SHARED" "$STATE/hermes-out/research" "$STATE/hermes-out/pr-review" "$STATE/hermes-out"

for id in sage dev mkt biz; do
  dest="$STATE/workspace-$id"
  mkdir -p "$dest"
  [[ -f "$ROOT/workspaces/$id/AGENTS.md" ]] && cp "$ROOT/workspaces/$id/AGENTS.md" "$dest/AGENTS.md"
  [[ -f "$ROOT/workspaces/$id/OVERNIGHT.md" ]] && cp "$ROOT/workspaces/$id/OVERNIGHT.md" "$dest/OVERNIGHT.md"
done

echo "Shared memory dir: $SHARED (clone vault docs here)"
echo "Hermes handoff dir: $STATE/hermes-out/"