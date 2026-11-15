---
name: producer-outreach-draft
description: Draft producer seeding emails from vst_market_intel CSV — never send.
allowed-tools: Read, Write, Grep, Glob
---

# Producer outreach draft

Follow `openclaw/skills/shared/vst_market_intel/references/producer-outreach-prompt.md`.

## Prerequisites

- `docs/outreach/producer_leads.csv` with `status=pending_review` (from `find_producer_leads.py`), or operator-supplied lead list
- Lite download URL **from operator** — do not invent URLs

## Steps

1. Filter `pending_review` rows; skip `opted_out` / `sent`.
2. For each lead, write `docs/outreach/drafts/<slug>.md`:
   - Subject line (template in outreach prompt)
   - 4-sentence body with specific beat title + platform URL
   - Compliance footer from `references/outreach-compliance.md` (read if present)
3. Append summary to `docs/outreach/drafts/_batch-summary.md` (paths + subjects only).

## Gates

- No SMTP, Resend, or Gmail API calls
- Affiliate code only if operator set `DLZ_AFFILIATE_CODE`
- Telegram-style brief ≤12 lines if asked