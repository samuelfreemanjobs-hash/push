# Producer outreach compliance (operator-owned)

OpenClaw **must not** send email without explicit operator approval per lead batch.

## Sourcing

- Only emails producers **publish** for beat inquiries / promos (bio, description, BeatStars contact).
- No login walls, no purchased lists, no inferred `@gmail.com` guesses.

## CAN-SPAM / GDPR hygiene

- Physical mailing address or registered-agent address in footer (operator configures).
- Plain-text opt-out: *"Reply 'no' if you don't want free kits."*
- Store opt-outs in `docs/outreach/producer_leads.csv` (`status=opted_out`).

## Deliverability

- Use a **secondary** warmed domain (e.g. `beats@yourbrand-audio.com`), not receipt/store email.
- Cap **10–20** sends/day during warmup; scale manually after bounce rate checks.
- No attachments on first touch; link to Lite VST landing only after operator approves URL.

## OpenClaw gates

- Harvest script → `pending_review` only.
- Draft step → `docs/outreach/drafts/<slug>.md` (mkt agent).
- SMTP / Gmail API send → operator one-click or manual; sage never auto-sends cold email.