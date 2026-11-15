#!/usr/bin/env bash
# Wire DLZ_REPO_ROOT for OpenClaw on Linux/macOS (cloud dev / WSL helpers).
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 /absolute/path/to/push" >&2
  exit 1
fi

REPO_ROOT="$(cd "$1" && pwd)"
STATE="${OPENCLAW_STATE_DIR:-$HOME/.openclaw}"
mkdir -p "$STATE" "$REPO_ROOT/Assets/Releases"

LOCAL="$STATE/disklordz.local.env"
if ! grep -q DLZ_REPO_ROOT "$LOCAL" 2>/dev/null; then
  echo "export DLZ_REPO_ROOT=$REPO_ROOT" >> "$LOCAL"
fi

cat > "$REPO_ROOT/openclaw/autogpt/.env" <<EOF
DLZ_WIN_ROOT=$REPO_ROOT
FIRECRAWL_API_KEY=
AUTOGPT_MAX_ITERATIONS=8
EOF

echo "DLZ_REPO_ROOT=$REPO_ROOT"
echo "Source: $LOCAL"
echo "AutoGPT: $REPO_ROOT/openclaw/autogpt/.env"
echo "See disklordz-ralph/STACK-4-LAYER.md"