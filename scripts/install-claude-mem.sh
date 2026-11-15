#!/usr/bin/env bash
# Linux/macOS/WSL — same as install-claude-mem.ps1
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
SETUP="${1:-defer}"
PROVIDER="${2:-}"

echo "Claude-Mem install from: $ROOT"
if ! command -v bun >/dev/null 2>&1; then
  echo "Install Bun: curl -fsSL https://bun.sh/install | bash" >&2
fi

ARGS=(claude-mem install --ide cursor --setup "$SETUP")
if [[ -n "$PROVIDER" ]]; then
  ARGS+=(--provider "$PROVIDER")
fi

npx "${ARGS[@]}"
echo "Restart Cursor. Seed: docs/dev/claude-mem-seed.md"
echo "Health: scripts/check-claude-mem-health.sh"