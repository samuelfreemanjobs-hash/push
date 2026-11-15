# Weekly Beatmaker Short-Form Content Brief

**Schedule:** Every Monday at 10:00 AM (`America/Detroit`)

**Automation:** `disklordz-agent-stack` → `npm run beatmaker-brief` → OpenClaw `MKT:` with skill `beatmaker_content_brief`.

Full prompt (copy for Base44 Scheduled Tasks):

```text
TASK NAME: Weekly Beatmaker Short-Form Content Brief
SCHEDULE: Every Monday at 10:00 AM

INSTRUCTIONS:
1. Research current beatmaking trends, viral production techniques, and common mixing frustrations from the past 7 days across YouTube beatmaking channels, TikTok/Reels producer trends, and producer forums (r/trapproduction, r/makinghiphop, r/WeAreTheMusicMakers). Cite URLs.
2. Select 3 distinct angles:
   - Concept 1 (Trend-Jacking): Recreate a trending beat style or artist sound.
   - Concept 2 (Problem/Solution): Fix a common producer headache in under 15 seconds.
   - Concept 3 (Sound Design / Cookup): Fast melody or drum bounce with minimal sounds.
3. Match each concept to a specific VST, preset, or drum kit from superagent-catalog.md only. Do not invent product features or preset names.
4. For each concept, output a 15–30 second vertical video brief:
   - Title & Target Angle
   - Featured Product & Specific Preset/Sound
   - 0:00–0:03 Hook (On-Screen Text + Spoken Line)
   - 0:03–0:20 DAW Screen Action & Audio Demo (bypass vs active / before vs after)
   - 0:20–0:25 CTA (link in bio → tripwire-sample-v1 or named VST)
   - Screen Recording Cues
   - Audio Hooks
   - Post Caption & 5–8 Niche Hashtags
   - Pinned Comment CTA

DELIVERY:
Subject: Monday Beatmaker Content Brief – [Date]
Send via Telegram (MKT:) / Slack / email per product-catalog.yaml delivery.channels.
```

Repo reference: `openclaw/skills/shared/beatmaker_content_brief/references/superagent-scheduled-task.md`