# DiskLordz Command Center

Private ops dashboard for solo DiskLordz business: kanban, tripwire launch path, vault PR queue, VST CI snapshot, and agent lane reference.

## Run locally

```bash
cd disklordz-command-center
npm install
npm run dev
```

Open http://127.0.0.1:5173

## Data & automation

- **Live snapshot:** `public/ops-snapshot.json` (vault kanban, vault PRs, Instruments workflow CI)
- **Refresh:** `npm run sync:ops` (uses `GITHUB_TOKEN` or unauthenticated GitHub API where public)
- **CI:** `.github/workflows/disklordz-ops-sync.yml` runs every 6 hours and commits snapshot updates. Add repo secret `DISKDLORDZ_OPS_TOKEN` (classic PAT with `repo` read on the vault) for live vault kanban/PR ingest; without it, sync uses committed fallbacks + public Instruments CI.
- **Kanban overrides:** column moves persist in `localStorage` until you update vault kanban in GitHub
- **Automations panel:** daily brief engine (P0 actions, gates, WIP, tripwire)

Fallback seed in `src/data/seed.ts` when snapshot fetch fails offline.

## Technology Intelligence (RADAR)

- Sidebar **Intelligence** → RADAR agent, 12-opportunity stack, factory M1 status, AI Dev Signals feed
- Snapshot: `public/intelligence-snapshot.json` via `npm run sync:intelligence` (merges `disklordz-ralph/prd.json` + optional `~/.openclaw/hermes-out/research/tech-radar/`)
- Pair with VPS: `disklordz-agent-stack → npm run tech-radar`
- Mobile chat uses `ai-agent-team` agent id `radar` when `VITE_AGENT_API_URL` is set

## Mobile crew (free coding agent)

- Sidebar **Mobile crew** → chat UI for elite agents (`CODE`, `PATCH`, `REVIEW`, …)
- Run `ai-agent-team` with a free [Google AI Studio](https://aistudio.google.com/) key
- Build or dev with API URL:

```bash
VITE_AGENT_API_URL=http://127.0.0.1:3000 npm run dev
```

Full OpenClaw multi-agent install: `openclaw/disklordz-elite-team/README.md`

## PWA

`npm run build` registers a service worker. On phone: open the site → Add to Home Screen.

## Publish

Built as a static Vite SPA suitable for Cursor **publish-app** from repo root with `project_dir: disklordz-command-center`.