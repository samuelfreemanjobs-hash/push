# Marketing AI agents — Claude Code & Cursor

DiskLordz splits **marketing** from **DSP/build** agents. All paid publish paths require **human approval** (`STACK-FROZEN.md`).

## When to use which IDE

| Task | Cursor | Claude Code |
|------|--------|---------------|
| Long-form copy + storefront HTML drafts | `@marketing-drum-commerce` rule, edit `storefront/` with review | `/disk-marketing-kit` → `marketing-strategist` |
| Launch bullets, changelog, 30-day sequencing | `@agency-launch-strategist` | `marketing-strategist` + `docs/launch/` |
| Shorts / Reels **scripts** (not render) | `@agency-short-video` | Same; output `docs/launch/shorts-*.md` |
| OpenMontage promo prompts | Chat + `docs/marketing/openmontage-prompt-templates.md` | After `/disk-export-kit`, Template A/B |
| Competitor / producer intel | Read-only; OpenClaw `vst_market_intel` | Draft only via `/disk-outreach-drafts` |
| Sample pack commercial copy | `audio-factory` manifest → marketing agent | `marketing-pack-launch` skill |

## Claude Code (Command → Agent → Skill)

| Command | Agent | Skill |
|---------|--------|--------|
| `/disk-marketing-kit` | `marketing-strategist` | `marketing-pack-launch` |
| `/disk-outreach-drafts` | `marketing-strategist` | `producer-outreach-draft` |
| `/disk-export-kit` | `kit-producer` | `commercial-kit-export` → then marketing handoff |

Agents: `.claude/agents/marketing-strategist.md`  
Skills: `.claude/skills/marketing-pack-launch/`, `producer-outreach-draft/`

## Cursor rules (invoke with `@`)

| Rule | Use for |
|------|---------|
| `marketing-drum-commerce` | Storefront bullets, pack README, BeatStars/Splice-style descriptions |
| `agency-launch-strategist` | Lite → Core → Suite funnel, `docs/launch/` |
| `agency-short-video` | Vertical promo scripts |
| `agency-ui-designer` | WebView / storefront visual tone (gunmetal + amber) |
| `agency-reality-checker` | No invented features; cross-check `LICENSING.md` |

Enable rules per chat via **@ rule name** or attach to `storefront/**` globs.

## AutoGen / factory (not marketing writes)

| Tool | Role |
|------|------|
| `factory_swarm.py` | DSP + WAV mastering + preset XML — **no** storefront API |
| `run_customer_concierge.py` | Read-only support triage |
| Future outreach swarm | Must use approve-then-send; mirror `producer-outreach-prompt.md` |

## Source-of-truth files

| Asset | Path |
|-------|------|
| Brand / legal | `LICENSING.md`, `disklordz-ralph/STACK-FROZEN.md` |
| Kit manifest | `factory/autoresearch/outputs/packs/*/manifest.json` |
| Factory pack | `factory/audio-factory/dist_sample_pack/manifest.json` |
| OpenMontage brief | `assets/latest_release/release_brief.json` (operator tree) |
| Promo templates | `docs/marketing/openmontage-prompt-templates.md` |
| Outreach drafts | `docs/outreach/drafts/` (never auto-send) |
| Storefront | `storefront/` |

## Human gates (non-negotiable)

1. Publish `storefront/`, Stripe, Gumroad, or email SMTP  
2. Claim copyrighted samples, iLok, or unshipped features  
3. Edit `LICENSING.md` without legal review  

## Quick start

**Cursor**

```
@marketing-drum-commerce Write BeatStars listing from factory/audio-factory/dist_sample_pack/manifest.json — draft only under docs/launch/
```

**Claude Code**

```
/disk-marketing-kit
```

Requires a completed kit export or `dist_sample_pack/manifest.json`.