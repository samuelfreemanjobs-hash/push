#!/usr/bin/env bash
set -euo pipefail
OC_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
STATE="${OPENCLAW_STATE_DIR:-$HOME/.openclaw}"
SNIPPET="$OC_ROOT/vst-factory-workspaces/_AGENTS_SNIPPET.md"

PODS=(vst-architect vst-dsp vst-gui vst-build vst-preset vst-qa vst-release)

for id in "${PODS[@]}"; do
  dest="$STATE/$id"
  mkdir -p "$dest"
  if [[ -f "$SNIPPET" ]]; then
    {
      echo "# Pod: $id"
      cat "$SNIPPET"
    } > "$dest/AGENTS.md"
  fi
done

# Ensure dev can delegate (workspace already exists from elite picks)
if [[ -d "$STATE/workspace-dev" ]] && [[ -f "$SNIPPET" ]]; then
  if ! grep -q "vst-factory-pods" "$STATE/workspace-dev/AGENTS.md" 2>/dev/null; then
    echo "" >> "$STATE/workspace-dev/AGENTS.md"
    echo "## VST factory subagents" >> "$STATE/workspace-dev/AGENTS.md"
    echo "Delegate to: vst-architect, vst-dsp, vst-gui, vst-build (see REGISTRY.yaml)." >> "$STATE/workspace-dev/AGENTS.md"
  fi
fi

echo "VST factory workspaces under $STATE/{${PODS[*]// /,}}"