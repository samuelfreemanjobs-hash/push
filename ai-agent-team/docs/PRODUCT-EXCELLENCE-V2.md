# Product excellence v2 (all 20 agents)

Every agent kit now ships **version 2.0.0** with three differentiators vs typical Etsy prompt PDFs:

1. **`04-examples/acme-sample.md`** — fictional “Acme Digital Studio” finished output  
2. **`06-scorecard.md`** — 100-point rubric (default pass: **85**)  
3. **`07-artifacts/`** — fillable CSV or Markdown you can use without AI  

Also: `VERSION.txt`, `CHANGELOG.md`, and README section **v2.0 — Excellence pack**.

## Regenerate

```bash
npm run agents:generate   # workflows + excellence
npm run products:build    # refresh Excel example tab + ZIPs
```

## Artifact index

| Agent | Artifact |
|-------|----------|
| 01 | `shop-tag-dedupe-tracker.csv` |
| 02 | `30-day-content-calendar-template.csv` |
| 03 | `weekly-marketing-review.md` + Excel **Example Acme Week** tab |
| 04 | `onboarding-milestone-checklist.md` |
| 05 | `email-sequence-planner.csv` |
| 06 | `ad-compliance-checklist.md` |
| 07 | `proposal-fill-in-template.md` |
| 08 | `copy-tracker.csv` |
| 09 | `pod-prompt-log.csv` |
| 10 | `listing-shot-list.csv` |
| 11 | `support-macro-library.csv` |
| 12 | `sop-template.md` |
| 13 | `lead-magnet-one-pager-outline.md` |
| 14 | `direct-mail-campaign-tracker.csv` |
| 15 | `brand-voice-card.md` |
| 16 | `article-outline-template.md` |
| 17 | `sku-priority-matrix.csv` |
| 18 | `agent-routing-map.md` |
| 19 | `etsy-stats-weekly-paste.csv` |
| 20 | `coaching-intake-template.md` |

## Edit rubrics

`scripts/agents/excellence-v2-data.mjs` → `npm run agents:generate`
