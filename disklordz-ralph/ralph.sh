#!/usr/bin/env bash
set -euo pipefail
RALPH="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$RALPH/.." && pwd)"
cd "$REPO"
export DLZ_REPO_ROOT="$REPO"

MAX="${RALPH_MAX_ITERATIONS:-40}"
LOG="$RALPH/ralph_output.log"
: >"$LOG"

for ((i = 1; i <= MAX; i++)); do
  echo ""
  echo "=== DiskLordz Ralph $i / $MAX ==="
  node "$RALPH/scripts/sync-priority-hint.mjs" || true
  if command -v claude >/dev/null 2>&1; then
  OUTPUT="$(claude -p "$(cat "$RALPH/PROMPT.md")" --dangerously-skip-permissions 2>&1 | tee -a "$LOG")"
  if grep -q '<promise>COMPLETE</promise>' <<<"$OUTPUT"; then
      echo "SUCCESS: All DiskLordz Ralph stories complete."
      exit 0
    fi
  else
    echo "No claude CLI — use Cursor Cloud Agent with $RALPH/PROMPT.md"
    node "$RALPH/scripts/verify-story.mjs" "$(node -e "
const p=require('$RALPH/prd.json');
const n=p.stories.filter(s=>!s.passes&&!s.actor).sort((a,b)=>a.priority.localeCompare(b.priority))[0];
console.log(n?n.id:'');
")" || true
    exit 0
  fi
  sleep 2
done