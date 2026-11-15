#!/usr/bin/env bash
# One-shot: DiskLordz OpenClaw + Telegram (Linux / VPS / Cloud Agent)
# Loads ~/.openclaw/disklordz.local.env if present; honors TELEGRAM_BOT_TOKEN + GEMINI_API_KEY in env.
set -euo pipefail

OC_SCRIPTS="$(cd "$(dirname "$0")" && pwd)"
OC_ROOT="$(cd "$OC_SCRIPTS/.." && pwd)"
REPO="${DLZ_REPO_ROOT:-$(cd "$OC_ROOT/.." && pwd)}"
STATE="${OPENCLAW_STATE_DIR:-$HOME/.openclaw}"
LOCAL_ENV="$STATE/disklordz.local.env"

need_node() {
  local major
  major="$(node -p "process.versions.node.split('.')[0]")"
  if [[ "$major" -lt 24 ]]; then
    echo "OpenClaw needs Node 24+. Current: $(node -v)"
    exit 1
  fi
}

openclaw_bin() {
  if command -v openclaw >/dev/null 2>&1; then
    command -v openclaw
    return
  fi
  if [[ -x "${OPENCLAW_BIN:-}" ]]; then
    echo "$OPENCLAW_BIN"
    return
  fi
  if [[ -x /tmp/openclaw-local/node_modules/.bin/openclaw ]]; then
    echo /tmp/openclaw-local/node_modules/.bin/openclaw
    return
  fi
  echo "openclaw CLI not found — install: npm install openclaw@2026.11.3 (Node 24+)" >&2
  exit 1
}

telegram_bot_profile() {
  [[ -n "${TELEGRAM_BOT_TOKEN:-}" ]] || return 0
  local base="https://api.telegram.org/bot${TELEGRAM_BOT_TOKEN}"
  curl -fsS -X POST "$base/setMyDescription" \
    -H "Content-Type: application/json" \
    -d '{"description":"DiskLordz command center — morning brief, dev status, drum kit ops. DM after OpenClaw pairing."}' \
    >/dev/null || true
  curl -fsS -X POST "$base/setMyShortDescription" \
    -H "Content-Type: application/json" \
    -d '{"short_description":"OpenClaw · sage. Prefixes: SAGE: DEV: MKT: DRUM:"}' \
    >/dev/null || true
  curl -fsS -X POST "$base/setMyCommands" \
    -H "Content-Type: application/json" \
    -d '{"commands":[{"command":"start","description":"Start / pair"},{"command":"brief","description":"Morning ops snapshot"},{"command":"help","description":"Agent prefixes"}]}' \
    >/dev/null || true
  echo "Telegram Bot API: description + commands updated"
}

export DLZ_REPO_ROOT="$REPO"
export OPENCLAW_STATE_DIR="$STATE"
mkdir -p "$STATE"

if [[ -f "$LOCAL_ENV" ]]; then
  # shellcheck disable=SC1090
  set -a && source "$LOCAL_ENV" && set +a
  echo "Loaded $LOCAL_ENV"
fi

need_node
OC="$(openclaw_bin)"

bash "$OC_SCRIPTS/install-pick-workspaces.sh" 2>/dev/null || bash "$OC_ROOT/disklordz-picks/scripts/install-pick-workspaces.sh"
bash "$REPO/disklordz-agent-stack/deploy/bootstrap-free.sh" || true
node "$OC_SCRIPTS/merge-disklordz-config.mjs"
node "$OC_SCRIPTS/merge-vst-factory-config.mjs"
(
  cd "$OC_SCRIPTS" && npm install --silent
)
node "$OC_SCRIPTS/apply-disklordz-config.mjs"
if [[ -f "$OC_SCRIPTS/apply-vst-factory-config.mjs" ]]; then
  node "$OC_SCRIPTS/apply-vst-factory-config.mjs"
fi

if [[ -n "${GEMINI_API_KEY:-}" ]]; then
  export GOOGLE_GENERATIVE_AI_API_KEY="$GEMINI_API_KEY"
  "$OC" config set models.providers.google.apiKey "$GEMINI_API_KEY" 2>/dev/null || true
fi

# Ensure gateway.mode for headless hosts
if ! grep -q '"mode"' "$STATE/openclaw.json" 2>/dev/null; then
  "$OC" config set gateway.mode local 2>/dev/null || true
fi

if [[ -n "${TELEGRAM_BOT_TOKEN:-}" ]]; then
  "$OC" channels add --channel telegram --token "$TELEGRAM_BOT_TOKEN"
  telegram_bot_profile
else
  echo "Skip Telegram channel (set TELEGRAM_BOT_TOKEN in $LOCAL_ENV or env)"
fi

"$OC" doctor --fix 2>/dev/null || true

AUTO_APPROVE="${OPENCLAW_AUTO_APPROVE_PAIRING:-}"
if [[ -n "${TELEGRAM_BOT_TOKEN:-}" && "$AUTO_APPROVE" == "1" ]]; then
  pending="$("$OC" pairing list telegram 2>/dev/null | awk '/^[A-Z0-9-]+/{print $1; exit}' || true)"
  if [[ -n "$pending" ]]; then
    "$OC" pairing approve telegram "$pending"
    echo "Auto-approved pairing: $pending"
  fi
fi

PAIR_FILE="$STATE/PAIRING-NEXT.txt"
cat >"$PAIR_FILE" <<EOF
DiskLordz OpenClaw — finish on phone
====================================
1) Telegram: open your bot, send any message (e.g. hi)
2) Server: $(basename "$OC") pairing list telegram
3) $(basename "$OC") pairing approve telegram <CODE>
   Or re-run with: OPENCLAW_AUTO_APPROVE_PAIRING=1 bash openclaw/scripts/finish-openclaw-telegram.sh
4) Test: SAGE: morning brief

Config: $STATE/openclaw.json
Gateway: $(basename "$OC") gateway run --bind loopback --port 18789  (or gateway start on VPS)
EOF

echo ""
echo "=== DiskLordz OpenClaw automation complete ==="
cat "$PAIR_FILE"

if [[ -z "${TELEGRAM_BOT_TOKEN:-}" ]]; then
  echo ""
  echo "Add secrets (never paste in chat):"
  echo "  Cursor → Secrets → TELEGRAM_BOT_TOKEN, GEMINI_API_KEY"
  echo "  Or: cp openclaw/scripts/disklordz.local.env.example → $LOCAL_ENV"
fi

"$OC" channels status --probe 2>/dev/null || true