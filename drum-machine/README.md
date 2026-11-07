# KitForge — Drum Synth + Step Sequencer (JUCE / C++)

KitForge is a drum machine built with JUCE that runs as a **VST3 plugin** and a **standalone app**.
There are no samples. Every sound comes from a built-in **drum synthesizer engine**, and the
**Kit Factory** uses that engine to design and generate whole drum kits.

## Features

### Step sequencer
- 8 tracks × 16 steps, 8 patterns (A–H, automatable)
- Per-step velocity: **click** toggles, **drag** paints or erases, **right-click/shift-click** cycles soft → medium → normal → accent, and **Alt-drag** sets the velocity finely
- Pattern length 1–16, MPC-style swing (50–75 %), mute and solo per track
- Copy / Paste / Clear patterns, plus **Generate**, which writes a groove that fits each track's drum type
- Follows the host transport and tempo when the DAW is playing. Otherwise it uses its own clock: press **PLAY** (in the standalone app or in the plugin)
- MIDI input: notes **C1–G1 (36–43)** play tracks 1–8, so you can finger-drum or sequence from your DAW

### Drum synth engine (per track)
| Section | Parameters |
|---|---|
| Tone oscillator | Wave (sine/tri/saw/square), Pitch, Pitch-envelope amount and decay, Decay, Level, FM amount and ratio |
| Noise / Metal | White noise or a six-oscillator metallic generator, an SVF filter (LP/BP/HP) with cutoff and resonance, Level, Decay, and clap-style **Bursts** |
| Shape / Output | Transient Click, Attack, Drive (saturation), Volume, Pan, Choke group (e.g. the open hat is choked by the closed hat) |

The master bus has a soft-knee safety clipper, so output never goes past 0 dBFS.

### Kit Factory
- **6 factory kits**: Boom Analog, Punch Machine, Electro Steel, Dusty Lo-Fi, Industrial Forge, Minimal Tick
- **FORGE KIT** builds a complete new kit (with a generated name) from per-drum-type "recipes"
- **FORGE VOICE** regenerates only the selected track, using its *Drum Type* (Kick, Snare, Clap, Closed/Open Hat, Tom, Rim, Cymbal, Cowbell, Perc)
- **MUTATE** nudges every parameter of the current kit in a random direction, for variations
- **SAVE / LOAD** stores kits as `.kitforge` XML files (by default in `Documents/KitForge Kits`). Double-click the kit name to rename it
- Kits, patterns and mixer state are all saved with your DAW project

## Building

Requirements: CMake ≥ 3.22 and a C++20 compiler (Visual Studio 2022, Xcode 14+, or GCC 11+/Clang 14+).
CMake downloads JUCE 8.0.4 automatically.

```bash
cd drum-machine
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

To use a JUCE checkout you already have, add `-DJUCE_DIR=/path/to/JUCE`.

Output files:
- VST3: `build/KitForge_artefacts/Release/VST3/KitForge.vst3`
- Standalone: `build/KitForge_artefacts/Release/Standalone/KitForge(.exe/.app)`

Copy `KitForge.vst3` to your VST3 folder:
- Windows: `C:\Program Files\Common Files\VST3`
- macOS: `~/Library/Audio/Plug-Ins/VST3`
- Linux: `~/.vst3`

Linux build dependencies:
```bash
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev \
  libxext-dev libxcomposite-dev libfreetype-dev libfontconfig1-dev libgl1-mesa-dev
```

On every push, GitHub Actions (`.github/workflows/kitforge.yml`) builds Windows, macOS and Linux
binaries and uploads them as downloadable artifacts.

### Test
`KitForgeRenderTest` renders every factory kit and a few forged kits for 4 bars and checks that the output is
finite, not silent, and never above 0 dBFS. It also checks parameter ranges, MIDI triggering, and the
save/restore round trips. It writes WAV files you can listen to:
```bash
./build/KitForgeRenderTest_artefacts/Release/KitForgeRenderTest renders
```

## Source layout
```
Source/
  Params.*          parameter definitions (global + 24 per track)
  DrumVoice.*       the synthesis engine (oscillator, FM, noise/metal, SVF, envelopes, drive)
  Pattern.h         lock-free pattern storage
  KitFactory.*      factory kits, forging, mutation, kit files, pattern generator
  PluginProcessor.* sequencer clock (host-synced / internal), MIDI, voice mixing, state
  PluginEditor.*    UI: transport, Kit Factory, track headers, voice editor
  SequencerGrid.*   the step grid
Tests/RenderTest.cpp
```
