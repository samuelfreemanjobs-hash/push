---
name: overnight_branch
description: Overnight coding on branch only with draft PR and build gate.
user-invocable: true
---

Contract:

- Branch `overnight/YYYY-MM-DD-slug` only; never push main.
- `npm run build` / `tsc` / relevant CI must pass before notifying.
- `gh pr create --draft` when possible.
- No prod deploy, Stripe live, vault merge.
- Write summary to `hermes-out/relay/` for sage morning brief.
- VST: mention pluginval + Windows CI in PR body.