# Kit Launch & Promo — AutoGPT block graph

Build in Agent Builder at `http://localhost:3000`. Read paths as **container** paths from `docker-compose.volumes.example.yml`.

## 1. Trigger

- **File / webhook**: `POST /webhooks/kit-drop` or watch `/workspace/dlz/releases/kit_manifest.json`
- Inputs: `manifest_path`, `zip_path` (optional)

## 2. Research & context

- **Read file**: `/workspace/dlz/releases/kit_manifest.json` (schema: `docs/factory/kit_manifest.schema.json`)
- **Read file**: `/workspace/dlz/docs/market-research/latest_snapshot.json`
- **Firecrawl scrape** (optional): top 2 competitor URLs from snapshot — public pages only

## 3. Copy & assets (LLM — Claude Sonnet / GPT-4o)

System context files (read-only mounts):

- `/workspace/dlz/readonly/LICENSING.md`
- Superpowers brand rules from operator vault (paste or MCP)

Outputs (write under `/workspace/dlz/docs/factory/staged/`):

- `storefront.md` — headline, bullets, DAW list
- `shorts-scripts.md` — 3× FL cook-up scripts (808 + Triggerman rolls)
- `announce-email.md` — existing customer list (draft only)

Use **gpt-4o-mini** for JSON field extraction; frontier model only for this block.

## 4. Human approval → publish

- **Telegram / Discord** block: send staged markdown summary + approve/reject buttons
- On approve: **HTTP** to storefront API (Shopify/Gumroad/Lemon) — **draft product only**
- On reject: write `staged/REJECTED.txt` and stop (no retry loop)

## Limits

| Setting | Value |
|---------|--------|
| max_retries | 2 |
| max_iterations | 8 |
| C++ / shell compile | disabled |
| Audio binary read | metadata JSON only |