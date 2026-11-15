#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
STATE="${OPENCLAW_STATE_DIR:-$HOME/.openclaw}"

mkdir -p "$STATE/skills"
cp -R "$ROOT/shared/"* "$STATE/skills/" 2>/dev/null || true
for d in "$ROOT/shared"/*; do
  [[ -d "$d" ]] || continue
  name="$(basename "$d")"
  mkdir -p "$STATE/skills/$name"
  cp "$d/SKILL.md" "$STATE/skills/$name/SKILL.md"
done

install_agent_skills() {
  local id="$1"
  local src="$ROOT/by-agent/$id"
  local dest="$STATE/workspace-$id/skills"
  [[ -d "$src" ]] || return 0
  mkdir -p "$dest"
  for d in "$src"/*; do
    [[ -d "$d" ]] || continue
    bn="$(basename "$d")"
    mkdir -p "$dest/$bn"
    cp "$d/SKILL.md" "$dest/$bn/SKILL.md"
  done
}

for id in sage dev mkt biz drum-lead grok-bridge drum-qc drum-pack drum-manifest review; do
  install_agent_skills "$id"
done

# Link shared into each workspace for highest precedence copies (optional duplicate)
for id in sage dev mkt biz drum-lead review; do
  mkdir -p "$STATE/workspace-$id/skills"
  for d in "$ROOT/shared"/*; do
    [[ -f "$d/SKILL.md" ]] || continue
    bn="$(basename "$d")"
    if [[ ! -f "$STATE/workspace-$id/skills/$bn/SKILL.md" ]]; then
      mkdir -p "$STATE/workspace-$id/skills/$bn"
      cp "$d/SKILL.md" "$STATE/workspace-$id/skills/$bn/SKILL.md"
    fi
  done
done

echo "Installed skills to $STATE/skills and workspace-*/skills"
echo "Add to openclaw.json: skills.load.extraDirs: [\"$ROOT/shared\"]"