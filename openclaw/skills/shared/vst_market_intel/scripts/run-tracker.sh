#!/usr/bin/env bash
set -euo pipefail
SKILL="$(cd "$(dirname "$0")/.." && pwd)"
ROOT="${DLZ_REPO_ROOT:-$(cd "$SKILL/../../../.." 2>/dev/null && pwd)}"
export DLZ_REPO_ROOT="$ROOT"
python3 "$SKILL/track_competitors.py"