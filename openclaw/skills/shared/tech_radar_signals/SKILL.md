---
name: tech_radar_signals
description: Screen AI Dev Signals issues into DiskLordz tech-radar reports (six categories, no article archive).
version: 1.0.0
user-invocable: true
homepage: https://github.com/samuelfreemanjobs-hash/push/tree/main/openclaw/skills/shared/tech_radar_signals
---

Before acting, read `references/workflow.md` via skills_read.

Inputs:

- `~/.openclaw/hermes-out/research/tech-radar/intake.json`
- Latest `REVIEW-*.md` and optional `inbox/latest-body.md`

Output:

- `~/.openclaw/hermes-out/research/tech-radar/report-YYYY-MM-DD.md`
- Prefix replies with `RADAR:`
- If any `verdict: adopt`, one bullet for SAGE morning brief

Do not save full newsletter text long-term; keep screened items only.