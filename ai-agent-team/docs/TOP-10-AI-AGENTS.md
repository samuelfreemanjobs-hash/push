# Top 10 AI agent kits (production catalog)

Ten outcome-based agent systems for Etsy digital products. Each kit includes **playbook + 10 workflows + implementation guide**; agents 3–4 include spreadsheets / full onboarding files.

## Build & download

```bash
cd ai-agent-team
npm run products:build
```

| Deliverable | ZIP |
|-------------|-----|
| All 10 agents | `dist/top-10-ai-agents-bundle.zip` |
| Individual agent | `dist/agent-01-etsy-listing-seo-agent-kit.zip` … `agent-10-…zip` |

Regenerate source only: `npm run agents:generate`

## The 10 agents

| # | Agent | Outcome KPI |
|---|--------|-------------|
| 1 | **Etsy Listing SEO** | Listing audit score ≥8/10 before publish |
| 2 | **Social Content Machine** | 30 days of channel-native posts planned |
| 3 | **Marketing Planner** | Weekly plan + Excel command center updated |
| 4 | **Client Onboarding** | Onboarding complete ≤14 days |
| 5 | **Email Sequence** | Full welcome → nurture → launch emails drafted |
| 6 | **Ad Copy** | Meta/Google ad sets with 3 angles each |
| 7 | **Sales Proposal & Discovery** | Proposal + discovery recap sent |
| 8 | **Copy Swipe File** | 50 templates + 10 agent refinement workflows |
| 9 | **POD Design Prompt** | Niche-ready design briefs for POD listings |
| 10 | **Listing Mockup & Photo Brief** | 8–12 listing image briefs + alt text |

## Etsy positioning

Sell each agent as its own listing ($24.99–$49.99) and the bundle at $149–$199. Use outcome titles from [ETSY-KEYWORD-PASS.md](ETSY-KEYWORD-PASS.md); add per-agent titles in a future keyword pass.

## Maintenance

Edit workflow copy in `scripts/agents/top10-definitions.mjs`, then `npm run agents:generate` and `npm run products:build`.
