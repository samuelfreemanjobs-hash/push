# DiskLordz factory imports (from operator uploads)

Source-only in git; **WAV demos are not committed** (unpack zips locally to `Assets/Imports/`).

| Folder | Origin zip | Role in stack |
|--------|------------|----------------|
| `drum-forge-v4/` | DiskLordz_Drum_Forge_4 | **Layer 2** — Python synthesis kit factory (memphis, drift_phonk, southern_trap). Output → `Assets/Releases/`. |
| `autoresearch/` | (in-repo) | **R&D** — Karpathy-style `drum_engine.py` + immutable `evaluate.py` overnight loop. |
| `audio-factory/` | (in-repo) | **Opt-in** — unified AutoGen pipeline: DSP harness → mastered packs → APVTS presets. |
| `cinematic-dsp/` | Cinematic_DSP_CPP + 35 WAVs | **R&D** — procedural cinematic moods; offline renderer + proto JUCE processor. |
| `drum-architecture/` | Drum_Design_Architecture | **R&D** — `ThreeLayerDrum.h` + master meta prompt. |
| `../luger-drum-lab/` | (in-repo) | **R&D** — Luger DNA on three-layer; M1 tests + optional JUCE. |
| `../tm88-drum-lab/` | (in-repo) | **R&D** — TM88-*inspired* dark/emo/space trap DNA; M1 tests + optional JUCE. |
| `../maybach-drum-lab/` | (in-repo) | **R&D** — luxury/cinematic hip-hop DNA (6 banks + Live/Hybrid/Trap morph); M1. |
| `../good-money-drum-lab/` | (in-repo) | **R&D** — West Coast player/soul DNA (8 banks + Player/Luxury morph); M1. |

## Ship priority (STACK-FROZEN)

1. **Ship path:** `DetroitPhonkTrapMaker` + `TrapCoreDSP` + `drum-forge-v4` for Louisiana/Memphis **WAV packs**
2. **Later:** merge vetted ideas from `CinematicDSP` / `ThreeLayerDrum` into `Modules/TrapCoreDSP/` with Catch2 (Ponytail full)
3. **Do not** run two competing JUCE products without a `prd.json` story

## Drum Forge v4 (quick start)

```powershell
cd disklordz-ralph\factory\drum-forge-v4
pip install -r requirements.txt
python drum_forge_v4.py --genres memphis drift_phonk --variations 4 --notes C1 D1 F1 --out ..\..\..\Assets\Releases\Forge4_Kit
```

Copy resulting `manifest.json` into release root for AutoGPT / OpenClaw.

## Cinematic / three-layer (quick start)

```powershell
cd disklordz-ralph\factory\cinematic-dsp
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

See each subfolder `README.md`. Meta prompt: `disklordz-ralph/docs/plans/cinematic-three-layer-meta-prompt.md`.

## Restore full WAV demos from your zips

```powershell
.\scripts\unpack-factory-demos.ps1 -ZipDir "C:\path\to\uploads"
```

Targets: `Assets/Imports/cinematic-dsp-demo/`, `Assets/Imports/drum-architecture-examples/`, `Assets/Imports/drum-forge-v4-demo/`.