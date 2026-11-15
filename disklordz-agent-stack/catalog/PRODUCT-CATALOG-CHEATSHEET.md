# Product catalog cheat sheet (Superagent / MKT knowledge)

**Canonical file:** `product-catalog.yaml` in this folder. When matching trends to products, use **only** ids, preset labels, and `solves` bullets from that file.

**Base44 upload:** Markdown mirror at `docs/superagent/superagent-catalog.md` + FAQ at `docs/superagent/superagent-faq.md`. See `docs/superagent/README.md`.

## Flagship offers for CTAs

| Offer | ID | Hook |
|-------|-----|------|
| Entry tripwire | `tripwire-sample-v1` | $9 · 5 WAVs · +75 credits |
| Lead VST (in dev) | `detroit-phonk-trap-maker` | Playable Detroit phonk trap drums in DAW |
| Punch / mix | `drum-crush`, `stem-shield`, `phonk-tape` | Before/after on bus or stems |

## Editing checklist

1. Set `delivery.channels` (Telegram / Slack / email).
2. Replace `offers.free_entry.url_placeholder` with live checkout URL.
3. Add real preset names as you ship banks (never let agents invent names).
4. Set `timezone` if not `America/Detroit`.

## Agent rule

If no catalog product fits a trend, say **no match** — do not fabricate a plugin or preset.