# Etsy store automation

This runtime automates the **research → product → listing → SEO → social → (optional) Etsy draft** loop. It does **not** replace Etsy’s seller policies, tax compliance, or fulfillment for physical goods—you still own those.

## What runs automatically

| Stage | Agent / module | Output |
|--------|----------------|--------|
| Market research | `research` | Niche, competition, pricing bands |
| Product design | `etsy-product` | JSON product plan (digital/POD/physical) |
| Listing creation | `etsy-listing` | Title, 13 tags, description, image prompts |
| Etsy SEO | `seo` | Title/tag refinements |
| Marketing | `social` | Pinterest / IG / TikTok calendar |
| Sales insights | `analytics` | KPIs when shop data is available |
| Publish (optional) | `etsy/publish` | **Draft** listing via Etsy Open API v3 |

**Still manual or partner-automated today (by design):**

- OAuth connect (one-time seller login)
- Uploading listing images and digital files (Etsy upload APIs or UI)
- Print-on-demand SKU sync (connect Printful/Printify via Zapier or their API)
- Etsy Ads budget and billing
- Customer support for edge cases

## One-time setup

### 1. Etsy Open API app

1. Create an app at [Etsy Developers](https://www.etsy.com/developers/).
2. Note the **keystring** → `ETSY_API_KEY`.
3. Set redirect URL → `ETSY_REDIRECT_URI` (e.g. `https://your-runtime.up.railway.app/api/etsy/oauth/callback`).
4. Start OAuth: `GET /api/etsy/oauth/start` → open `authorizationUrl` → complete login.
5. Store `access_token` / `refresh_token` as `ETSY_ACCESS_TOKEN` and `ETSY_REFRESH_TOKEN`.
6. Set `ETSY_SHOP_ID` and a default `ETSY_DEFAULT_TAXONOMY_ID` (from Etsy taxonomy API or developer docs).

### 2. AI and database

| Variable | Purpose |
|----------|---------|
| `GEMINI_API_KEY` | All agents |
| `SUPABASE_URL` / `SUPABASE_KEY` | Optional run history (`003_etsy_pipeline.sql`) |
| `ETSY_AUTOMATION_SECRET` | Protects cron/webhook endpoints |

### 3. Store config

```bash
cp config/etsy-store.yaml.example config/etsy-store.yaml
# edit niche, product_type, channels
```

Or set `ETSY_NICHE` and `ETSY_PRODUCT_TYPE` for the CLI.

### 4. Schedule (n8n or cron)

- Import `integrations/n8n/etsy-daily-pipeline.workflow.json`.
- Set `RAILWAY_API_URL`, `N8N_WEBHOOK_SECRET`, optional `SLACK_WEBHOOK_URL`.
- Or cron: `node scripts/etsy-daily-run.js` (add `--publish` only when drafts should be created).

## API

```bash
# Health + capability flags
curl "$BASE/api/etsy/status"

# Full pipeline (requires x-etsy-automation-secret if configured)
curl -X POST "$BASE/api/etsy/pipeline/run" \
  -H "Content-Type: application/json" \
  -H "x-etsy-automation-secret: $ETSY_AUTOMATION_SECRET" \
  -d '{"niche":"boho nursery wall art printables","productType":"digital","publish":false}'

# Shop snapshot + AI recommendations (needs Etsy tokens)
curl "$BASE/api/etsy/shop/snapshot" \
  -H "x-etsy-automation-secret: $ETSY_AUTOMATION_SECRET"
```

## Zapier / Slack

- **Zapier**: Connect Etsy triggers (new order, new message) to Slack/email; use this runtime for daily listing generation via Webhooks.
- **Slack** (native MCP): Post pipeline summaries to a channel after each n8n run.

## Compliance checklist

- No trademark / copyright violations in generated copy
- Accurate “who made” and “what is it” fields
- Digital files delivered per Etsy instant-download rules
- Refresh OAuth tokens before expiry
