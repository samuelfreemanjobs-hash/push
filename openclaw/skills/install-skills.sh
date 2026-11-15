#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
STATE="${OPENCLAW_STATE_DIR:-$HOME/.openclaw}"

mkdir -p "$STATE/skills"
for d in "$ROOT/shared"/*; do
  [[ -d "$d" ]] || continue
  name="$(basename "$d")"
  rm -rf "$STATE/skills/$name"
  cp -a "$d" "$STATE/skills/$name"
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
    rm -rf "$dest/$bn"
    cp -a "$d" "$dest/$bn"
  done
}

for id in sage dev arch mkt biz radar drum-lead grok-bridge drum-qc drum-pack drum-manifest review; do
  install_agent_skills "$id"
done

for id in sage dev arch mkt biz radar drum-lead review; do
  mkdir -p "$STATE/workspace-$id/skills"
  for d in "$ROOT/shared"/*; do
    [[ -d "$d" ]] || continue
    bn="$(basename "$d")"
  if [[ ! -d "$STATE/workspace-$id/skills/$bn" ]]; then
      cp -a "$d" "$STATE/workspace-$id/skills/$bn"
    fi
  done
done

echo "Installed skills (with references/) to $STATE/skills"
echo "ClawHub prep: cd $ROOT && npm run verify && npm run publish:prep"
echo "openclaw.json: skills.load.extraDirs: [\"$ROOT/shared\"]"