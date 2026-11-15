---
name: vst_market_intel
description: Firecrawl competitor pricing, producer lead harvest, and Superpowers landing-page copy — no auto-send email.
version: 1.0.0
user-invocable: true
metadata:
  {"openclaw":{"emoji":"📉","requires":{"env":["FIRECRAWL_API_KEY"],"bins":["python3"]}}}
homepage: https://github.com/samuelfreemanjobs-hash/push/tree/main/openclaw/skills/shared/vst_market_intel
---

Before acting, read `references/workflow.md`, `references/superpowers-copy-prompt.md`, and (for outreach) `references/producer-outreach-prompt.md` + `references/outreach-compliance.md` via skills_read.

## Price monitor (cron or `MKT: check competitor VST prices`)

1. From repo root (`DLZ_REPO_ROOT`):  
   `python openclaw/skills/shared/vst_market_intel/track_competitors.py`  
   Or after install: `python ~/.openclaw/skills/vst_market_intel/track_competitors.py`
2. Read `docs/market-research/latest_snapshot.json` and `~/.openclaw/hermes-out/research/vst-market-latest.json`.
3. If `alerts` is non-empty, send a **Telegram brief** (≤12 lines) via `$telegram_brief`; do not auto-email producers.

## Superpowers copy (user asks for landing page / tripwire copy)

Follow `references/superpowers-copy-prompt.md`. Load latest snapshot + `disklordz-ralph/vst/LICENSING.md` tiers. Output draft to `docs/copy/landing-page.md` in repo; operator approves before publish.

## Producer seeding & outreach (weekly or `MKT: find new producer leads`)

1. From repo root:  
   `python openclaw/skills/shared/vst_market_intel/find_producer_leads.py`  
   Or after install: `python ~/.openclaw/skills/vst_market_intel/find_producer_leads.py`
2. Read `docs/outreach/producer_leads.csv` (`status=pending_review`) and `docs/outreach/latest_harvest.json`.
3. Draft 4-sentence emails per `references/producer-outreach-prompt.md` → `docs/outreach/drafts/<slug>.md` (hook on `recent_beat_title`, Lite VST + 25 WAVs offer).
4. Telegram brief with draft paths — **human approves before any SMTP/Gmail send** (`references/outreach-compliance.md`).

## Wiring

- Cron examples: `cron-vst-market-intel.json5.example`, `cron-producer-leads.json5.example`
- Deps: `pip install -r track_competitors-requirements.txt` (operator machine or VPS)