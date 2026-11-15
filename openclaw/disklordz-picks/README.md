# DiskLordz OC + Hermes operating model (your picks)

One **Telegram** front door on **OpenClaw**. **Hermes** runs webhooks, heavy cron, Codex, research loops, and orchestration—then **hands artifacts to OpenClaw** (files, relay), never a second Telegram bot.

## Division

| Pillar | OpenClaw (VPS) | Hermes (VPS, same host OK) |
|--------|------------------|----------------------------|
| **Front door** | Telegram → `sage` | No Telegram (relay only) |
| **Pods** | dev, mkt, biz (+ sage) | Subagents / kanban workers |
| **Kanban truth** | Read vault + Command Center snapshot | Agent task queue only |
| **Morning brief** | Cron 06:00 `sage` | Engineering report feeds brief |
| **Friday KPI** | Cron Fri 16:00 `biz` | Scheduled report job → JSON file |
| **Launch gates** | `plan` / `biz` agents | — |
| **GitHub Actions backup** | Cron re-run sync if stale | Webhooks on fail |
| **Overnight** | `dev` branch + draft PR | Codex `/bg` for heavy code/VST |
| **Memory** | `workspace-shared/` vault mirror | Shared memory + session recall |
| **Drum tripwire cron** | `drum-lead` on ingest/QC | Cron tripwire step ping |
| **Creative sound** | Ingest only (`grok-bridge`) | Grok/Codex creative + YouTube/Reddit |
| **Code review relay** | `review` / merge decision | Hermes PR webhook → artifact |

Full pick lists: `PICKS.yaml`.

## Install order

1. OpenClaw: `../disklordz-elite-team/` + `../drum-kit-factory/` + merge `openclaw.stack.json5.example`
2. Hermes: `../../hermes/disklordz-picks/` + webhooks + crons
3. Env: `DISKLORDZ_OPS_TOKEN`, `GEMINI`, optional Codex, `gh auth login` on VPS

## Daily flow

```text
06:00  OC sage  ← morning brief (snapshot + Hermes engineering-report.json)
       H cron  → writes hermes/out/engineering-report.json

Day    H webhooks → PR/CI events → hermes/out/events/
       H research → youtube/reddit briefs → hermes/out/research/

You    Telegram → sage | DEV: | MKT: | BIZ: | DRUM:

23:00  OC dev overnight goal OR H /bg Codex (one repo, branch only)

Fri    OC biz Friday KPI ← biz-metrics.json (Hermes scheduled + GitHub APIs)
```