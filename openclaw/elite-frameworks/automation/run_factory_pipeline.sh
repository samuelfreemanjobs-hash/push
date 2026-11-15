#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
exec python3 "$ROOT/openclaw/elite-frameworks/automation/run_factory_pipeline.py"