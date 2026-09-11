# Top 20 AI agent kits (market-demand catalog)

Twenty outcome-based agent systems aligned to Etsy buyer demand (ranks 1–20). Each kit includes **playbook + 10 workflows + setup** unless noted.

## Build

```bash
cd ai-agent-team
npm run agents:generate   # materialize products/agent-01-… through agent-20-…
npm run products:build    # ZIPs in dist/
```

| Bundle | ZIP |
|--------|-----|
| All 20 | `dist/top-20-ai-agents-bundle.zip` |
| Agents 1–10 only | `dist/top-10-ai-agents-bundle.zip` |
| Single agent | `dist/agent-NN-….zip` |

## The 20 agents

| # | Agent | Folder | Bundled assets |
|---|--------|--------|----------------|
| 1 | Etsy Listing SEO | `agent-01-etsy-listing-seo-agent-kit` | — |
| 2 | Social Content Machine | `agent-02-social-content-machine-agent-kit` | — |
| 3 | Marketing Planner | `agent-03-marketing-planner-agent-kit` | Marketing Command Center `.xlsx` |
| 4 | Client Onboarding | `agent-04-client-onboarding-agent-kit` | Full SOP + 12 workflows + tracker |
| 5 | Email Sequence | `agent-05-email-sequence-agent-kit` | — |
| 6 | Ad Copy | `agent-06-ad-copy-agent-kit` | — |
| 7 | Sales Proposal & Discovery | `agent-07-sales-proposal-discovery-agent-kit` | — |
| 8 | Copy Swipe File | `agent-08-copy-swipe-agent-kit` | 50 copy templates |
| 9 | POD Design Prompt | `agent-09-pod-design-prompt-agent-kit` | — |
| 10 | Listing Mockup & Photo Brief | `agent-10-listing-mockup-photo-brief-agent-kit` | — |
| 11 | Customer Support Reply | `agent-11-customer-support-reply-agent-kit` | — |
| 12 | SOP & Operations Doc | `agent-12-sop-operations-doc-agent-kit` | — |
| 13 | Lead Magnet & Opt-in | `agent-13-lead-magnet-opt-in-agent-kit` | — |
| 14 | Direct Mail Campaign | `agent-14-direct-mail-campaign-agent-kit` | Postcard pack templates |
| 15 | Brand Voice & Messaging | `agent-15-brand-voice-messaging-agent-kit` | — |
| 16 | Blog & SEO Article | `agent-16-blog-seo-article-agent-kit` | — |
| 17 | Business-in-a-Box Orchestrator | `agent-17-business-in-a-box-orchestrator-agent-kit` | Bundle readme |
| 18 | AI Ops & Admin | `agent-18-ai-ops-admin-agent-kit` | 50 AI ops workflows |
| 19 | Etsy Shop Analytics | `agent-19-etsy-shop-analytics-agent-kit` | — |
| 20 | Coach & Consultant Professional | `agent-20-coach-consultant-professional-agent-kit` | — |

## Source of truth for copy edits

| Agents | Edit file |
|--------|-----------|
| 1–10 | `scripts/agents/top10-definitions.mjs` |
| 11, 20 (full) + 12 base | `scripts/agents/top20-agents-11-20.mjs` + legacy `top20-agents-11-20.mjs` |
| 13–19 | `scripts/agents/market-top20-11-20.mjs` |

After edits: `npm run agents:generate` then `npm run products:build`.

**Etsy SEO:** titles, 13 tags, and openers for all 20 agents in [ETSY-KEYWORD-PASS.md](ETSY-KEYWORD-PASS.md).

**v2 excellence pack** (examples, scorecards, artifacts): [PRODUCT-EXCELLENCE-V2.md](PRODUCT-EXCELLENCE-V2.md) — version **2.0.0** in each kit.

## Legacy note

`docs/TOP-10-AI-AGENTS.md` describes the first tranche; this document is the full **20-agent** catalog.
