---
name: disk_ops_snapshot
description: Read DiskLordz ops-snapshot.json and hermes-out for morning briefs and status.
version: 1.0.0
user-invocable: true
metadata:
  {"openclaw":{"emoji":"📊","requires":{"env":["DLZ_REPO_ROOT"]}}}
homepage: https://github.com/samuelfreemanjobs-hash/push/tree/main/openclaw/skills/shared/disk_ops_snapshot
---

Before acting, read `references/workflow.md` (and sibling files under `references/`) via skills_read.


When asked for ops status, morning brief input, or kanban counts:

1. Read `disklordz-command-center/public/ops-snapshot.json` under `DLZ_REPO_ROOT`.
2. Read `~/.openclaw/hermes-out/engineering-report.json` if present.
3. Summarize: P0 cards, blocked items, vault PRs, tripwire step active, VST CI fleet.
4. Never invent metrics — cite fields from JSON only.
5. End with up to 3 suggested prefixes (`FLOW:`, `DRUM:`, `BIZ:`).