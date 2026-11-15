# Serena + Context7 — production JUCE 8 workflow (Windows)

## Problem

| Bottleneck | Fix |
|------------|-----|
| clangd indexes `build/_deps/juce-src` → GB RAM | `.serena/project.yml` `ignored_paths: build/**` |
| MSVC `/W4` confuses clangd | Root `.clangd` strips MSVC flags; use `compile_commands.json` |
| Half-finished APVTS refactors | Serena memory SOP + `excluded_tools: replace_content` |

## Template paths

| File | Location |
|------|----------|
| Serena project | `vst/templates/drumkit-plugin/.serena/project.yml` |
| SOP memory | `.serena/memories/dsp_refactoring_sop.md` |
| clangd | `vst/templates/drumkit-plugin/.clangd` |
| Setup | `vst/templates/drumkit-plugin/setup-serena.ps1` |

Copy the same trio to `starter-cpp/` after operator approval (ship path).

## One-time init

```powershell
cd disklordz-ralph\vst\templates\drumkit-plugin
.\setup-serena.ps1
```

Requires JUCE at `vst/starter-cpp/externals/JUCE`. Prefer **Ninja** for reliable `compile_commands.json`.

## Context7 + Serena together

1. **Context7** — authoritative JUCE 8 API text before edits.
2. **Serena** — `find_referencing_symbols` across `src/` + `ui/app.js` + `index.html`.
3. **Gate** — pluginval strictness 10 (or swarm `vst_rd_swarm.py`).

## Cross-language APVTS

`ui/app.js` exports `CONTROL_IDS`; relay id in `PluginEditor.h` must match `outputGain` in HTML.

When adding a parameter, extend all three in one session with Serena reference search.

## Agent attach

- Claude Code / Goose: open repo at `drumkit-plugin`; Serena loads `.serena/memories/` automatically.
- Cursor: enable clangd; optional Serena MCP per vendor docs.

See also: `docs/dev/elite-dsp-agent-context7.md`, `drumkit-plugin/AGENTS.md`.