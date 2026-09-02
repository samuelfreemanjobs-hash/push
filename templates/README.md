# [PRODUCT NAME]

[One sentence description of what this does and who it's for.]

## Endpoints

### Health Check
```bash
GET /health
```

### [Main Endpoint]
```bash
POST /api/[endpoint]
Content-Type: application/json

{
  "field1": "your input here",
  "userId": "user123"
}
```

### [List Endpoint]
```bash
GET /api/[endpoint]/:userId
```

## Environment Variables

| Variable | Description |
|----------|-------------|
| `GEMINI_API_KEY` | Google Gemini API key |
| `SUPABASE_URL` | Supabase project URL |
| `SUPABASE_KEY` | Supabase anon key |
| `PORT` | Server port (default: 3000) |
| `NODE_ENV` | `production` or `development` |

## Deploy on Railway

1. Connect your GitHub repo
2. Set root directory to `[folder-name]`
3. Add all env vars from `.env.example` with real values
4. Deploy — Railway auto-detects Node.js

## Local Development

```bash
npm install
cp .env.example .env   # then fill in real values
npm run dev
```

Visit `http://localhost:3000/health` to verify the server is running.
