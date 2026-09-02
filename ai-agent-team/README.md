# AI Agent Team — Marketing Team Framework

Multi-agent marketing team powered by Claude (Anthropic) + Gemini + Supabase.

## Architecture

```
Marketing Orchestrator (Claude Haiku — cheap router)
├── Research Agent     (Haiku — market research, audience insights)
├── Content Agent      (Sonnet 5 — blog posts, email, ad copy)
├── SEO Agent          (Haiku — keywords, meta tags, optimization)
├── Social Agent       (Sonnet 5 — LinkedIn, Twitter, Instagram, Facebook)
└── Analytics Agent    (Haiku — metrics interpretation, recommendations)
```

**Token efficiency:**
- System prompts are cached with `cache_control: ephemeral` — saves ~90% on repeated agent calls
- Haiku handles routing and structured tasks; Sonnet handles creative work
- Each agent only sees context it needs — no full conversation history
- Per-request token budget prevents runaway costs

## Endpoints

### Full Marketing Team
```bash
POST /api/marketing/task
{
  "task": "Write a blog post about our new project management tool for remote teams",
  "userId": "user123",
  "tokenBudget": 8000
}
```
The orchestrator decides which agents to activate. You pay only for what runs.

### Single Agent (direct)
```bash
POST /api/marketing/agent/content
{ "task": "Write a cold email for our SaaS product", "context": "optional research context" }

POST /api/marketing/agent/social
{ "task": "Create LinkedIn and Twitter posts for our product launch" }

POST /api/marketing/agent/seo
{ "task": "Optimize our landing page for 'project management software'" }

POST /api/marketing/agent/research
{ "task": "Analyze the market for B2B project management tools" }

POST /api/marketing/agent/analytics
{ "task": "Our email open rate dropped from 28% to 19% last week, here are the numbers..." }
```

### List Agents
```bash
GET /api/marketing/agents
```

### Original Gemini Agent (unchanged)
```bash
POST /api/agent
GET /api/conversations/:userId
GET /health
```

## Response Shape

```json
{
  "success": true,
  "plan": "Research the market, then write targeted content",
  "agentsUsed": ["research", "content", "social"],
  "results": {
    "research": "...",
    "content": "...",
    "social": "..."
  },
  "tokenUsage": {
    "maxTokens": 8000,
    "usedTokens": 2340,
    "remainingTokens": 5660,
    "agents": [...]
  }
}
```

## Environment Variables

| Variable | Required | Description |
|----------|----------|-------------|
| `ANTHROPIC_API_KEY` | Yes | Claude API key (marketing team) |
| `GEMINI_API_KEY` | Yes | Google Gemini key (original agent) |
| `SUPABASE_URL` | Yes | Supabase project URL |
| `SUPABASE_KEY` | Yes | Supabase public API key |
| `PORT` | No | Server port (default: 3000) |

## Supabase Tables

The marketing team logs to `marketing_tasks`:
```sql
create table marketing_tasks (
  id uuid default gen_random_uuid() primary key,
  user_id text,
  task text,
  routing text[],
  results jsonb,
  token_usage jsonb,
  created_at timestamptz default now()
);
```

## Adding Your Own Agents

1. Create `src/agents/myagent.js` — export `async function runMyAgent(task, budget)`
2. Import and add it to the `AGENT_MAP` in `src/routes/marketing.js`
3. Add a case to the `switch` in the direct agent endpoint
4. Register it in the `/api/marketing/agents` listing

## Local Development

```bash
npm install
npm run dev
```
