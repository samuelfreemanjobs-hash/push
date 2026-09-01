# n8n integration — Phase 1 (MedFlow / Push runtime)

n8n is the **automation bus**. The **push runtime** (this repo) is the **brain** — it reads Supabase CRM data, generates EA dashboards, and posts to Slack.

```
Weekday 7 AM ──► n8n cron ──► POST /api/webhooks/n8n/ea-daily ──► Slack (#executive-assistant)
                                    │
                                    └── reads Supabase (tasks, gates, pipeline)
                                    └── logs dashboard_runs

/ea dashboard ──► Slack slash ──► n8n webhook ──► POST /api/webhooks/n8n/slack-ea ──► response
```

## Railway services (2)

| Service | Image / root | Port |
|---------|----------------|------|
| `push-runtime` | `ai-agent-team/` (Node) | 3000 |
| `n8n` | `n8n/Dockerfile` | 5678 |

## Environment variables

### push-runtime (Railway)

| Variable | Required | Description |
|----------|----------|-------------|
| `SUPABASE_URL` | yes | Supabase project URL |
| `SUPABASE_SERVICE_KEY` | yes | Service role key (CRM writes) |
| `GEMINI_API_KEY` | optional | Powers `/ea plan today` |
| `SLACK_WEBHOOK_URL` | yes* | Incoming webhook → `#executive-assistant` |
| `N8N_WEBHOOK_SECRET` | yes | Shared secret for n8n → runtime auth |
| `PRIMARY_PRODUCT` | no | Default `medflow` (shown on `/health`) |
| `PORT` | no | Default 3000 |

\*Required for automated daily posts. Manual API calls work without it.

### n8n (Railway)

| Variable | Required | Description |
|----------|----------|-------------|
| `RAILWAY_API_URL` | yes | Public URL of push-runtime (no trailing slash) |
| `N8N_WEBHOOK_SECRET` | yes | Same value as push-runtime |
| `N8N_HOST` | yes | Public n8n hostname (e.g. `n8n-xxx.up.railway.app`) |
| `N8N_PROTOCOL` | yes | `https` |
| `WEBHOOK_URL` | yes | `https://<n8n-host>/` |
| `GENERIC_TIMEZONE` | no | `America/New_York` |
| `SLACK_EA_CHANNEL` | no | `#executive-assistant` |

## Setup order

1. **Supabase** — run `supabase/migrations/001_crm_schema.sql`
2. **Deploy push-runtime** on Railway → copy public URL
3. **Deploy n8n** on Railway (see `n8n/Dockerfile`)
4. **Set env vars** on both services (shared `N8N_WEBHOOK_SECRET`)
5. **Import workflows** (n8n UI → Workflows → Import from file):
   - `ea-daily-dashboard.workflow.json`
   - `ea-slack-commands.workflow.json`
   - `cloud-architect-audit.workflow.json`
6. **Activate** all three workflows
7. **Slack** — import `integrations/slack/app-manifest.yaml` (replace `YOUR_N8N_HOST`)
8. **Test**:
   ```bash
   curl -X POST "$RAILWAY_API_URL/api/webhooks/n8n/ea-daily" \
     -H "x-n8n-secret: $N8N_WEBHOOK_SECRET" \
     -H "Content-Type: application/json" \
     -d '{}'
   ```

## Slack slash commands

Point each command's Request URL to the **ea-slack-commands** n8n webhook URL:

| Command | Maps to |
|---------|---------|
| `/ea` | dashboard (default) |
| `/ea dashboard` | daily dashboard |
| `/ea plan today` | AI plan |
| `/ea followups` | overdue follow-ups |
| `/ea weekly review` | weekly rollup |

## Editing daily output

Edit `config/ea-dashboard.yaml` — no code deploy needed for copy/focus blocks.

Edit CRM data in Supabase tables: `operator_gates`, `tasks`, `follow_ups`, `opportunities`, `leads`.

## Phase 2 hooks

- Product-specific webhooks: `/api/webhooks/n8n/medflow-*`
- Linear notifications on gate completion
- Cloud audit: Supabase advisors + Railway service inventory
