# MAYBACH DRUM LAB PRO (R&D)

Luxury hip-hop / cinematic soul / orchestral trap **inspired** drum DNA — **original synthesis only**.

| Piece | Path |
|-------|------|
| Six identities | `Source/MaybachIdentity.h` |
| Live / Hybrid / Trap morph | `Source/MaybachAcousticMorph.h` |
| Three-layer core | `../factory/drum-architecture/Source/ThreeLayerDrum.h` |
| Master spec | `docs/plans/maybach-drum-lab-master-prompt.md` |
| M1 status | `MILESTONE-1-STATUS.md` |

**Not** the Lite ship path (`vst/starter-cpp`). **STACK-FROZEN:** ship Detroit Phonk Trap Maker before productizing this lab.

## M1 build (no JUCE)

```powershell
cd disklordz-ralph\maybach-drum-lab
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\MaybachM1Renderer.exe renders
```

## Optional JUCE VST3

```powershell
cmake -S . -B build -DDISKLORDZ_MAYBACH_BUILD_JUCE=ON
cmake --build build --config Release --target DiskLordzMaybachDrumLab_VST3
```

MIDI: notes **36** kick, **37** snare, **38** clap.

## Vol. 1 pack goal (M9)

500 one-shots + grooves — see master prompt §17; export will extend `export_kit.py` patterns, not proprietary recordings.