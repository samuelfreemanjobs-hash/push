# DiskLordz Luger Drum Lab

Genre engine inside **DiskLordz Drum Design Lab** — Lex Luger–*influenced* Southern orchestral trap character through **original three-layer synthesis** (`factory/drum-architecture/`). Not a replacement for the shipping product **Detroit Phonk Trap Maker** (`vst/starter-cpp/`).

## Stack position

| Track | Path | Status |
|-------|------|--------|
| **Ship (v1)** | `vst/starter-cpp/` + `TrapCoreDSP` + `drum-forge-v4` | Lite VST3 |
| **R&D (this)** | `luger-drum-lab/` | Milestone 1 offline + tests |
| **Shared core** | `ThreeLayerDrum.h`, `CinematicDSP.h` | Reuse — do not fork |

Master agent prompt: `docs/plans/luger-drum-lab-master-prompt.md`.

## Milestone 1 (implemented in repo)

- `Source/LugerIdentity.h` — five DNA identities → `DrumDesign` modifiers
- `KickMode` presets for Luger Kick Cannon (recipe layer only; full DSP in M2)
- `Tests/milestone1_test.cpp` — kick / snare / 808 × 5 identities, deterministic
- `Tools/render_milestone1.cpp` — 15 showcase WAVs (offline)
- Optional JUCE: `-DDISKLORDZ_LUGER_BUILD_JUCE=ON` → VST3 + Standalone (`LugerPluginProcessor.cpp`)

### Build (Linux / macOS / Windows)

```bash
cd disklordz-ralph/luger-drum-lab
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/LugerM1Renderer ./renders
```

Windows:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\Release\LugerM1Renderer.exe .\renders
```

JUCE plugin (operator machine with network for FetchContent):

```powershell
cmake -S . -B build-juce -G "Visual Studio 17 2022" -A x64 -DDISKLORDZ_LUGER_BUILD_JUCE=ON
cmake --build build-juce --config Release
```

## What is **not** done yet (honest)

| Part | Milestone |
|------|-----------|
| Seven full flagship engines | M2 |
| Orchestral Percussion Forge | M2–M3 |
| DNA morph interpolation + locks | M4 |
| 450-shot War Machine factory | M7 |
| 16-step sequencer + rolls | M6 |
| Crimson industrial GUI | M8 |
| pluginval / FL matrix | M9 |

## Integration rules (from STACK-FROZEN)

1. No new agent frameworks; Superpowers + Ponytail for DSP edits.
2. Port proven voice kernels into `TrapCoreDSP` only with Catch2 stories — don't split the live plugin codebase blindly.
3. War Machine Vol. 1 ships as an **expansion SKU** after Core trap plugin is stable.

Milestone report: `MILESTONE-1-STATUS.md`.