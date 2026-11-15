#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
cd "$ROOT"
export PYTHONPATH="${ROOT}:${PYTHONPATH:-}"
python3 openclaw/elite-frameworks/waza/evals/test_agent_artifacts.py
echo "Waza: all artifact evals passed."