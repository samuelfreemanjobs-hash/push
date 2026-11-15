# Tool stack — locked (no new agent frameworks)

**Policy:** Ship Lite VST3 + first kit before adding tools. Ponytail is **rules-only** (no MCP/Docker required).

## In stack

| Layer | Tools | Repo entrypoints |
|-------|--------|------------------|
| 1 — DSP/VST | Cursor, **Superpowers** (plan + Catch2), **REA**, **Ponytail full** | `rea-juce-superpowers.mdc`, `ponytail-juce.mdc` |
| 2 — Kits | C++ CLI / generator (operator), manifests | `Assets/`, `docs/factory/kit_manifest.schema.json` |
| 3 — Intel | **OpenClaw**, **Firecrawl** | `vst_market_intel`, crons in `openclaw.disklordz.telegram.json5.example` |
| 4 — Launch | **AutoGPT** (optional Friday drops) | `openclaw/autogpt/` |

## Out of stack (for now)

- New MCP servers, agent frameworks, or meta-skills
- Ponytail **ultra** on `Modules/TrapCoreDSP/` or `processBlock` (use **full** only)
- Duplicate upstream `ponytail.mdc` if `ponytail-juce.mdc` is present (one ruleset)

## Division of labor

- **Superpowers** owns: brainstorm → plan → failing test → ship checklist  
- **Ponytail** owns: smallest C++/Python diff after the test exists  
- **REA** owns: post-build PDB spot checks, not architecture debates  

Weekly rhythm: `STACK-4-LAYER.md`. Ship path: `LAUNCH-ROADMAP-30D.md`.