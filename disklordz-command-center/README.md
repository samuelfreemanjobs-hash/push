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

## Publish

Built as a static Vite SPA suitable for Cursor **publish-app** from repo root with `project_dir: disklordz-command-center`.