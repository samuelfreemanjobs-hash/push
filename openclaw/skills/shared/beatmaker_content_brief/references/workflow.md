# beatmaker_content_brief workflow

## Schedule

Monday **10:00** in catalog timezone (`America/Detroit` default). Cron: `npm run beatmaker-brief` then OpenClaw `MKT:` completion.

## Steps

### 1. Live trend & pain research (7-day lookback)

Sources (must cite URLs):

- YouTube beatmaking tutorials (queries in `intake.json` → `research.youtube_queries`)
- TikTok/Reels (`research.tiktok_hashtags`)
- Reddit: r/trapproduction, r/makinghiphop, r/WeAreTheMusicMakers

Extract **three distinct angles** for the week (feeds concepts 1–3).

### 2. Product & preset matching

Load `product-catalog.yaml`. Pick `products[].id` and exact `presets_sounds[].label`. If no fit, state **no match** — do not invent.

### 3. Shot-by-shot scripts (×3)

Each concept: **15–30s vertical** with:

- Title & target angle
- Featured product + preset/sound
- **0:00–0:03** hook (on-screen text + spoken line) — finished beat or pattern interrupt
- **0:03–0:20** DAW cues — clicks, bypass vs active, before/after audio
- **0:20–0:25** CTA — tripwire `tripwire-sample-v1` or relevant VST
- **Screen recording cues** — region zoom, plugin name visible, meter legible
- **Audio hooks** — what listener hears at 0:00

### 4. Captions & tags

Per concept: platform caption (TikTok + Reels + YouTube Shorts variants), 5–8 niche hashtags, pinned comment CTA.

## Delivery

Subject: `Monday Beatmaker Content Brief – [Date]`

Channels from catalog `delivery.channels`. Telegram: shorten with `telegram_brief` rules then link to full `COMPLETE-*.md`.

## Concept templates

1. **Trend-jacking** — trending style / artist sound
2. **Problem/solution** — muddy 808, thin drums, harsh highs, etc.
3. **Sound design cookup** — minimal-sound bounce challenge