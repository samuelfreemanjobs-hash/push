# Product matrix — push / ai-agent-team

Launch priority queue for autonomous agents. P0 ships revenue; P1 enables distribution; P2 expands catalog.

| Priority | Product line | Repo surface | Buyer / user | Autonomous build status | Human gate |
|----------|--------------|--------------|--------------|-------------------------|------------|
| P0 | Etsy agent kits + store automation | `cursor/etsy-store-automation-f219` | Digital sellers on Etsy | Branch ready; merge blocked on 3 files | Etsy OAuth, Canva MCP |
| P1 | Manuscript Master (KDP) | `src/manuscript-master/`, `manuscripts/` | Authors, course creators | **Merged** — API + full draft manuscript | KDP publish account |
| P1 | AI Agent Team API | `src/index.js` | Internal / Railway deploy | Healthy baseline + Gemini | `GEMINI_API_KEY`, Supabase |
| P2 | MedFlow n8n + EA Slack | Issue #1 branch | Healthcare ops client | Not started in this repo | n8n host, Slack app |
| P2 | Micro SaaS factory | Issue #4 branch | Indie hackers | Not started | None |

## Scoring (RICE-style, relative)

| Initiative | Reach | Impact | Confidence | Effort | Score |
|------------|-------|--------|------------|--------|-------|
| Etsy automation | High | High | Medium | Large | **Top** |
| Manuscript Master | Medium | High | High | Done | **Shipped** |
| MedFlow runtime | Low | High | Medium | Medium | Queue |
| Micro SaaS scaffold | Medium | Medium | Medium | Medium | Queue |

## Weekly PM summary template

Post to `#pm-product-matrix`:

1. **Shipped** — PRs merged, tags, deploys
2. **In flight** — branches + % complete
3. **Blocked** — auth, secrets, owner actions
4. **Next autonomous batch** — what Cloud Agent should run next
