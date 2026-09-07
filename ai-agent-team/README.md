# AI Agent Team

A multi-agent AI application powered by Google Gemini and Supabase.

## Agents

### Manuscript Master

Autonomous bestselling business book architect and executive ghostwriter for Amazon KDP, Audible scripting, and high-ticket information products.

**Features:**
- 14 XML-zoned prompt modules (TCREI scaffold)
- 5-phase KDP production pipeline
- Per-chapter drafting loop with adversarial quality audit
- State compaction every 3 chapters
- Meta-prompt generator for bespoke modules
- 60% Rule enforcement (800–1,500 words per generation pass)

**Quick start:**

```bash
# List all modules and playbook
curl http://localhost:3000/api/manuscript/modules

# Run Module 01: Concept Generator
curl -X POST http://localhost:3000/api/manuscript/module/01 \
  -H "Content-Type: application/json" \
  -d '{
    "variables": {
      "niche_expertise": "B2B SaaS customer success",
      "primary_audience": "VP Customer Success at $5M-$50M ARR companies",
      "monetization_goal": "Consulting pipeline + course sales"
    }
  }'

# Run full chapter loop (06→04→03→07→09→14)
curl -X POST http://localhost:3000/api/manuscript/chapter \
  -H "Content-Type: application/json" \
  -d '{
    "chapterConfig": {
      "chapter_number": 1,
      "chapter_title": "The Retention Trap",
      "chapter_objectives": "Expose why NRR dashboards lie",
      "chapter_topic": "Net revenue retention blind spots",
      "chapter_mechanism": "The Churn Forensics Protocol",
      "target_audience": "VP Customer Success",
      "upcoming_chapter_theme": "Building the early warning system"
    }
  }'
```

### General AI Agent

Simple Gemini chat endpoint with optional Supabase conversation logging.

## Endpoints

### Health Check
```bash
GET /health
```

### Manuscript Master
| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/manuscript/modules` | List modules, phases, playbook |
| GET | `/api/manuscript/prompt/:id` | Get raw prompt template |
| GET | `/api/manuscript/system-directive` | Get system directive XML |
| POST | `/api/manuscript/module/:id` | Run a single module |
| POST | `/api/manuscript/chapter` | Run per-chapter drafting loop |
| POST | `/api/manuscript/audit` | Run module + adversarial audit |
| POST | `/api/manuscript/pipeline` | Run full 5-phase pipeline |
| POST | `/api/manuscript/meta-prompt` | Generate bespoke XML module |
| POST | `/api/manuscript/next-step` | Get recommended next step |

### General Agent
```bash
POST /api/agent
GET /api/conversations/:userId
```

## Environment Variables

- `GEMINI_API_KEY` — Google Gemini API key
- `GEMINI_MODEL` — Model name (default: `gemini-2.0-flash`)
- `SUPABASE_URL` — Supabase project URL
- `SUPABASE_KEY` — Supabase public API key
- `PORT` — Server port (default: 3000)
- `NODE_ENV` — Environment (production/development)

## Project Structure

```
ai-agent-team/
├── prompts/manuscript-master/
│   ├── system-directive.xml
│   ├── meta-prompt-generator.xml
│   └── modules/          # 14 XML prompt modules (01–14)
├── src/
│   ├── agents/manuscript/
│   │   ├── base.js           # Gemini client
│   │   ├── module-registry.js
│   │   ├── prompt-loader.js
│   │   ├── state.js
│   │   ├── module-runner.js
│   │   └── orchestrator.js
│   ├── routes/manuscript.js
│   └── index.js
└── CURSOR                    # Cursor rules for Manuscript Master
```

## Operational Playbook

1. **Module 01** — Generate 5 book concepts → select winner
2. **Module 08** — Test 15 title variants → lock title + keywords
3. **Module 02** — Build 10–14 chapter blueprint
4. **Module 05** — Draft introduction
5. **Chapter loop** (per chapter): 06 → 04 → 03 → 07 → 09 → 14
6. **Module 11** — Write conclusion with 90-day roadmap
7. **Modules 12 + 13** — KDP blurb and author bio

## Local Development

```bash
npm install
npm run dev
```

Visit `http://localhost:3000/health` to verify the server is running.

## Deployment

Set environment variables and deploy the `ai-agent-team` directory. Compatible with Railway, Render, or any Node.js host.
