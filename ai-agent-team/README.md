# AI Agent Team

A powerful AI agent application powered by Google Gemini and Supabase.

## Features

- 🤖 AI-powered responses using Google Gemini
- 📚 **Manuscript Master** — autonomous KDP book architect with 14 prompt modules
- 💾 Conversation history with Supabase
- 🔄 RESTful API
- 🚀 Production-ready

## Manuscript Master

Autonomous bestselling business book architect for Amazon KDP. Includes 14 enterprise-grade prompt modules, workflow orchestration, state compaction, and adversarial quality auditing.

See `skills/manuscript-master/SKILL.md` for the full operational playbook.

### Manuscript Master Endpoints

```bash
GET  /api/manuscript-master/modules          # List all 14 prompt modules
GET  /api/manuscript-master/playbook         # Get step-by-step workflow
POST /api/manuscript-master/sessions         # Create workflow session
GET  /api/manuscript-master/sessions/:id     # Get session status & state
POST /api/manuscript-master/sessions/:id/next # Execute next playbook step
POST /api/manuscript-master/execute          # Run a specific module
POST /api/manuscript-master/preview          # Preview prompt without AI call
```

### Quick Start — Book Concept Generation

```bash
# Create session
curl -X POST http://localhost:3000/api/manuscript-master/sessions \
  -H "Content-Type: application/json" \
  -d '{"niche_expertise":"B2B SaaS sales","target_audience":"VP Sales at $5M-$50M ARR companies","monetization_goal":"Consulting Pipeline"}'

# Execute concept generator (Module 01)
curl -X POST http://localhost:3000/api/manuscript-master/execute \
  -H "Content-Type: application/json" \
  -d '{"moduleId":"01_concept_generator","variables":{"niche_expertise":"B2B SaaS sales","primary_audience":"VP Sales","monetization_goal":"Consulting Pipeline"},"sessionId":"<sessionId>"}'
```

## MCP servers (Canva, Etsy, Linear, Gemini)

Custom stdio MCP servers for **Canva Connect**, **Etsy Open API v3**, **Linear GraphQL**, and **Google Gemini**. Setup: `docs/MCP-SETUP.md`.

## PM Product Matrix Operations

Autonomous backlog and launch queue for this monorepo. Routing manifests: `LINEAR`, `CURSOR`, `CLAUDE`.

- `pm/backlog.yaml` — P0–P2 queue synced to GitHub issues
- `pm/product-matrix.md` — portfolio scoring
- Skill: `skills/pm-agent/SKILL.md`

```bash
GET /api/pm/backlog   # JSON backlog
GET /api/pm/matrix    # product matrix markdown
npm run pm:sync       # refresh titles/state from GitHub (requires gh)
```

## Endpoints

### Health Check
```bash
GET /health
```

### Send Message to AI Agent
```bash
POST /api/agent
Content-Type: application/json

{
  "message": "Your question here",
  "userId": "user123"
}
```

### Get Conversation History
```bash
GET /api/conversations/:userId
```

## Environment Variables

- `GEMINI_API_KEY` - Google Gemini API key
- `SUPABASE_URL` - Supabase project URL
- `SUPABASE_KEY` - Supabase public API key
- `PORT` - Server port (default: 3000)
- `NODE_ENV` - Environment (production/development)

## Deployment on Railway

1. Connect your GitHub repo
2. Set root directory to `ai-agent-team`
3. Add environment variables from Railway dashboard
4. Deploy!

## Local Development

```bash
npm install
npm run dev
```

Visit `http://localhost:3000/health` to verify the server is running.

