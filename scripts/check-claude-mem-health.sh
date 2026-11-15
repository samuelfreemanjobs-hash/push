#!/usr/bin/env bash
set -euo pipefail
PORT_FILE="${HOME}/.claude-mem/.worker.port"
if [[ ! -f "$PORT_FILE" ]]; then
  echo "Run scripts/install-claude-mem.sh first" >&2
  exit 1
fi
PORT="$(tr -d '[:space:]' < "$PORT_FILE")"
curl -sf "http://127.0.0.1:${PORT}/api/health" | python3 -m json.tool