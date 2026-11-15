# DiskLordz — optimal 4-layer stack

Assign each tool one job. Do not use AutoGPT for C++ DSP, or Cursor for scheduled scraping.

| Layer | Tool | Responsibility |
|-------|------|----------------|
| **1. C++ DSP & VST3** | Cursor + Superpowers + REA + Ponytail | `TrapCoreDSP`, Catch2, MSVC `.vst3`, REA `.pdb` RT safety; `.cursor/rules/ponytail-juce.mdc` |
| **2. Sound & kit factory** | C++ CLI (`LouisianaKitGenerator.exe`) | 24-bit WAV, encrypted `.blob`, `kit_manifest.json` — no LLM audio |
| **3. Scheduled intel & outreach** | OpenClaw + Firecrawl | Price cron, producer leads, Telegram only on alerts / draft approval |
| **4. Launches & support** | AutoGPT Agent Builder | On kit drop: storefront copy, Shorts scripts, install FAQ — human approve APIs |

## Weekly loop

| Day | Focus |
|-----|--------|
| **Mon–Wed** | Cursor: one DSP or preset → Catch2 → REA → run kit generator → drop into `Assets/Releases/` |
| **Thu** | OpenClaw: `track_competitors.py` + `find_producer_leads.py`; approve outreach drafts (~5 min) |
| **Fri** | AutoGPT: new release triggers staged copy in `docs/factory/staged/`; approve → draft storefront |

## Shared paths (one tree)

All layers read/write the same repo root (`DLZ_REPO_ROOT` / `DLZ_WIN_ROOT`):

- `docs/market-research/`, `docs/outreach/`, `docs/copy/`, `docs/factory/`
- `Assets/Releases/` (+ `kit_manifest.json`)
- `disklordz-ralph/vst/` — docs only for AutoGPT; **builds on Windows host**

Setup: `openclaw/scripts/setup-dlz-shared-workspace.ps1` and `openclaw/autogpt/docker-compose.volumes.example.yml`.

## Related docs

- VST architecture: `vst/TRAP-PHONK-ARCHITECTURE.md`
- OpenClaw pods: `openclaw/disklordz-picks/README.md`
- AutoGPT kit graph: `openclaw/autogpt/workflows/kit-launch-blocks.md`
- Drag-export: `vst/starter-cpp/DLZ-VST-007-INTEGRATION.md`