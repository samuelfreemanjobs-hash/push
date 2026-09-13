# Etsy store automation — build-ready plan (issue #5)

Branch: `cursor/etsy-store-automation-f219`

## Goal

Automate research → listing copy → Canva art pipeline → n8n daily stub for the agent-kit product catalog.

## Epics

### EP1 — Merge & runtime

- Resolve merge conflicts with `cursor/pm-system-autonomy-c0f8` (README, `src/index.js`, `package-lock.json`)
- Preserve Manuscript Master routes under `/api/manuscript-master`
- Add Etsy scripts from branch: `agents:generate`, `products:build`, `products:canva-autofill`

**Done when:** `npm install && npm start` serves `/health`, `/api/pm/backlog`, manuscript routes.

### EP2 — Documentation

- Ship `docs/ETSY-AUTOMATION.md`, `docs/CANVA-AUTOMATION.md` from branch
- Link from README

### EP3 — Human gates

- Canva MCP connect (local Cursor)
- `CANVA_ACCESS_TOKEN` for `npm run products:canva:submit --submit`
- Etsy API credentials for live publish

## Acceptance criteria

1. Product ZIPs build via `npm run products:build`
2. n8n workflow JSON present under `integrations/n8n/`
3. PM backlog marks push-5 `status: done` after merge + smoke test
