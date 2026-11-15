# DiskLordz Drum Design Architecture — v0.2 prototype

This package includes a compilable **pure C++20 three-layer synthesis prototype** and one **master meta-prompt** for extending it into the full JUCE 8 VST3 and standalone product.

## Included
- `Source/CinematicDSP.h` — retained original 7-voice Cinematic DSP foundation.
- `Source/ThreeLayerDrum.h` — new deterministic Body / Attack / Texture engine, independently enabled and rendered.
- `Tests/three_layer_test.cpp` — deterministic, finite, audible, separate-layer tests across 7 voices × 5 Cinematic moods.
- `Tools/render_three_layer.cpp` — creates 35 original 24-bit/48 kHz WAVs.
- `Examples/` — 35 exported WAVs.
- `DISKLORDZ_MASTER_META_PROMPT.md` — comprehensive architecture/build prompt for Cursor or Claude Code.

## Build standalone renderer (Windows Visual Studio)
```
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\Release\ThreeLayerRenderer.exe .\renders
```
Linux/macOS host build (requires C++20 toolchain):
```
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/ThreeLayerRenderer ./renders
```

## Scope and honest limitations
The three-layer core has been compiled and tested on Linux; this ZIP does **not** yet contain a complete JUCE VST3 plugin or a tested Windows GUI. Live parameter smoothing, cross-layer phase compensation, oversampled distortion, robust output limiting/true-peak checks, advanced instrument-specific generators, and production-ready thread ownership are **future milestones**. Current outputs are intentionally not loudness-mastered. Use the master prompt to implement and test these before selling the synthesizer.

## Integration intent
Place `Source/ThreeLayerDrum.h` and `Source/CinematicDSP.h` in the existing `DiskLordz_Cinematic_DSP_CPP` project. New voices can use `dl::ThreeLayerDrum` with `prepare()`, `start(design)`, `next()` and `stop()`. Render only on offline/worker threads using `dl::renderDesign()` because it allocates a vector. `next()` does not allocate.
