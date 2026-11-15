---
name: vst_market_intel
description: Firecrawl competitor drum VST pricing plus Superpowers landing-page copy workflow.
version: 1.0.0
user-invocable: true
metadata:
  {"openclaw":{"emoji":"📉","requires":{"env":["FIRECRAWL_API_KEY"],"bins":["python3"]}}}
homepage: https://github.com/samuelfreemanjobs-hash/push/tree/main/openclaw/skills/shared/vst_market_intel
---

Before acting, read `references/workflow.md` and `references/superpowers-copy-prompt.md` via skills_read.

## Price monitor (cron or `MKT: check competitor VST prices`)

1. From repo root (`DLZ_REPO_ROOT`):  
   `python openclaw/skills/shared/vst_market_intel/track_competitors.py`  
   Or after install: `python ~/.openclaw/skills/vst_market_intel/track_competitors.py`
2. Read `docs/market-research/latest_snapshot.json` and `~/.openclaw/hermes-out/research/vst-market-latest.json`.
3. If `alerts` is non-empty, send a **Telegram brief** (≤12 lines) via `$telegram_brief`; do not auto-email producers.

## Superpowers copy (user asks for landing page / tripwire copy)

Follow `references/superpowers-copy-prompt.md`. Load latest snapshot + `disklordz-ralph/vst/LICENSING.md` tiers. Output draft to `docs/copy/landing-page.md` in repo; operator approves before publish.

## Wiring

- Cron example: `openclaw/disklordz-picks/cron-vst-market-intel.json5.example`
- Deps: `pip install -r track_competitors-requirements.txt` (operator machine or VPS)