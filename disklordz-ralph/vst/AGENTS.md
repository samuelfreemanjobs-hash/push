# AGENTS.md — Trap/Phonk C++ JUCE plugin & sample architecture

Place at **`C:\Dev\DetroitPhonkTrapVST3\AGENTS.md`** (bootstrap copies this file).

## 1. Core engineering philosophy (DRY & YAGNI)

- **Shared core library (`Modules/TrapCoreDSP/`):** Never duplicate DSP math, sample loaders, or MIDI state machines across plugins. All reusable audio code lives here and links into `DetroitPhonkTrapMaker`.
- **Data-driven content:** Never hardcode kit paths, pad counts, or UI colors. Drive kits and themes via JSON/XML manifests and encrypted `.blob` sample containers (see `C:\Dev\push\openclaw\drum-kit-factory\handoff\`).
- **Audio thread real-time safety:** Zero heap allocations (`new`, `malloc`, `std::vector::push_back`), zero locks, zero file or UI calls inside `processBlock()` or `renderNextBlock()`.

## 2. Superpowers workflow rules

- **Brainstorm → plan → execute:** Before production C++ for any DSP module, preset system, or packer, run Superpowers `brainstorming` + `writing-plans`; save `docs/plans/<feature>-spec.md`.
- **Strict red-green-refactor TDD:** No production DSP without a failing **Catch2** test in `tests/` first. Tests verify glide/saturation math, zero NaN/Inf, and (where possible) envelope curves on synthetic `juce::AudioBuffer<float>` blocks.
- **Automated verification gate:** Every task is incomplete until these pass on Windows:

```powershell
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
.\tools\pluginval\pluginval.exe --strictness-level 5 --validate ".\build\DetroitPhonkTrapMaker_artefacts\Release\VST3\Detroit Phonk Trap Maker.vst3"
```

(`COPY_PLUGIN_AFTER_BUILD` defaults OFF — validate the build-tree `.vst3`, not Program Files, unless you enabled admin copy.)

## 3. REA guardrails (Windows)

- **Targeted inspection only:** No full-binary decompile. Locate the inner PE in the `.vst3` bundle; filter by PDB, `GetPluginFactory`, or `Trap*` / `processBlock` symbols.
- **Ignore JUCE boilerplate:** Exclude graphics, Direct2D, PNG, zlib unless debugging UI.
- **Evidence format:** Observations / Inferences / Unknowns → append **`docs/rea-audit.md`** (and sync summaries to `C:\Dev\push\disklordz-ralph\vst\rea-notes\`).

Full prompts: `C:\Dev\push\disklordz-ralph\vst\REA-WORKFLOW.md`.

## 4. Sample pack & asset pipeline

- **Single source of truth:** Master WAVs in `Assets/RawMasters/`.
- **Automated packaging:** Kit scripts validate zero-crossing, silence trim, peak headroom; output unlocked customer WAV pack + encrypted plugin asset in one pass.

## 5. Layout

```text
Modules/TrapCoreDSP/   # shared DSP
Source/                # Processor, editor
tests/                 # Catch2 + CTest
docs/plans/            # Superpowers specs
docs/rea-audit.md      # REA session log
Assets/RawMasters/     # 24-bit masters
```

Ralph stories: `C:\Dev\push\disklordz-ralph\prd.json` (`DLZ-VST-*`).