---
name: mem0-dsp-memory
description: Recall and store DiskLordz autoresearch memory via Mem0 Platform or Claude /mem0 commands.
allowed-tools: Read, Bash
---

# Mem0 DSP memory

**Docs:** `docs/dev/mem0-opt-in.md`

## Recall (start of cycle)

- Plugin: `/mem0:search --category results 808 pitch`
- CLI: `python disklordz-ralph/business/mem0/search_dsp_memory.py "<query>"`

Requires `MEM0_API_KEY` for CLI. Default scopes: `DLZ_MEM0_USER_ID`, `DLZ_MEM0_AGENT_ID=memory-curator`, `DLZ_MEM0_APP_ID=disklordz-push`.

## Store (after keep)

```bash
python disklordz-ralph/business/mem0/record_autoresearch_win.py --track cpp
# or --track kit
```

Optional: `$env:DLZ_MEM0_GIT_SHA = (git rev-parse --short HEAD)` before record.

## Dual-write with Ruflo (optional)

If `npx ruflo@latest memory` is active, mirror numeric wins:

`genre=southern_trap TOTAL_LOSS=... sha=... pitch_tau=...`

Prefer **one** source of truth per session — Mem0 for Claude Code, Supermemory for Cursor.