---
name: launch_gates
description: DiskLordz launch gates G1-G6 status and unblock actions.
user-invocable: true
---

Use ops-snapshot `launchGates` and kanban P0 links.

| Gate | Theme |
|------|--------|
| G1 | Vercel production secrets |
| G2 | Tripwire checkout webhook |
| G3 | QC'd WAV pack |
| G4 | HTTPS deploy |
| G5 | LAUNCH_CAMPAIGN_START |
| G6 | Slack bootstrap |

For each blocked gate: one owner prefix and single next action.