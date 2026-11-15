# AutoGPT platform (DiskLordz)

Use [Significant-Gravitas/AutoGPT](https://github.com/Significant-Gravitas/AutoGPT) as a **visual workflow** layer for text, JSON, and storefront APIs—not for C++ / audio DSP.

| Tool | Owns |
|------|------|
| **Cursor + Superpowers + REA** | JUCE `Source/`, MSVC builds, Catch2, `.pdb` audit |
| **OpenClaw** | Telegram sage, cron, `vst_market_intel`, vault kanban |
| **AutoGPT Agent Builder** | Kit drop → copy + draft listing + approval webhook |
| **C++ CLI** (`LouisianaKitGenerator.exe`, tripwire packer) | WAV render, `.blob`, `kit_manifest.json` |

## Deploy (Windows)

1. Clone AutoGPT: `git clone https://github.com/Significant-Gravitas/AutoGPT`
2. From `autogpt_platform/`: `docker compose up -d` → UI at `http://localhost:3000`
3. Merge volume mounts from `docker-compose.volumes.example.yml` (this repo)
4. Block graph: `workflows/kit-launch-blocks.md`

## Shared workspace (single source of truth)

Mount the same paths OpenClaw and Cursor use:

- `DLZ_REPO_ROOT/docs/` — market research, outreach, copy drafts
- `DLZ_REPO_ROOT/Assets/Releases/` — finished kit `.zip` + `kit_manifest.json`
- Optional read-only: `disklordz-ralph/vst/LICENSING.md`, `docs/copy/landing-page.md`

OpenClaw continues to own **scheduled** Firecrawl + Telegram; AutoGPT owns **event-driven** kit launch when a new release folder appears.

## Guardrails

- `max_retries: 2`, cheap model on CSV/metadata steps, frontier model only on final sales copy
- No AutoGPT code execution against `Source/` or MSVC
- Human approval block before Shopify/Gumroad/Lemon Squeezy write APIs
- Token/credit caps per run in Agent Builder settings

## Out of scope

- Compiling VST3, pluginval, REA Ghidra passes
- Autonomous cold email (see `vst_market_intel` outreach compliance)