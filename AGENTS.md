# AGENTS.md — DiskLordz (monorepo + Trap/Phonk JUCE)

Cursor, Superpowers, Claude Code (see root **`CLAUDE.md`** for Ruflo swarms), and Ralph read this at session start.

## Monorepo lanes

| Path | Agent |
|------|--------|
| `disklordz-ralph/` | One `prd.json` story / iteration |
| `openclaw/` | Telegram, skills, drum-kit-factory |
| `disklordz-ralph/vst/` | VST architecture, tests, REA notes |
| `C:\Dev\DetroitPhonkTrapVST3` | Windows MSVC plugin workspace — copy `disklordz-ralph/vst/AGENTS.md` to project root |

---

# Trap/Phonk C++ JUCE plugin & sample architecture

## 1. Core engineering (DRY & YAGNI)

- **Shared core (`Modules/TrapCoreDSP/`):** No duplicated DSP, sample loaders, or MIDI state across plugins. Static CMake library (or INTERFACE until `.cpp` land) linked by every VST3 target.
- **Data-driven content:** No hardcoded kit paths, pad counts, or UI colors. Kits/themes from JSON/YAML manifests + `openclaw/drum-kit-factory/handoff/kit-spec.yaml`; encrypted `.blob` for factory audio when needed.
- **Real-time safety:** Zero heap allocs, locks, or file/UI I/O in `processBlock()` / `renderNextBlock()`.

## 2. Superpowers workflow

Install: Cursor Agent → `/add-plugin superpowers` ([obra/superpowers](https://github.com/obra/superpowers)).

- **Brainstorm → plan → execute:** Before new DSP/preset/packer code, run `brainstorming` + `writing-plans`; save `docs/plans/<feature>-spec.md` (VST tree) or `disklordz-ralph/vst/docs/plans/` (push repo).
- **Red-green-refactor:** No production DSP without a **failing Catch2 test** first (synthetic buffers; assert glide math, no NaN/Inf).
- **Subagents / worktrees:** One feature per branch (glide, clip, choke, drag-export).
- **DAW bugs:** `systematic-debugging`, then REA — not guess patches.

Details: `disklordz-ralph/vst/SUPERPOWERS.md`.

## 3. Verification gate (Windows VST)

Incomplete until all pass:

```powershell
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
.\tools\pluginval\pluginval.exe --strictness-level 5 --validate ".\build\DetroitPhonkTrapMaker_artefacts\Release\VST3\Detroit Phonk Trap Maker.vst3"
```

Optional RelWithDebInfo build for Ghidra/REA (`disklordz-ralph/vst/WINDOWS-BUILD.md`).

Ralph: `node disklordz-ralph/scripts/verify-story.mjs DLZ-VST-*`.

## 4. REA guardrails (Windows)

- **Scoped only:** No full-binary decompile. Use PE inside `.vst3`, filter PDB symbols / `GetPluginFactory` / `Trap*` / `processBlock`.
- **Ignore JUCE boilerplate:** Skip graphics, zlib, PNG unless debugging UI.
- **Evidence cache:** Observations / Inferences / Unknowns → append `docs/rea-audit.md` (VST project) and/or commit `disklordz-ralph/vst/rea-notes/*.md` in push.

Playbook: `disklordz-ralph/vst/REA-WORKFLOW.md`. Cursor rules: `.cursor/rules/rea-juce-superpowers.mdc`, `.cursor/rules/ponytail-juce.mdc` (optional [Ponytail](https://github.com/DietrichGebert/ponytail) minimalism). On-demand sprint personas: `@agency-*` in `docs/launch/NEXUS-RUNBOOK.md`.

**Claude-Mem (optional, local Cursor):** `docs/dev/claude-mem.md` — persistent session memory; run `scripts/install-claude-mem.ps1` on Windows after [Bun](https://bun.sh) is installed.

**Gemini CLI (optional, terminal):** root `GEMINI.md` — kit audits & release copy; `docs/dev/gemini-cli.md`. Do not use for MSVC/pluginval; Cursor owns C++.

**Impeccable (optional, UI):** `docs/dev/impeccable.md`, `PRODUCT.md`, `DESIGN.md`, `storefront/` — landing + WebView mocks; not `TrapCoreDSP`.

**Luger Drum Lab (R&D, post-Lite):** `disklordz-ralph/luger-drum-lab/` — genre engine on `ThreeLayerDrum`; prompt `docs/plans/luger-drum-lab-master-prompt.md`; does not replace `starter-cpp` ship path.

**Autoresearch (optional):** `factory/autoresearch/` — edit only `drum_engine.py`; `program.md` + `evaluate.py`; C++ path `docs/dev/autoresearch-cpp.md`.

**Hybrid commercial stack:** `docs/dev/hybrid-agent-stack.md` — Cursor=C++/pluginval, Python=kits, RAGFlow=retrieval; gate script `scripts/vst-build-pluginval-gate.ps1`.

**C++ autoresearch:** `vst/autoresearch-cpp/` — `Source/VoiceDSP.h`, `HeadlessRender`, `evaluate_vst.py`, `-DDLZ_AUTORESEARCH=ON`.

## 5. Sample pack & asset pipeline

- **Masters:** `Assets/RawMasters/` (24-bit / 44.1 kHz in VST workspace).
- **Single pass:** Validate zero-cross, trim silence, headroom → customer WAV zip + encrypted plugin blob.
- **Tripwire:** Factory scripts in `openclaw/drum-kit-factory/`; Grok `kit-spec` is source of truth.

## 6. Human gates

Operator only: Telegram pairing, Stripe, vault merge, ClawHub publish, G3 kit ship (`prd.json` `actor: operator`).