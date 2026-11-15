#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
STATE="${OPENCLAW_STATE_DIR:-$HOME/.openclaw}"

for id in drum-lead grok-bridge drum-qc drum-pack drum-manifest; do
  dest="$STATE/workspace-$id"
  mkdir -p "$dest/kits"
  if [[ -f "$ROOT/workspaces/$id/AGENTS.md" ]]; then
    cp "$ROOT/workspaces/$id/AGENTS.md" "$dest/AGENTS.md"
    echo "Installed $dest/AGENTS.md"
  fi
  cp "$ROOT/handoff/kit-spec.template.yaml" "$dest/kits/kit-spec.template.yaml"
  cp "$ROOT/handoff/grok-architect-prompt.md" "$dest/grok-architect-prompt.md"
done

echo ""
echo "Register drum factory agents:"
for id in drum-lead grok-bridge drum-qc drum-pack drum-manifest; do
  echo "  openclaw agents add $id --workspace $STATE/workspace-$id --non-interactive"
done
echo ""
echo "Validate a Grok export:"
echo "  cd $ROOT && npm install && npm run validate:kit-spec -- ./handoff/kit-spec.template.yaml"