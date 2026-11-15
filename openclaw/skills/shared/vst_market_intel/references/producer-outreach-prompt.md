# Producer seeding drafts (mkt)

Prerequisites: `docs/outreach/producer_leads.csv` with `status=pending_review`; Lite download URL in operator vault (not invented).

## After `find_producer_leads.py`

1. List new `pending_review` rows (skip `opted_out`, `sent`).
2. For each lead, write `docs/outreach/drafts/<slug>.md` where `<slug>` = lowercase producer name, spaces → `-`, max 60 chars.
3. Email template (4 sentences, no hype blocks):

**Subject:** `Free Louisiana 808 & Bounce VST for your [Niche] beats`

**Body:**

- Sentence 1: Reference `[recent_beat_title]` and `[platform_url]` specifically.
- Sentence 2: One line on why the Lite pack fits their niche (BR 808, bounce swing, raw WAVs).
- Sentence 3: Lite VST3/AU + 25 Baton Rouge/Bounce WAVs — direct link from operator vault.
- Sentence 4: Optional 30% affiliate code **only if** operator set `DLZ_AFFILIATE_CODE` in env; else omit.

4. Append footer block from `references/outreach-compliance.md` (address + opt-out).
5. Telegram brief via `$telegram_brief`: list draft paths + subjects; **no send**.
6. When operator approves, set CSV `status=approved`; sending is manual or future gated SMTP skill.