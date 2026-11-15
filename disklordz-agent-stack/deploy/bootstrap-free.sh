#!/usr/bin/env bash
# DiskLordz free agent stack bootstrap — run on VPS or home server (Ubuntu/Debian-ish)
set -euo pipefail

REPO="${DLZ_REPO_ROOT:-$HOME/push}"
STATE="${OPENCLAW_STATE_DIR:-$HOME/.openclaw}"
OUT="$STATE/hermes-out"

if ! command -v node >/dev/null; then
  echo "Install Node 22+ first (OpenClaw installer can do this)"
  exit 1
fi

mkdir -p "$OUT/research" "$OUT/pr-review" "$OUT/relay" "$OUT/events"
mkdir -p "$STATE/workspace-shared/vault-docs"

if [[ ! -d "$REPO/.git" ]]; then
  echo "Clone repo to $REPO and set DLZ_REPO_ROOT"
  exit 1
fi

cd "$REPO/disklordz-agent-stack"
npm install
export DLZ_REPO_ROOT="$REPO"
export OPENCLAW_STATE_DIR="$STATE"
npm run all

echo ""
echo "OK: hermes-out at $OUT"
echo "Next: openclaw onboard (Gemini free key), merge openclaw/disklordz-picks/openclaw.stack.json5.example"
echo "Cron: see deploy/crontab.free.example"