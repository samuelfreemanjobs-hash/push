---
name: flow_github_actions
description: GitHub Actions handoffs — disklordz-ops-sync, backup sync, workflow fixes.
version: 1.0.0
user-invocable: true
homepage: https://github.com/samuelfreemanjobs-hash/push/tree/main/openclaw/skills/shared/flow_github_actions
---

Before acting, read `references/workflow.md` (and sibling files under `references/`) via skills_read.


- `disklordz-ops-sync.yml` every 6h; token `DISKDLORDZ_OPS_TOKEN`
- Free backup: `disklordz-agent-stack` `npm run backup-sync`
- Fix workflows with minimal YAML diffs; test on branch
- Report workflow URL on failure