---
name: flow_github_actions
description: GitHub Actions handoffs — disklordz-ops-sync, backup sync, workflow fixes.
user-invocable: true
---

- `disklordz-ops-sync.yml` every 6h; token `DISKDLORDZ_OPS_TOKEN`
- Free backup: `disklordz-agent-stack` `npm run backup-sync`
- Fix workflows with minimal YAML diffs; test on branch
- Report workflow URL on failure