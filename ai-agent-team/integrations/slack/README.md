# Slack setup — Phase 1

1. Go to [api.slack.com/apps](https://api.slack.com/apps) → **Create New App** → **From manifest**
2. Select your workspace
3. Paste `app-manifest.yaml` — replace `YOUR_N8N_HOST` with your Railway n8n URL (no `https://` prefix in manifest URLs — use full `https://...` as shown)
4. Install app to workspace
5. Create channel `#executive-assistant` (or use existing)
6. **Incoming Webhook** (for daily auto-posts):
   - Slack app → Incoming Webhooks → On → Add to `#executive-assistant`
   - Copy URL → Railway env `SLACK_WEBHOOK_URL` on **push-runtime**
7. Test slash command: `/ea dashboard` in any channel where the app is installed

## Verify

```bash
# Direct runtime (bypass n8n)
curl "$RAILWAY_API_URL/api/ea/dashboard"

# Full n8n path
curl -X POST "$RAILWAY_API_URL/api/webhooks/n8n/ea-daily" \
  -H "x-n8n-secret: $N8N_WEBHOOK_SECRET"
```

Daily posts appear in `#executive-assistant` at 7 AM ET weekdays (after n8n cron is active).
