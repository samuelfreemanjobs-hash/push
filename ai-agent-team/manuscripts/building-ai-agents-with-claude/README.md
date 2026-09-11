# Building AI Agents with Claude

**Full manuscript** — publication-ready draft.

## Title

**Building AI Agents with Claude**  
*How to Build a Digital Workforce That Automates Grunt Work, Cuts Overhead, and Scales Your Business*

## Manuscript Contents

| # | File | Section |
|---|------|---------|
| — | `00-blueprint.md` | Book architecture schematic |
| — | `00-introduction.md` | Introduction (~1,450 words) |
| 1 | `01-chapter-end-of-overhead.md` | Act I: The Digital Shop Floor |
| 2 | `02-chapter-claude-advantage.md` | Act I: Reasoning, Context, Tools |
| 3 | `03-chapter-digital-worker-architecture.md` | Act I: TCAV & Zero-Drift Prompt Spec |
| 4 | `04-chapter-mcp-integrations.md` | Act II: MCP & Tool Integrations |
| 5 | `05-chapter-multi-agent-choreography.md` | Act II: Orchestrator-Workers & Swarms |
| 6 | `06-chapter-revenue-engine.md` | Act II: SDR & Pipeline Automation |
| 7 | `07-chapter-back-office-engine.md` | Act II: Document Reconciliation |
| 8 | `08-chapter-client-delivery.md` | Act II: Support & Account Management |
| 9 | `09-chapter-guardrail-protocol.md` | Act III: Circuit Breakers & HITL |
| 10 | `10-chapter-synthetic-workforce.md` | Act III: Executive Operator |
| 11 | `11-chapter-30-day-roadmap.md` | Act III: 30-Day Deployment Sprint |
| A–D | `appendix-implementation-toolkit.md` | Production artifacts |
| — | `kdp-blurb.md` | Amazon product page copy (2 variants) |
| — | `author-bio.md` | Author bio (3 lengths) |
| — | `state.json` | Manuscript Master session state |

## Word Count

Run: `wc -w *.md | tail -1`

Target: 38,000–42,000 words (introduction + 11 chapters + appendix)

## KDP Keywords

- Building AI Agents
- Claude AI
- Digital Workforce
- Automate Business Operations
- No-Code AI Agents
- AI Automation Systems

## Core Frameworks

- **TCAV Protocol:** Trigger → Context → Action → Verification
- **Digital Shop Floor:** Agents as deterministic processing stations
- **Zero-Drift Prompt Spec:** XML-zoned system directives
- **Circuit Breaker Pattern:** Cost and loop containment
- **Evaluator-Optimizer Loop:** Quality gates before delivery

## Status

✅ Manuscript complete — ready for developmental edit, formatting, and KDP upload.

## Review PDF

Generate a letter-size review PDF (introduction + all chapters + appendix):

```bash
cd ai-agent-team/manuscripts
npm install marked puppeteer --no-save
node build-manuscript-pdf.mjs
```

Output: `manuscripts/output/Building-AI-Agents-with-Claude-Review-Draft.pdf`
