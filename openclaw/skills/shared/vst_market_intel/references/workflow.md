# vst_market_intel workflow

## Environment

- `FIRECRAWL_API_KEY` — operator secret (OpenClaw env or `~/.openclaw/.env`, never commit)
- `DLZ_REPO_ROOT` — git root containing `docs/market-research/`

## Outputs

| File | Purpose |
|------|---------|
| `docs/market-research/latest_snapshot.json` | Last run + alerts |
| `docs/market-research/price_history.json` | Deduped competitor keys |
| `hermes-out/research/vst-market-latest.json` | MKT pod / morning brief |

## Alert policy

Message user only when:

- Price drop vs previous snapshot
- Scrape error on target URL
- New product name not seen in `price_history.json`

## Outreach outputs

| File | Purpose |
|------|---------|
| `docs/outreach/producer_leads.csv` | Deduped leads (`pending_review` → operator) |
| `docs/outreach/latest_harvest.json` | Last agent run summary |
| `docs/outreach/drafts/*.md` | Personalized emails (not sent by agent) |
| `hermes-out/research/producer-leads-latest.json` | MKT / sage brief |

## Legal

Public pricing pages only. No DRM circumvention, no login-walled scraping.

Producer emails: **public promo addresses only**; drafts only until operator approves; no autonomous cold send (`references/outreach-compliance.md`).