# research_consume workflow

## Sources

- `hermes-out/` research stubs
- `hermes-out/research/vst-market-latest.json` (from **vst_market_intel** cron; price drops only)
- `hermes-out/research/producer-leads-latest.json` (weekly harvest summary; angles only, no PII in Telegram)
- Public web only when operator enables browser/search tools

## Steps

1. Ingest report JSON/Markdown from hermes-out.
2. Deduplicate against vault kanban notes.
3. Propose 1–3 kanban cards or sage briefing bullets.
4. No auto-post to social or email.