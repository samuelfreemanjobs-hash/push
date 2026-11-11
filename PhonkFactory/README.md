# Phonk Factory

Memphis phonk drum kit + loop generator as a **VST3 instrument** and a **standalone app** (JUCE 8 / C++17).
It is a C++ port of the original Python/scipy kit script, extended into a playable, tempo-synced instrument.

* **Kit synth** – 808-style kick (exponential pitch drop + click), snare (shell + band-passed wires), closed/open
  metallic hats, cowbell; 4 variations of each = 20 pads on MIDI notes **C1-G2** (36-55).
  Every voice runs through the cassette chain: tape saturation -> 12-bit SP-1200 quantiser -> gated hiss -> low-pass.
* **Loop factory** – seeded Memphis pattern generator (classic 16-step groove, ghost notes, open-hat slides, hat
  drop-outs, triplet / 32nd rolls, cowbell, swing, humanise). `Variation = 0` gives exactly the original groove.
  Plays live, sample-accurate, locked to the host tempo/PPQ (or an internal clock in standalone).
* **Tape bus** – wow & flutter, drive, bit-crush, hiss, LPF on the whole mix.
* **Export** – `EXPORT KIT` writes 20 one-shots + 3 classic loops (112/118/124) + 5 seeded variations + the current
  loop as 16-bit WAVs (MPC-ready, loops are seamless / tails wrap). `DRAG LOOP TO DAW` renders the current loop and
  starts a file drag into your DAW or MPC software.

Same seed = same audio, on every platform (own RNG, no libm-dependent randomness).

## Build

Needs CMake >= 3.22 and a C++17 compiler. JUCE 8.0.9 is downloaded by CMake.

```sh
# Linux deps (Debian/Ubuntu): libasound2-dev libfreetype-dev libfontconfig1-dev libx11-dev libxrandr-dev
#                             libxinerama-dev libxcursor-dev libxext-dev libgl-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Outputs under `build/PhonkFactory_artefacts/Release/`: `VST3/Phonk Factory.vst3`, `Standalone/Phonk Factory`
(+ `AU` on macOS). Windows: use the Visual Studio generator, same commands.

Without JUCE (just the WAV generator, no GUI): `cmake -S . -B build -DPHONK_BUILD_PLUGIN=OFF && cmake --build build`, then
`build/kitgen --out Memphis_Phonk_MPC_Kit --seed 7 --bpm 118 --bars 4 --variation 0.7`.

## Use

Standalone: press **PLAY** (turn **HOST SYNC** off), click pads or play MIDI. In a DAW: load as an instrument, keep
**HOST SYNC** on, press play - the pattern follows the host tempo and position. **NEW PATTERN** / **REROLL KIT** roll
new seeds; kit knobs re-render the one-shots in the background, tape/mix/sequencer knobs are live.

## Test

`PhonkSelfTest` (built with the plugin) drives the real processor headlessly - idle silence, pad latency, sample-accurate
sequencer timing and loop wrap, block-size independence, host tempo/PPQ sync, state recall, export, editor paint:

```sh
xvfb-run -a build/PhonkSelfTest_artefacts/Release/PhonkSelfTest snapshot.png
```

## Layout

| Path | |
|---|---|
| `Source/PhonkDSP.*` | JUCE-free core: kit synth, filters, pattern generator, bus FX, loop render, WAV writer, export |
| `Source/PluginProcessor.*` | voices, MIDI pads, sequencer, host sync, background kit renderer, lock-free hand-off (`RcuSlot.h`) |
| `Source/PluginEditor.*` | UI: pads, pattern view, knobs, export, drag-out |
| `Tools/kitgen.cpp`, `Tools/selftest.cpp` | CLI and headless test |
