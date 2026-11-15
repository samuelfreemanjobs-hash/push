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

## OpenClaw + Telegram (start here)

Step-by-step: [`../SETUP-TELEGRAM.md`](../SETUP-TELEGRAM.md)

```bash
bash openclaw/scripts/setup-disklordz-telegram.sh
```

Windows: `.\openclaw\scripts\setup-disklordz-telegram.ps1`

## Install order

### Free ($0) — start here

1. **`../../disklordz-agent-stack/`** — cron → `~/.openclaw/hermes-out/` (Hermes optional)
2. OpenClaw + Gemini free + `openclaw.stack.json5.example`
3. `disklordz-agent-stack/deploy/bootstrap-free.sh`

See **`../../disklordz-agent-stack/FREE.md`**.

### Hermes upgrade (later)

1. `../../hermes/disklordz-picks/` webhooks/crons (same `hermes-out/` paths)
2. Codex only if you already have a subscription

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