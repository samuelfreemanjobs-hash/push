---
name: overnight_branch
description: Overnight coding on branch only with draft PR and build gate.
version: 1.0.0
user-invocable: true
homepage: https://github.com/samuelfreemanjobs-hash/push/tree/main/openclaw/skills/shared/overnight_branch
---

Before acting, read `references/workflow.md` (and sibling files under `references/`) via skills_read.


Contract:

- Branch `overnight/YYYY-MM-DD-slug` only; never push main.
- `npm run build` / `tsc` / relevant CI must pass before notifying.
- `gh pr create --draft` when possible.
- No prod deploy, Stripe live, vault merge.
- Write summary to `hermes-out/relay/` for sage morning brief.
- VST: mention pluginval + Windows CI in PR body.