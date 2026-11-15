# DiskLordz Technology Intelligence (AI Dev Signals)

Lightweight **technology radar** for solo founder ops: one input feed ([AI Dev Signals](https://www.aidevsignals.com/)), six screening categories, short reports instead of article hoarding.

## Categories

| Category | What to watch for |
|----------|-------------------|
| Audio AI | Sound generation, recognition, separation, tagging |
| Coding AI | C++, build-system, and agent coding tools |
| DSP research | Synthesis, filtering, physical modeling |
| Local models | Affordable CPU/GPU inference and quantization |
| Agent frameworks | Testing, orchestration, automation |
| Commercial opportunity | Plugin or SaaS feature you could ship |

## Report fields (per kept item)

- Technology summary (plain language)
- Licensing
- Integration difficulty (1–5)
- Expected cost
- **Product fit** — VST factory, tripwire packs, command center, agent stack, or none
- Verdict: **adopt**, **watch**, or **skip**

Implementation details live in `lib/tech-radar.mjs`.

## Run (free tier)

```bash
cd disklordz-agent-stack
npm run tech-radar
```

Writes under `~/.openclaw/hermes-out/research/tech-radar/`:

| File | Purpose |
|------|---------|
| `intake.json` | RSS metadata + category rubric |
| `REVIEW-YYYY-MM-DD.md` | Prompt template for RADAR agent |
| `report-YYYY-MM-DD.md` | Filled by OpenClaw (not by this script) |

### Inbox fallback

If RSS is blocked (some VPS egress policies) or you read email offline:

```bash
mkdir -p ~/.openclaw/hermes-out/research/tech-radar/inbox
# Paste newsletter text or export
nano ~/.openclaw/hermes-out/research/tech-radar/inbox/issue.md
npm run tech-radar
```

Override feed URL:

```bash
export AI_DEV_SIGNALS_FEED_URL='https://rss.beehiiv.com/feeds/f5e63dd4-5653-4f09-83e2-321a8b1ba526.xml'
```

## OpenClaw

Install skills, then message:

```text
RADAR: review latest AI Dev Signals issue
```

Skill: `openclaw/skills/shared/tech_radar_signals`

Agent registry: `radar` pod in `openclaw/agents/REGISTRY.yaml` (skill shared with `arch` and `mkt`).

## Cron suggestion

After the newsletter lands (US morning):

```cron
35 14 * * 1-5 cd $DLZ_REPO_ROOT/disklordz-agent-stack && npm run tech-radar
```

Pair with SAGE morning brief or a dedicated Telegram `RADAR:` thread.