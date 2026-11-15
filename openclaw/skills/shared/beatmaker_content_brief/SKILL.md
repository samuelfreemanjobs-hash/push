---
name: beatmaker_content_brief
description: Weekly beatmaker short-form brief — trends, catalog match, shot scripts, captions (Mon 10:00).
version: 1.0.0
user-invocable: true
homepage: https://github.com/samuelfreemanjobs-hash/push/tree/main/openclaw/skills/shared/beatmaker_content_brief
---

Before acting, read `references/workflow.md` and `references/superagent-scheduled-task.md` via skills_read.

Inputs:

- `~/.openclaw/hermes-out/research/beatmaker-brief/LATEST.md`
- `~/.openclaw/hermes-out/research/beatmaker-brief/intake.json`
- Repo catalog: `disklordz-agent-stack/catalog/product-catalog.yaml`

Output:

- Completed brief: `~/.openclaw/hermes-out/research/beatmaker-brief/COMPLETE-YYYY-MM-DD.md`
- Prefix `MKT:`; deliver via `telegram_brief` when delivery.telegram enabled
- Never invent preset names or product features