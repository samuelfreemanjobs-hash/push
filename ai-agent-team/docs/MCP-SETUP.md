# Canva, Etsy & Linear MCP servers

Custom **stdio MCP servers** in this repo for APIs that need scripted control (Canva Connect, Etsy listings, Linear issues). They complement any official Cursor MCP plugins.

## Canva — two options

| Option | Best for | Setup |
|--------|----------|--------|
| **Official Cursor Canva MCP** | Chat-driven designs in Canva UI | Cursor → **Settings → Tools & MCP → Canva → Connect** |
| **`push-canva-connect` (this repo)** | Brand-template autofill, PNG/PDF export | `CANVA_ACCESS_TOKEN` + `mcp.json` |

## Linear — two options

| Option | Best for | Setup |
|--------|----------|--------|
| **Official Linear MCP** (if listed in Cursor) | OAuth in UI | Settings → Tools & MCP → **Linear** → Connect |
| **`push-linear` (this repo)** | PM backlog sync, issue CRUD, GraphQL escape hatch | `LINEAR_API_KEY` + `mcp.json` |

For PM work (`pm/backlog.yaml`, project **PM Product Matrix Operations**), `push-linear` is enough with a [personal API key](https://linear.app/settings/account/security).

## Install

```bash
cd ai-agent-team/mcp
npm install
```

## Cursor `mcp.json`

Copy from `ai-agent-team/.cursor/mcp.json.example` (includes all three servers). Restart Cursor after saving.

## Environment variables

See `ai-agent-team/.env.example`.

### Linear

1. Linear → **Settings → Account → Security & access** → **Personal API keys** → create key (`lin_api_…`).
2. Set `LINEAR_API_KEY` in MCP `env` (no `Bearer` prefix).
3. Optional: `LINEAR_DEFAULT_TEAM_ID` for `linear_create_issue` without `team_id`.
4. OAuth alternative: set `LINEAR_ACCESS_TOKEN` with `Bearer` handled by the server.

### Canva Connect

1. [Canva Developers](https://www.canva.com/developers/) → OAuth → `CANVA_ACCESS_TOKEN`.

### Etsy Open API v3

1. [Etsy Developers](https://www.etsy.com/developers/) → `ETSY_API_KEY`, OAuth via `etsy_build_oauth_url` tools.

## Tools exposed

### Linear (`push-linear`)

- `linear_config_status` / `linear_viewer`
- `linear_list_teams` / `linear_list_projects` / `linear_find_project`
- `linear_list_issues` / `linear_get_issue`
- `linear_create_issue` / `linear_update_issue` / `linear_create_comment` (writes — confirm with user)
- `linear_graphql` (advanced)

### Canva (`push-canva-connect`)

- `canva_config_status`, `canva_create_autofill`, `canva_get_autofill`, `canva_export_design`

### Etsy (`push-etsy`)

- OAuth helpers, shop/listings, draft create/update

## Cloud Agents

Put API keys/tokens in the MCP server `env` block in your environment or Cursor MCP config. Personal Linear keys work well for automation; do not commit them to git.

## Verify locally

```bash
cd ai-agent-team/mcp
npx @modelcontextprotocol/inspector node linear-server/index.js
npx @modelcontextprotocol/inspector node canva-server/index.js
npx @modelcontextprotocol/inspector node etsy-server/index.js
```
