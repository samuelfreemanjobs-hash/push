# Copy-paste: scheduled task prompt (Base44 / OpenClaw / Hermes)

Point knowledge base at: `disklordz-agent-stack/catalog/product-catalog.yaml` + `PRODUCT-CATALOG-CHEATSHEET.md`.

```text
TASK NAME: Weekly Beatmaker Short-Form Content Brief
SCHEDULE: Every Monday at 10:00 AM (America/Detroit)

INSTRUCTIONS:
1. Research current beatmaking trends, viral production techniques, and common mixing frustrations from the past 7 days across YouTube beatmaking channels, TikTok/Reels producer trends, and producer forums (r/trapproduction, r/makinghiphop, r/WeAreTheMusicMakers). Cite URLs.
2. Select 3 distinct angles:
   - Concept 1 (Trend-Jacking): Recreate a trending beat style or artist sound.
   - Concept 2 (Problem/Solution): Fix a common producer headache in under 15 seconds.
   - Concept 3 (Sound Design / Cookup): Fast melody or drum bounce with minimal sounds.
3. Match each concept to a specific VST, preset, or drum kit from the uploaded Product Catalog. Do not invent product features or preset names.
4. For each concept, output a 15–30 second vertical video brief:
   - Title & Target Angle
   - Featured Product & Specific Preset/Sound
   - 0:00–0:03 Hook (On-Screen Text + Spoken Line)
   - 0:03–0:20 DAW Screen Action & Audio Demo (bypass vs active / before vs after)
   - 0:20–0:25 CTA (link in bio → $9 starter pack or named VST)
   - Screen Recording Cues
   - Audio Hooks
   - Post Caption & 5–8 Niche Hashtags
   - Pinned Comment CTA

DELIVERY:
Send via Telegram (MKT:) / Slack / email — subject: Monday Beatmaker Content Brief – [Date]
```

DiskLordz automation: run `npm run beatmaker-brief` on VPS first, then invoke this prompt against `hermes-out/research/beatmaker-brief/LATEST.md`.