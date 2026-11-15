# DiskLordz — agent instructions (Cursor / Claude Code / Ralph)

## Scope

| Path | Agent role |
|------|------------|
| `disklordz-ralph/` | One story per iteration; read `prd.json` + `PROMPT.md` |
| `openclaw/` | Telegram sage, skills, drum factory — no VST binary dumps in cloud |
| `disklordz-ralph/vst/` | JUCE / trap-phonk plugin — **Superpowers + REA + TDD** |
| `C:\Dev\DetroitPhonkTrapVST3` | MSVC workspace (copy `vst/AGENTS.md` there) |

## Superpowers (Cursor)

When the **Superpowers** plugin is installed (`/add-plugin superpowers` or Marketplace):

1. **Before C++ DSP:** run matching skills — `brainstorming` → `writing-plans` → `test-driven-development`.
2. **No production DSP** without a failing test first (Catch2 or `DrumTestCLI` assertion).
3. **Subagent tasks:** one module per worktree (glide, clip, choke, export).
4. **DAW bugs:** `systematic-debugging` then scoped **REA** (`vst/REA-WORKFLOW.md`).

If no Superpowers skill matches, still follow plans in `disklordz-ralph/vst/docs/plans/` and red-green-refactor manually.

## REA (Windows + Ghidra)

- Audit **your** RelWithDebInfo `.vst3` only; symbol-filtered prompts in `disklordz-ralph/vst/REA-WORKFLOW.md`.
- Cache in `disklordz-ralph/vst/rea-notes/`; never commit binaries or circumvent DRM.
- Pair with rule: `.cursor/rules/rea-juce-superpowers.mdc`.

## DRY business architecture

- **TrapCoreDSP** static lib — single home for glide, clip, choke (`vst/trap-core-dsp/`).
- **Kit manifests** — JSON/YAML from `openclaw/drum-kit-factory/handoff/`; no hardcoded WAV paths in C++.
- **Sample QA** — tripwire factory + agent-stack; one source folder → encrypted plugin blob + customer WAV zip.

## Verify matrix

| Lane | Command |
|------|---------|
| Monorepo CI | `npm run build` in `disklordz-command-center`, `openclaw/skills` |
| VST | `ctest -C Release`, `pluginval --strictness-level 5` |
| Ralph | `node disklordz-ralph/scripts/verify-story.mjs <id>` |

## Human gates

Operator-only: Telegram pairing, Stripe, vault merge, ClawHub publish, G3 sample-pack ship.