# Tool stack — locked (no new agent frameworks)

**Policy:** Ship Lite VST3 + first kit before adding tools. Ponytail is **rules-only** (no MCP/Docker required).

## In stack

| Layer | Tools | Repo entrypoints |
|-------|--------|------------------|
| 1 — DSP/VST | Cursor, **Superpowers** (plan + Catch2), **REA**, **Ponytail full** | `rea-juce-superpowers.mdc`, `ponytail-juce.mdc` |
| 2 — Kits | C++ CLI / generator (operator), manifests | `Assets/`, `docs/factory/kit_manifest.schema.json` |
| 3 — Intel | **OpenClaw**, **Firecrawl** | `vst_market_intel`, crons in `openclaw.disklordz.telegram.json5.example` |
| 4 — Launch | **AutoGPT** (optional Friday drops) | `openclaw/autogpt/` |

## Opt-in local memory (operator machine)

**Claude-Mem** — `npx claude-mem install` via `scripts/install-claude-mem.ps1`; docs `docs/dev/claude-mem.md`. Local worker + Cursor hooks; not required to ship v1. Cloud agents use repo `AGENTS.md` + Supermemory when configured.

**Gemini CLI** — `npm i -g @google/gemini-cli`; root `GEMINI.md` + `docs/dev/gemini-cli.md`. Catalog audits, multimodal refs, SEO copy — **not** primary C++ IDE.

**Impeccable** — `npx impeccable install`; `PRODUCT.md`, `DESIGN.md`, `storefront/` — UI polish only; native JUCE editor ships v1 Lite.

## Opt-in Agency personas (not new tools)

Six `.cursor/rules/agency-*.mdc` files — summon with `@agency-*` in Agent chat only (`alwaysApply: false`). Runbook: `docs/launch/NEXUS-RUNBOOK.md`.

**Ruflo (Claude Code swarms)** — operator-local `npx ruflo@latest init`; roles in root `CLAUDE.md`; security `docs/dev/ruflo-opt-in.md`. Not required to ship Lite; trim default agent permissions after init.

## Out of stack (for now)

- New MCP servers, agent frameworks, or meta-skills beyond Agency **rule files**
- Ponytail **ultra** on `Modules/TrapCoreDSP/` or `processBlock` (use **full** only)
- Duplicate upstream `ponytail.mdc` if `ponytail-juce.mdc` is present (one ruleset)

## Division of labor

- **Superpowers** owns: brainstorm → plan → failing test → ship checklist  
- **Ponytail** owns: smallest C++/Python diff after the test exists  
- **REA** owns: post-build PDB spot checks, not architecture debates  

Weekly rhythm: `STACK-4-LAYER.md`. Ship path: `LAUNCH-ROADMAP-30D.md`.