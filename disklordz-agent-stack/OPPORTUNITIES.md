# DiskLordz opportunity stack (proposed applications)

These are **DiskLordz applications you build** — not products sold by vendors you research. Tech radar (AI Dev Signals, tools research) should answer: *which opportunity does this accelerate, and is that opportunity Immediate, Next, or Later?*

## Ranked opportunities

| Rank | Opportunity | Business benefit | Build priority |
|------|-------------|------------------|----------------|
| 1 | AI-assisted drum sample factory | Produce original drum kits faster | **Immediate** |
| 2 | Automated JUCE VST development system | Ship more reliable plugins | **Immediate** |
| 3 | Automated audio quality-control pipeline | Improve commercial sample consistency | **Immediate** |
| 4 | AI-powered sample tagging and search | Organize products and improve discovery | **Immediate** |
| 5 | Marketing and launch automation | Reduce repetitive business work | **Immediate** |
| 6 | AI-driven preset and sound design assistant | Differentiate synthesizers | Next |
| 7 | AI drum sample SaaS | Create recurring revenue | Next |
| 8 | Smart product recommendation engine | Increase cross-selling and bundles | Next |
| 9 | Automated plugin testing laboratory | Reduce bugs and support costs | Next |
| 10 | Natural-language drum kit designer | Offer a compelling SaaS interface | Next |
| 11 | AI customer support and documentation assistant | Improve post-purchase service | Later |
| 12 | AI market-intelligence monitoring | Find product gaps and opportunities | Later |

Machine-readable source: `lib/opportunities.mjs` (also mirrored to `hermes-out/research/opportunities.json` when you run `npm run tech-radar`).

## How agents use this

| Agent | Use |
|-------|-----|
| **RADAR** | Each screened signal gets `opportunity_ids: [rank-N id]`; prefer **adopt** when build priority is `immediate` and integration ≤ 3 |
| **SAGE** | Morning brief: at most one line per **immediate** opportunity touched by last radar report |
| **PLAN** | Kanban cards should cite `opportunity_id` when the work maps to the stack above |
| **DRUM / FORGE** | Ranks 1–3 are the factory north star |

## Opportunity #12 (market intelligence)

Implemented as the **Technology Intelligence** loop: AI Dev Signals intake → six-category screening → short reports. See [TECH-RADAR.md](./TECH-RADAR.md). Treat its build priority as **Later** for *net-new product surface*, but the **monitoring pipeline** is already on the free stack so Immediate-tier opportunities get better inputs now.