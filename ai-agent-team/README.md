# Push Runtime — MedFlow Automation

n8n-facing runtime for **MedFlow** and future agent products. Supabase is the CRM. Railway hosts both this API and n8n.

## Architecture

```
Slack (#executive-assistant)
    ↕ slash commands
n8n (Railway) — cron + webhooks
    ↕ x-n8n-secret
push runtime (this repo) — EA + agent APIs
    ↕
Supabase — CRM (tasks, gates, leads, pipeline)
```

## Quick start (local)

```bash
cd ai-agent-team
npm install
cp .env.example .env   # fill Supabase + secrets
npm run dev
```

Run Supabase migration: `supabase/migrations/001_crm_schema.sql`

## API endpoints

| Method | Path | Auth | Description |
|--------|------|------|-------------|
| GET | `/health` | — | Runtime health |
| GET | `/api/ea/dashboard` | — | Daily dashboard JSON |
| POST | `/api/ea/dashboard` | — | Build + optional Slack post |
| POST | `/api/ea/plan-today` | — | AI plan from dashboard context |
| POST | `/api/ea/followups` | — | Overdue follow-ups |
| POST | `/api/ea/weekly-review` | — | Weekly rollup |
| POST | `/api/webhooks/n8n/ea-daily` | `x-n8n-secret` | n8n cron → Slack daily post |
| POST | `/api/webhooks/n8n/slack-ea` | `x-n8n-secret` | n8n Slack command handler |
| POST | `/api/agent` | — | Gemini chat (legacy) |

## Deploy on Railway

### Service 1: push-runtime

1. Connect repo, set **root directory** to `ai-agent-team`
2. Add env vars from `.env.example`
3. Deploy → copy public URL → this is `RAILWAY_API_URL`

### Service 2: n8n

1. New service, same repo, set **root directory** to `ai-agent-team/n8n`
2. Set env: `RAILWAY_API_URL`, `N8N_WEBHOOK_SECRET`, `N8N_HOST`, `N8N_PROTOCOL=https`, `WEBHOOK_URL`
3. Import workflows from `integrations/n8n/`
4. Activate workflows

Full guide: `integrations/n8n/README.md`

## Config

- **Operating system:** `docs/MEDFLOW-OS.md` — channels, playbook, salvage map from Freeman Intelligence OS
- Daily dashboard copy: `config/ea-dashboard.yaml`
- CRM schema: `supabase/migrations/001_crm_schema.sql` + `002_medflow_playbook.sql`
- Slack app: `integrations/slack/app-manifest.yaml`

## Phase 1 complete checklist

- [ ] Supabase migration applied
- [ ] push-runtime deployed on Railway
- [ ] n8n deployed on Railway
- [ ] Workflows imported + active
- [ ] `SLACK_WEBHOOK_URL` set → test `POST /api/webhooks/n8n/ea-daily`
- [ ] Slack manifest imported → test `/ea dashboard`

After Phase 1: share your MedFlow business direction — we'll salvage useful pieces from Freeman Intelligence OS and design the next operating system.
