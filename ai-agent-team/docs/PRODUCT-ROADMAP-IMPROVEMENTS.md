# Product roadmap — improvements beyond v2.1

You asked what else to add after **vertical packs**, **15s Loom scripts**, and **deeper workflows (13–19)**. Prioritized for Etsy digital sellers selling **outcomes** (systems, not prompt dumps).

## Tier A — Highest impact on conversion & refunds

| Initiative | What buyers get | Why it wins |
|------------|-----------------|-------------|
| **Listing mockup pack** | 5–8 PNG screenshots per agent (scorecard, artifact, folder tree) | Etsy shoppers judge in 2 seconds; proof beats bullet lists |
| **Canva prompt library** | ✅ `docs/CANVA-ART-PROMPTS.md` + per-SKU files — run `npm run products:canva-prompts` | Fast consistent heroes, Pinterest, sales pages |
| **Fillable PDF exports** | Same artifacts as PDF forms (Canva or Acrobat) | Many buyers never open CSV; PDF feels “finished” |
| **Notion duplicate** | One-click duplicate link + matching DB properties | Coaches/consultants live in Notion; lowers setup friction |
| **“First 30 minutes” video** | 3–5 min walkthrough per agent (not 15s teaser) | Cuts support messages; justifies premium vs $5 prompt packs |
| **Commercial license tiers** | Personal / client work / agency resale add-on | Upsell on same ZIP; clear legal story |

## Tier B — Catalog & revenue expansion

| Initiative | Notes |
|------------|--------|
| **Vertical listing SKUs** | Separate Etsy listings: “Etsy Seller Edition” vs “Coach Edition” — same ZIP, different hero image + keywords |
| **Agent #21 — Bookkeeping snapshot** | Weekly cash narrative + categorization prompts + Excel tab (pairs with agent 18) |
| **Mega-bundle listing** | Top 20 + Marketing Excel + postcards + business-in-a-box with one keyword-optimized listing doc |
| **Update subscription** | Annual “v2.x refresh” email + changelog; optional Gumroad/Patreon for updates |
| **n8n / Make mini-flows** | One JSON per agent: “Monday SEO loop” triggers folder + reminder (no API keys required) |

## Tier C — Quality & trust

| Initiative | Notes |
|------------|--------|
| **Human-edited Acme samples** | Replace generic Acme with niche-specific samples (Etsy planner, coach intake, local HVAC) |
| **Scorecard → auto checklist** | Google Sheet or Excel that sums weights when buyer checks boxes |
| **Policy snippets** | Etsy-safe disclaimer blocks per agent (email, ads, health/finance guardrails) |
| **Review response kit** | Tie agent 11 + 19: templates when buyers leave 4★ with feedback |

## Tier D — Automation (your shop, not the ZIP)

| Initiative | Notes |
|------------|--------|
| **Etsy daily pipeline** | Already in `integrations/n8n/` — wire `GEMINI_API_KEY` + OAuth for draft listings |
| **Keyword refresh cron** | Monthly re-run agent 01 on top 5 SKUs; log in `etsy-store.yaml` |
| **Zapier MCP** | Post new listing to Pinterest/Buffer after human approve (writes need confirmation) |
| **Proof collection** | Loom library per agent → embed in listing gallery |

## Suggested sequence (technical, not calendar)

1. Ship **v2.1** ZIPs (verticals + Loom + deep 13–19) — done in repo via `npm run agents:generate` + `products:build`.
2. Generate **mockup PNGs** from scorecard + artifact (batch script or Figma template).
3. Add **Notion templates** for agents 04, 05, 13, 20 (highest coach demand).
4. Publish **3 vertical-specific listings** for your top 3 revenue agents (01, 03, 04).
5. **Agent 21** + bookkeeping Excel as new SKU.
6. Turn on **n8n daily** in production with publish gated behind manual approval.

## Commands

```bash
cd ai-agent-team
npm run agents:generate   # workflows + v2 + v2.1
npm run products:build      # ZIPs
```

## v2.1 buyer-facing summary (for listings)

- **08-verticals/** — paste-ready niche context (Etsy / coach / local)
- **09-video-scripts/** — 15-second shop preview script
- **Agents 13–19** — workflow briefs aligned with top-10 depth (deliverables + QC)
