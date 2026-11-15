#!/usr/bin/env bash
set -euo pipefail
ROOT="${DLZ_REPO_ROOT:-$(cd "$(dirname "$0")/../../../.." && pwd)}"
export DLZ_REPO_ROOT="$ROOT"
exec python3 "$ROOT/openclaw/skills/shared/vst_market_intel/find_producer_leads.py"