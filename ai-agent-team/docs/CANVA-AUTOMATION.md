# Canva automation

Three ways to go from product data → listing art, from manual to fully API-driven.

| Level | What | Command |
|-------|------|---------|
| **1** | Markdown prompts (paste in Magic Media) | `npm run products:canva-prompts` |
| **2** | **Bulk Create CSV** (import in Canva) | `npm run products:canva-autofill` → `dist/canva-bulk/bulk-create.csv` |
| **3** | **Canva Connect API** (autofill brand templates + export PNG) | Configure JSON + `npm run products:canva:submit --submit --export` |
| **4** | **Cursor Canva MCP** | Settings → Tools & MCP → Canva → Connect (chat-driven designs) |
| **5** | **n8n** | `integrations/n8n/canva-product-art.workflow.json` |

---

## Level 2 — Bulk Create (no API)

1. In Canva, create a **Brand Template** for Etsy (2000×2000) with text boxes named to match CSV columns: `Headline`, `Subhead`, `Bullet1`, etc. (see [example field map](../config/canva-brand-templates.example.json)).
2. Run:

```bash
cd ai-agent-team
npm run products:canva-autofill
```

3. Open Canva **Apps → Bulk create** (or Data merge) and upload `dist/canva-bulk/bulk-create.csv`.
4. Map columns to template fields; generate one design per row (28 products).

Image backgrounds: use `prompt_etsy_hero` etc. columns as reference — either run Magic Media manually once per style, or use API level 3.

---

## Level 3 — Canva Connect API

### Prerequisites

1. [Canva Developers](https://www.canva.com/developers/) → create integration.
2. Enable scopes: **design:content:write**, **design:meta:read**, **brandtemplate:meta:read** (and export if available on your plan).
3. OAuth: obtain a **user access token** with refresh (store in `CANVA_ACCESS_TOKEN` for CLI, or n8n credential).

### Brand templates

1. In Canva, design one template per asset type (`etsy_hero`, `etsy_inside`, …).
2. Name text elements exactly as in `config/canva-brand-templates.example.json` (e.g. `Headline`, `Subhead`).
3. Publish as **Brand template** and copy each template ID into config:

```bash
cp config/canva-brand-templates.example.json config/canva-brand-templates.json
# edit brand_template_id values — do not commit secrets
```

Add `config/canva-brand-templates.json` to `.gitignore` if it contains real IDs (optional).

### Generate job payloads

```bash
npm run products:canva-autofill
```

Outputs:

- `dist/canva-autofill/manifest.json` — index
- `dist/canva-autofill/jobs/<slug>/<asset>.json` — one autofill job each

### Dry run (no API calls)

```bash
npm run products:canva:submit
```

### Submit to Canva + export PNG

```bash
export CANVA_ACCESS_TOKEN="your_oauth_access_token"
npm run products:canva:submit -- --submit --export --slug agent-01-etsy-listing-seo-agent-kit --asset etsy_hero
```

Results: `dist/canva-exports/submit-results.json` with `designId` and download URLs when export succeeds.

**Rate limits:** Use `--limit 5` for testing. Full catalog = 28 × 7 = 196 jobs — batch overnight.

### API response shape

Canva may return `job.status` of `in_progress` until `success`. The CLI polls every 2s. If your account uses different field names, adjust `fields` mapping in `canva-brand-templates.json` only — job JSON stays the same.

---

## Level 4 — Canva MCP in Cursor

Connect the **Canva** MCP server in Cursor (same account as your shop). After auth, you can ask the agent to create or edit designs using natural language. This cloud agent environment cannot complete OAuth for you; use local Cursor for MCP.

Pair MCP with repo data:

```bash
npm run products:canva-autofill
# Then in chat: "Using jobs/agent-01-.../etsy_hero.json, create the Etsy hero in Canva"
```

---

## Level 5 — n8n workflow

Import `integrations/n8n/canva-product-art.workflow.json`:

1. **Schedule** or manual trigger after `products:build`.
2. **Execute Command** node: `npm run products:canva-autofill` (on your host).
3. **HTTP Request** nodes: POST autofill / poll / export using `CANVA_ACCESS_TOKEN` from n8n credentials.
4. **Write files** to S3/Drive or attach to Etsy draft listing API (separate Etsy OAuth).

---

## Regenerate everything

```bash
npm run products:canva-prompts    # docs/canva-prompts/*.md
npm run products:canva-autofill   # dist/canva-bulk + dist/canva-autofill
```

After changing copy in `scripts/agents/canva-art-data.mjs`, run both.

---

## What Canva cannot automate (yet)

- **Magic Media** image generation from prompts is not exposed on Connect API the same way as the UI; backgrounds are usually **fixed in the brand template** or uploaded assets.
- **Etsy upload** is separate (your Etsy automation pipeline / manual).
- **OAuth** must be completed on your machine or in n8n — not in unsigned cloud runs.

---

## Related

- [CANVA-ART-PROMPTS.md](./CANVA-ART-PROMPTS.md) — human-readable prompts
- [ETSY-AUTOMATION.md](./ETSY-AUTOMATION.md) — listing pipeline
