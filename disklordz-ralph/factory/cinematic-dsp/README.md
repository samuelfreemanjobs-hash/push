# DiskLordz CINEMATIC SHADOWS — C++ DSP Engine

**Original synthesis inspired by broad dark/cinematic trap drum design.** No copyrighted audio samples or imitation of any specific recording. This is a new C++ reimplementation of the sound *families* in the 64-file starter pack, not a bit-for-bit port of its Python generator.

## What is included
- `Source/CinematicDSP.h`: standalone C++20 voice DSP (808, kick, snare, clap, closed hat, open hat, FM/modal percussion) with 5 mood recipe families: Dark, Luxury, Haunted, Aggressive, Atmospheric.
- `Tools/render_main.cpp`: zero-dependency command-line renderer, 35 mono 48 kHz / 24-bit WAV files.
- `Tests/dsp_test.cpp`: determinism, non-silence, finite output, peak-level smoke checks for all 35 combinations.
- `Source/PluginProcessor.cpp`: initial JUCE 8 instrument processing MIDI notes 36–42, 32 concurrent one-shot voices, stereo output, GenericAudioProcessorEditor, APVTS mood and level parameters.
- `CMakeLists.txt`: C++ test and renderer build; JUCE VST3 + Standalone is opt-in.

## Build renderer now (Windows / Visual Studio 2022)
```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\Release\CinematicRenderer.exe .\renders
```

## Build renderer now (Linux/macOS)
```sh
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/CinematicRenderer ./renders
```

## Build JUCE VST3 + Standalone (Windows / requires Git and internet)
```powershell
cmake -S . -B build-juce -G "Visual Studio 17 2022" -A x64 -DDISKLORDZ_BUILD_JUCE_PLUGIN=ON
cmake --build build-juce --config Release
```
JUCE 8.0.4 is fetched from the official `juce-framework/JUCE` GitHub repository. Consult JUCE licensing requirements before distributing closed-source plugins.

### MIDI mapping (prototype)
- 36 = 808
- 37 = Kick
- 38 = Snare
- 39 = Clap
- 40 = Closed Hat
- 41 = Open Hat
- 42 = FM/modal percussion

## Current limitations / next engineering steps
- Plugin build is **not tested in this environment**; JUCE is not installed here and network fetch is unavailable. Renderer and unit tests can be compiled and executed locally.
- Basic one-pole filters share state in each voice; separate band-specific filters and multi-stage responses should replace these for a commercial release.
- Distortion is **not yet oversampled**. Add 4x/8x oversampled distortion with latency compensation and frequency-domain QA.
- MIDI triggers choose a new seed each time; for repeatable *plugin playback*, use per-pad persistent seeds or a deterministic counter persisted in plugin state.
- No open/closed hi-hat choke, independent pad envelopes, voice-stealing fades, real-time parameter smoothing, custom GUI, preset browser, batch mutations, or true peak limiter yet.
- Offline one-shot renderer does not yet recreate the original four loop demos. Use the original starter pack for those.
- Current audio is mono synthesized and replicated to both stereo channels.
- This is a technical foundation, not an audited commercial-quality VST.

## Original starter kit relation
The prior Python kit includes 12 808s, 10 kicks, 10 snares, 8 claps, 8 closed hats, 4 open hats, 8 percussion hits, and 4 demo loops. The new C++ renderer outputs 7 sound types × 5 cinematic moods = 35 fresh one-shots.
