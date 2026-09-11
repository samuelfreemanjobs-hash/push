# AI Agent Team

A powerful AI agent application powered by Google Gemini and Supabase.

## Features

- 🤖 AI-powered responses using Google Gemini
- 🛍️ **Etsy automation pipeline** — research → product → listing → SEO → social → optional draft publish
- 📣 Marketing agent team (`/api/marketing`)
- 💾 Conversation history with Supabase
- 🔄 RESTful API
- 🚀 Production-ready

See [docs/ETSY-AUTOMATION.md](docs/ETSY-AUTOMATION.md) for full Etsy setup (OAuth, n8n cron, compliance).

Shop config: `config/etsy-store.yaml`. Listings: [ETSY-LAUNCH-PACK.md](docs/ETSY-LAUNCH-PACK.md). SEO titles/tags: [ETSY-KEYWORD-PASS.md](docs/ETSY-KEYWORD-PASS.md). **Top 20 AI agent kits:** [TOP-20-AI-AGENTS.md](docs/TOP-20-AI-AGENTS.md) · `npm run agents:generate` · `npm run products:build`

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

### Run Etsy automation pipeline
```bash
POST /api/etsy/pipeline/run
Content-Type: application/json
x-etsy-automation-secret: <ETSY_AUTOMATION_SECRET>

{
  "niche": "digital wedding invitations",
  "productType": "digital",
  "publish": false
}
```

### Etsy automation status
```bash
GET /api/etsy/status
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

