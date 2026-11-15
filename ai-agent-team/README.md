# AI Agent Team

Free-tier **elite agent roster** for DiskLordz — mobile coding when Cursor/Grok are down.

## Features

- Prefix routing (`CODE:`, `SAGE:`, `REVIEW:`, …) — same as Slack vault
- **CODE** default: Gemini Flash (free) or Ollama `qwen2.5-coder`
- Optional Supabase conversation log
- Pairs with Command Center **Mobile crew** PWA tab

## Endpoints

### Health Check
```bash
GET /health
```

### List elite agents
```bash
GET /api/agents
```

### Send Message to AI Agent
```bash
POST /api/agent
Content-Type: application/json

{
  "message": "CODE: fix the vite PWA plugin config",
  "userId": "user123",
  "agentId": "code"
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

