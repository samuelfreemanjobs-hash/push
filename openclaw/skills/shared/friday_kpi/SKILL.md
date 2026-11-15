---
name: friday_kpi
description: Friday KPI narrative from biz-metrics.json and ops-snapshot only.
user-invocable: true
---

Read `~/.openclaw/hermes-out/biz-metrics.json` and ops-snapshot.

Sections (8-12 lines max):

1. Factory (VST CI counts)
2. Vault PRs (open, stale)
3. Launch / tripwire active step
4. Business (Stripe only if present in JSON — else "not wired")

Never fabricate MRR. Suggest one BIZ/PLAN action for next week.