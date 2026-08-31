# AI Agent Team

A powerful AI agent application powered by Google Gemini and Supabase.

## Features

- 🤖 AI-powered responses using Google Gemini
- 💾 Conversation history with Supabase
- 🔄 RESTful API
- 🚀 Production-ready

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

