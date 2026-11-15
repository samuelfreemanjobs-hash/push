#!/usr/bin/env bash
# DiskLordz OpenClaw + Telegram (VPS / Linux / WSL)
set -euo pipefail

OC_SCRIPTS="$(cd "$(dirname "$0")" && pwd)"
OC_ROOT="$(cd "$OC_SCRIPTS/.." && pwd)"
REPO="${DLZ_REPO_ROOT:-$(cd "$OC_ROOT/.." && pwd)}"
STATE="${OPENCLAW_STATE_DIR:-$HOME/.openclaw}"

need_node() {
  local major
  major="$(node -p "process.versions.node.split('.')[0]")"
  if [[ "$major" -lt 24 ]]; then
    echo "OpenClaw needs Node 24+ or 26+. Current: $(node -v)"
    echo "Install: https://nodejs.org/ or use nvm install 26"
    exit 1
  fi
}

if ! command -v openclaw >/dev/null 2>&1; then
  echo "Installing OpenClaw CLI..."
  curl -fsSL https://openclaw.ai/install.sh | bash
fi

need_node

export DLZ_REPO_ROOT="$REPO"
export OPENCLAW_STATE_DIR="$STATE"

mkdir -p "$STATE/hermes-out/research" "$STATE/hermes-out/pr-review" "$STATE/workspace-shared/vault-docs"

bash "$OC_ROOT/disklordz-picks/scripts/install-pick-workspaces.sh"
bash "$REPO/disklordz-agent-stack/deploy/bootstrap-free.sh" || true
node "$OC_SCRIPTS/merge-disklordz-config.mjs"

if [[ ! -f "$STATE/openclaw.json" ]]; then
  echo ""
  echo "=== First-time OpenClaw onboarding ==="
  echo "Run: openclaw onboard --classic"
  echo "Provider: Google AI Studio → Gemini 2.0 Flash (free tier)"
fi

if [[ -n "${TELEGRAM_BOT_TOKEN:-}" ]]; then
  openclaw channels add --channel telegram --token "$TELEGRAM_BOT_TOKEN"
else
  echo ""
  echo "Telegram: create bot via @BotFather, then:"
  echo "  export TELEGRAM_BOT_TOKEN='123:abc...'"
  echo "  openclaw channels add --channel telegram --token \"\$TELEGRAM_BOT_TOKEN\""
fi

echo ""
echo "=== Gateway ==="
echo "  openclaw gateway install"
echo "  openclaw gateway start"
echo "  openclaw channels status --probe"
echo ""
echo "=== Pairing (required for DMs) ==="
echo "  1) Message your bot in Telegram"
echo "  2) openclaw pairing list telegram"
echo "  3) openclaw pairing approve telegram <CODE>"
echo ""
echo "=== Test messages (sage reads prefixes) ==="
echo "  SAGE: morning brief"
echo "  DEV: status"
echo "  DRUM: validate kit-spec"
echo ""
echo "Merged config draft: $STATE/disklordz.merged.json5"
echo "Docs: $OC_ROOT/SETUP-TELEGRAM.md"