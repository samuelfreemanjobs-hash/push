# DiskLordz Hermes layer (your H picks)

Hermes runs **webhooks, crons, Codex `/bg`, research loops, PR review, orchestration**.  
**Telegram = OpenClaw only.** Hermes writes to `~/.openclaw/hermes-out/` for sage/mkt/biz/dev to read.

## Handoff directory

```text
~/.openclaw/hermes-out/
  engineering-report.json    # scheduled engineering report (I3)
  biz-metrics.json           # Stripe/GitHub reads for Friday KPI
  pr-review/<n>.md           # code review drafts (approve before post)
  relay/<ts>-code.md           # relay code output for OC
  research/
    youtube-weekly.md
    reddit-pain.md
    revenue-opportunities.md
```

After each job, optional: `curl` local OpenClaw hook or notify sage cron on next brief.

## Install

```bash
# Hermes gateway on same VPS as OpenClaw
hermes gateway setup
# Register webhooks: see webhooks/github.yaml
# Register crons: see cron/disklordz.yaml
```

## Your H picks (mapped)

| Pick | Implementation |
|------|----------------|
| Webhooks | `webhooks/github.yaml` |
| Crons | `cron/disklordz.yaml` |
| Drum lead tripwire cron | `tripwire-factory-status` job |
| Codex | `/bg` templates in `codex/overnight.md` |
| Creative sound design | `sessions/drum-architect.md` (export kit-spec for OC) |
| YouTube/competitor spy | `research/youtube-competitor.md` |
| Reddit pain points | `research/reddit-pain.md` |
| Revenue opportunities | `research/revenue-scan.md` |
| Relay code output | `relay/code-output-template.md` |
| Shared memory | Hermes memory plugin → same vault path as OC |
| Code review | PR webhook → `pr-review` artifact |
| Scheduled engineering reports | `engineering-weekly` cron |
| Full orchestration | `orchestration/research-pipeline.yaml` |

## Grok Drum Kit Architect

Creative sessions live here (Hermes or Grok Bot). Export `kit-spec.yaml` → validate → OpenClaw `DRUM:` (`openclaw/drum-kit-factory/`).

## Duplication guard

- Hermes does **not** open Telegram DM to you for routine briefs (OC sage does).
- Hermes may `POST` one-line to OpenClaw internal webhook if you configure it—otherwise file drop only.