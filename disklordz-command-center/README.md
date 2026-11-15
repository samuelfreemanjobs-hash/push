# DiskLordz Command Center

Private ops dashboard for solo DiskLordz business: kanban, tripwire launch path, vault PR queue, VST CI snapshot, and agent lane reference.

## Run locally

```bash
cd disklordz-command-center
npm install
npm run dev
```

Open http://127.0.0.1:5173

## Data

Seed data mirrors vault `agents/disklordz/kanban.yaml`, `business-os.yaml`, and Instruments CI snapshot (2026-11-15). Kanban column moves persist in `localStorage`.

To refresh from vault, update `src/data/seed.ts` or add a future sync script.

## Publish

Built as a static Vite SPA suitable for Cursor **publish-app** from repo root with `project_dir: disklordz-command-center`.