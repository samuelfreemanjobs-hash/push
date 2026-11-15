# /disk-outreach-drafts

Draft producer seeding emails only. **Never send.**

## Steps

1. Invoke **marketing-strategist** with skill **producer-outreach-draft**.
2. Read `docs/outreach/producer_leads.csv` and `openclaw/skills/shared/vst_market_intel/references/producer-outreach-prompt.md`.
3. Write `docs/outreach/drafts/<slug>.md` per pending lead.
4. Output batch summary; operator approves CSV `status=approved` before any manual send.