# Canva & Etsy MCP servers

This repo includes **custom MCP servers** for automation that the built-in Cursor Canva plugin does not cover (Connect API autofill/export) and for **Etsy Open API v3** (listings, OAuth).

## Two Canva options

| Option | Best for | Setup |
|--------|----------|--------|
| **Official Cursor Canva MCP** | Chat-driven designs in Canva UI | Cursor → **Settings → Tools & MCP → Canva → Connect** |
| **`@push/canva-mcp` (this repo)** | Brand-template autofill, PNG/PDF export via [Canva Connect API](https://www.canva.dev/docs/connect/) | Env token + `mcp.json` below |

Use both if you want conversational design **and** scripted bulk listing art.

## Install

```bash
cd ai-agent-team/mcp
npm install
```

## Cursor `mcp.json`

Copy into your user or project MCP config (adjust paths if your clone lives elsewhere):

```json
{
  "mcpServers": {
    "push-canva-connect": {
      "command": "node",
      "args": ["/absolute/path/to/push/ai-agent-team/mcp/canva-server/index.js"],
      "env": {
        "CANVA_ACCESS_TOKEN": "your_oauth_access_token"
      }
    },
    "push-etsy": {
      "command": "node",
      "args": ["/absolute/path/to/push/ai-agent-team/mcp/etsy-server/index.js"],
      "env": {
        "ETSY_API_KEY": "your_keystring",
        "ETSY_ACCESS_TOKEN": "optional_until_oauth_done",
        "ETSY_REFRESH_TOKEN": "optional",
        "ETSY_SHOP_ID": "your_shop_id",
        "ETSY_REDIRECT_URI": "https://your-app.example/oauth/callback"
      }
    }
  }
}
```

Example file: `ai-agent-team/.cursor/mcp.json.example`

Restart Cursor after editing MCP config.

## Environment variables

See `ai-agent-team/.env.example` for the full list.

### Canva Connect

1. [Canva Developers](https://www.canva.com/developers/) → create integration.
2. Scopes: `design:content:write`, `brandtemplate:meta:read`, export as needed.
3. Complete OAuth; set `CANVA_ACCESS_TOKEN`.

### Etsy Open API v3

1. [Etsy Developers](https://www.etsy.com/developers/) → create app → `ETSY_API_KEY` (keystring).
2. Set redirect URL → `ETSY_REDIRECT_URI`.
3. In Cursor, call MCP tool **`etsy_build_oauth_url`**, open the URL, approve.
4. Call **`etsy_exchange_oauth_code`** with `code` + `code_verifier`; save tokens to env.
5. Set `ETSY_SHOP_ID`.

## Tools exposed

### Canva (`push-canva-connect`)

- `canva_config_status`
- `canva_create_autofill` / `canva_get_autofill`
- `canva_export_design`

### Etsy (`push-etsy`)

- `etsy_config_status`
- `etsy_build_oauth_url` / `etsy_exchange_oauth_code` / `etsy_refresh_token`
- `etsy_get_shop` / `etsy_list_active_listings`
- `etsy_create_draft_listing` / `etsy_update_listing` (writes — confirm with user)

## Cloud Agents

OAuth browser flows for Etsy and Canva usually must be completed **on your machine** (or your deployed OAuth callback). Cloud agents can use the servers once tokens are in the environment MCP config.

## Verify locally

```bash
cd ai-agent-team/mcp
npx @modelcontextprotocol/inspector node canva-server/index.js
npx @modelcontextprotocol/inspector node etsy-server/index.js
```
