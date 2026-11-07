# KitForge — Drum Synth + Step Sequencer (JUCE / C++)

KitForge is a drum machine built with JUCE that runs as a **VST3 plugin** and a **standalone app**.
There are no samples. Every sound comes from a built-in **drum synthesizer engine**, and the
**Kit Factory** uses that engine to design and generate whole drum kits.

## Features

### Step sequencer
- **16 pads** (A01–A16, two banks of 8 shown together) × 16 steps, 8 patterns (A–H, automatable)
- Per-step velocity: **click** toggles, **drag** paints or erases, **right-click/shift-click** cycles soft → medium → normal → accent, and **Alt-drag** sets the velocity finely
- **Hi-hat rolls / ratchets**: any step can play x2 (1/32), x3 (1/16-triplet roll) or x4 (1/64) repeats. Turn on
  **ROLL** mode and click steps to cycle the ratchet, or Ctrl/Cmd-click in normal mode. Rolls span the step as it is
  actually played, so they follow the swing. Repeats sit slightly under the first hit (80 % velocity).
- Pattern length 1–16, MPC-style swing (50–75 %), mute and solo per pad
- Copy / Paste / Clear patterns, plus **Generate**, which writes a groove that fits each pad's drum type. It picks
  one of three feels: boom-bap, four-on-the-floor, or trap (half-time snare, 808 sub, hat rolls)
- Built-in patterns: A boom-bap, B four-on-the-floor with shaker, C trap with hat rolls
- Follows the host transport and tempo when the DAW is playing. Otherwise it uses its own clock: press **PLAY** (in the standalone app or in the plugin)
- MIDI input: notes **C1–D#2 (36–51)** play pads 1–16 (standard 16-pad layout), so you can finger-drum or sequence from your DAW

### Drum synth engine (per track)
| Section | Parameters |
|---|---|
| Tone oscillator | Wave (sine/tri/saw/square), Pitch, Pitch-envelope amount and decay, Decay, Level, FM amount and ratio |
| Noise / Metal | White noise or a six-oscillator metallic generator, an SVF filter (LP/BP/HP) with cutoff and resonance, Level, Decay, and clap-style **Bursts** |
| Shape / Output | Transient Click, Attack, Drive (saturation), Volume, Pan, Choke group (e.g. the open hat is choked by the closed hat) |

It also has **FM Decay** (the FM index envelope, for DX-style chimes that strike metallic and fade to a pure sine)
and a per-pad **Lo-Fi Send** (how much of the pad goes through the vintage bus; 0 keeps an 808 sub fully clean).

### Vintage Sampler Bus (12-bit lo-fi)
The drum sends run through an emulation of a classic 12-bit sampler:

```
[Low Keep crossover] -> lows below the crossover bypass everything (kicks / 808 subs stay clean)
        | highs
        v
Drive (preamp saturation) -> Bit depth (4-16, continuous) -> Sample rate (zero-order hold, no interpolation)
        -> 2-pole 12 dB/oct low-pass  -> + lows -> Transient shaper (Punch / Tail) -> master
```

- **Pitch Hack** is the classic sampler trick (pitch up +N semitones before sampling, back down on playback).
  On a sampler with no interpolation, that is the same as sampling at `rate / 2^(N/12)` with a zero-order hold.
  A 26.04 kHz converter with +5 st works out to about 19.5 kHz, so the highs fold back as ringing alias tones.
  The panel shows the effective rate live.
- **Machine presets**: Clean, SP-1200 style (26.04 kHz / 12-bit / +5 st / 11 kHz LP), MPC60 style (40 kHz / 12-bit),
  DR-660 phonk (32 kHz / 12-bit, driven), SP-808 / Lofi-12 style (22.05 kHz, heavy vinyl). These are character
  approximations of those units, not circuit models.
- **Vinyl** adds hiss, crackle and pops. They go through the converter too, and only play while the machine is
  running or ringing out.

The master bus has a soft-knee safety clipper, so output never goes past 0 dBFS. The editor window can be resized
(the UI scales).

### Kit Factory
- **8 factory kits**: Boom Analog, Punch Machine, Electro Steel, Dusty Lo-Fi, Industrial Forge, Minimal Tick,
  plus **Jazz Phonk (16 pads)**: a jazz × phonk × trap 12-bit kit (see below). Every kit has 16 pads. In the original
  six kits, pads A09–A16 are derived from bank 1: low/high toms, a snare variation, a short "roll hat" that chokes
  with the main hats, an 808 sub, a percussion variation, a short crash and a shaker. Kit files saved with 8 pads
  get the same derived bank 2 when loaded.
- Drum types now include **808 Sub** (sine tuned to E1–A#1 with a glide; never crushed) and **Shaker**
- **FORGE KIT** builds a complete new kit (with a generated name) from per-drum-type "recipes"
- **FORGE VOICE** regenerates only the selected track, using its *Drum Type* (Kick, Snare, Clap, Closed/Open Hat, Tom, Rim, Cymbal, Cowbell, Perc)
- **MUTATE** nudges every parameter of the current kit in a random direction, for variations
- **SAVE / LOAD** stores kits as `.kitforge` XML files (by default in `Documents/KitForge Kits`). Double-click the kit name to rename it
- **EXPORT** renders every pad through the vintage bus into a **sample pack**: 24-bit / 44.1 kHz WAVs, an `.sfz`
  keymap (with hat choke groups), the `.kitforge` file and, if Vinyl is up, a 4-second vinyl texture loop.
  Files are named `<Kit>_<Type>_<Name>_A<pad>_<bits>bit.wav` (e.g. `JazzPhonk_Kick_JazzSubKick_A01_12bit.wav`).
  They are normalised by type: kicks / snares / claps to -1 dBFS, toms / rims -2, percussion -3, hats / cymbals -4.
- Each pad has a name. Double-click the name in the voice editor to change it; export uses it in the file names.
- Kits (including bus settings and pad names), patterns and mixer state are all saved with your DAW project

### Jazz × Phonk × Trap 16-pad kit
| Pad | Bank 1 | Pad | Bank 2 |
|---|---|---|---|
| A01 | Jazz Sub Kick: thud over sub, half send (mild crush) | A09 | Low Tom Drive: deep, saturated body |
| A02 | Memphis Bite Snare: hollow mid-range bite, hard attack | A10 | Mid Tom Bite: snappy drum-machine tom |
| A03 | Silk Rim Stab: FM wood strike + room tail | A11 | Phonk Vox Stab: saw through a resonant "ah" formant (a synth stand-in for a vocal chop) |
| A04 | Rage Trap Hat Closed: metallic hat pitched +4 st | A12 | Smooth Chime Perc: 2-op FM, ratio 1.414, 300 ms index decay |
| A05 | Chiff Hat Open: brushed swell (choked by A04) | A13 | Sub Glide 808: F1 sine gliding down, saturated, **no crush** |
| A06 | Phonk Cowbell F#: square, +12 st snap over 15 ms, +7 ct detune, driven | A14 | Vinyl Crackle: high-passed noise bursts |
| A07 | Jazz Brush Ghost: soft swirl, rolled-off top | A15 | Crash Jazz Muted: dampened metallic crash |
| A08 | Trap Clap Stack: 3 stacked claps, crunchy | A16 | Shaker LoFi Groove: soft double-grain shaker |

The kit uses the SP-1200 style bus with Low Keep 100 Hz, Punch 0.25 (about +1.5 dB on attacks) and light vinyl.
EXPORT writes all 16 pads at once, mapped to MIDI notes 36–51 in the SFZ.

Not covered by the synth engine: real acoustic or vocal recordings, chorus, and reverb. Layer those in your sampler or DAW.

### KitForgeFactory (command-line kit factory)
This tool generates sample packs in batches using the same engine and bus:
```bash
./build/KitForgeFactory_artefacts/Release/KitForgeFactory --factory                       # all factory kits
./build/KitForgeFactory_artefacts/Release/KitForgeFactory --kits 20 --machine 1 --seed 42 # 20 SP-1200 style kits
./build/KitForgeFactory_artefacts/Release/KitForgeFactory --kits 10 --random-machine --low-keep 100 --out packs
./build/KitForgeFactory_artefacts/Release/KitForgeFactory --help
```
Each kit gets its own folder of WAVs + SFZ + `.kitforge`, which you can load back into the plugin. The same seed
always produces the same batch.

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
finite, not silent, and never above 0 dBFS. It also checks parameter ranges, MIDI triggering, the
save/restore round trips, the vintage bus (12-bit quantisation, decimation, the pitch-hack rate, Low Keep
protecting a 45 Hz sub, Punch) and the sample-pack exporter (naming, levels, SFZ). It writes WAV files you can listen to:
```bash
./build/KitForgeRenderTest_artefacts/Release/KitForgeRenderTest renders
```

## Source layout
```
Source/
  Params.*          parameter definitions (global + 24 per track)
  DrumVoice.*       the synthesis engine (oscillator, FM, noise/metal, SVF, envelopes, drive)
  VintageBus.*      12-bit sampler emulation, transient shaper, vinyl noise
  Exporter.*        sample pack rendering (WAV / SFZ / kit file)
  Pattern.h         lock-free pattern storage
  KitFactory.*      factory kits, forging, mutation, kit files, pattern generator
  PluginProcessor.* sequencer clock (host-synced / internal), MIDI, voice mixing, state
  PluginEditor.*    UI: transport, Kit Factory, track headers, voice editor
  SequencerGrid.*   the step grid
Tests/RenderTest.cpp
Tools/FactoryCLI.cpp  KitForgeFactory command-line tool
```
