#!/usr/bin/env bash
# Activate VST Development Factory on operator VPS / Linux / WSL
set -euo pipefail

OC_SCRIPTS="$(cd "$(dirname "$0")" && pwd)"
OC_ROOT="$(cd "$OC_SCRIPTS/.." && pwd)"
REPO="${DLZ_REPO_ROOT:-$(cd "$OC_ROOT/.." && pwd)}"
STATE="${OPENCLAW_STATE_DIR:-$HOME/.openclaw}"

export DLZ_REPO_ROOT="$REPO"
export OPENCLAW_STATE_DIR="$STATE"

echo "DLZ_REPO_ROOT=$REPO"
echo "OPENCLAW_STATE_DIR=$STATE"

bash "$OC_SCRIPTS/install-vst-factory-workspaces.sh"
node "$OC_SCRIPTS/merge-vst-factory-config.mjs"

echo ""
echo "=== Verify repo automation (no OpenClaw required) ==="
bash "$REPO/openclaw/elite-frameworks/automation/run_factory_pipeline.sh"

echo ""
echo "=== Merge into openclaw.json ==="
echo "1) Backup: cp \"$STATE/openclaw.json\" \"$STATE/openclaw.json.bak\""
echo "2) Open:   \"$STATE/disklordz.vst-factory.merged.json5\""
echo "3) Copy into openclaw.json:"
echo "   - skills.load.extraDirs + skills.entries (factory skills enabled)"
echo "   - agents.entries (vst-* pods)"
echo "   - dev.subagents.allowAgents: vst-architect, vst-dsp, vst-gui, vst-build, review"
echo "   - automation.jobs (optional cron)"
echo ""
echo "4) Restart gateway: openclaw gateway restart"
echo ""
echo "=== Mobile API skill ids ==="
echo "arch, dsp, gui, build, preset, qa, release → see ai-agent-team/src/agents/openclawSkillsMap.js"
echo ""
echo "Full doc: openclaw/SETUP-VST-FACTORY.md"