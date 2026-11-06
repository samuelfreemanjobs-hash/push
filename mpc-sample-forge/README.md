# MPC Sample Forge

Load any **VST3 instrument** and automatically render **100 (or any number of) one-shot
key and chord samples** in the music style you choose, ready to drop onto an **Akai MPC**.

Written in C++ with [JUCE](https://juce.com). It runs on Windows, macOS and Linux.

* **Hosts VST3 instruments** (pianos, Rhodes, synths, pads, organs and so on). Open the plugin's
  own window to pick or tweak a patch, then render with it.
* **14 built-in styles:** Neo-Soul, Lo-Fi Hip Hop, Dark Trap, Drill, Modern R&B, Gospel,
  Deep House, Jazz, Boom Bap, Afrobeats, Pop, Cinematic, Ambient, Synthwave. You can also
  **write your own style** as a small JSON file.
* **Music theory engine:** 38 chord types (triads to 13ths and altered dominants) and 8 voicing
  types (close, inversions, drop-2, open, rootless, shell, spread). Chords can be rolled like a
  real player, with natural velocity variation.
* **Random keys, or lock every sample to one key and scale.** Locking a key gives you
  a kit where every chord fits together (e.g. "all diatonic to Eb minor").
* **Mix of samples:** held chords, short stabs, single notes, octaves and fifths. The
  share of each comes from the style.
* **MPC-ready WAVs:**
  * 44.1 or 48 kHz, 16 or 24 bit, stereo.
  * Silence trimmed so the sample starts on the hit.
  * Release tail kept until it fades out, then a short fade.
  * Peak-normalised to -1 dBFS.
  * Root note written into the WAV `smpl` chunk.
  * Names with no awkward characters (`#` becomes `s`).
* **Reproducible:** the same seed gives the same batch. Every batch also writes
  `_manifest.csv` (the notes in each file) and `_batch.json` (the settings used).
* **Automation:** `mpcforge`, a command-line tool, can run batches from scripts.
* A small **built-in FM electric piano** lets the app work even before you install a VST3.

Example output for `--style neo-soul --key Eb`:

```
Neo-Soul_Ebmin_001_Chord_Eb9sus4_shell.wav
Neo-Soul_Ebmin_002_Chord_Bbm11_rootless.wav
Neo-Soul_Ebmin_003_Chord_Gb6-9_drop2.wav
...
Neo-Soul_Ebmin_014_Stab_Bbm9_spread.wav
...
Neo-Soul_Ebmin_078_Key_Db3.wav
Neo-Soul_Ebmin_099_Octave_Bb2.wav
_manifest.csv
_batch.json
```

Note names follow the MPC / Ableton convention, where MIDI 60 = **C3**.

---

## Building

You need **CMake 3.22+** and a C++17 compiler:

* **Windows:** Visual Studio 2022 (Community is fine) with "Desktop development with C++".
* **macOS:** Xcode, or just its command-line tools (`xcode-select --install`).
* **Linux:** `sudo apt install build-essential cmake libasound2-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxcomposite-dev libxext-dev libfreetype-dev libfontconfig1-dev`

```bash
cd mpc-sample-forge
cmake -B build -DCMAKE_BUILD_TYPE=Release     # downloads JUCE 8.0.4 automatically
cmake --build build --config Release
```

The build produces:

| | Windows | macOS | Linux |
|---|---|---|---|
| App | `build/MPCSampleForge_artefacts/Release/MPC Sample Forge.exe` | `.../Release/MPC Sample Forge.app` | `.../Release/MPC Sample Forge` |
| CLI | `build/mpcforge_artefacts/Release/mpcforge.exe` | `build/mpcforge_artefacts/Release/mpcforge` | same |

If you already have JUCE checked out, add `-DJUCE_DIR=/path/to/JUCE`.

The GitHub Actions workflow in `.github/workflows/mpc-sample-forge.yml` builds Windows,
macOS and Linux versions. Download them from the run's **Artifacts** section.

---

## Using the app

1. **Instrument:** pick a VST3 from the list (it shows everything in the standard VST3
   folders), or click **Browse...** to choose a `.vst3` file.
2. **Edit Sound:** opens the plugin's own window. Choose a preset, such as a Rhodes, a
   Wurli, a grand piano or a pad. Play the on-screen keyboard, or a MIDI controller, to hear it.
   **Save Sound** stores the plugin and its settings in a `.forgesound` file so you can
   reuse that sound later or from the command line.
3. **Style:** choose a genre, or load your own style with **Style JSON...**.
4. **Key / Scale:**
   * Leave **Random keys** for chords in every key.
   * Pick a key (e.g. Eb) to make everything diatonic to that key. By default the scale
     is the style's; you can override it (major, minor, dorian, phrygian, lydian,
     mixolydian, harmonic/melodic minor).
5. **Samples:** 100 by default. **Seed:** the same seed gives the same batch. Use **Dice** for a new one.
6. **Preview List** shows what will be rendered. **Audition Random** plays one sample through
   the plugin.
7. **Generate.** Each batch goes into its own subfolder of the output folder. That folder
   opens when the batch finishes.

### Getting the samples onto the MPC

* **Standalone MPC:** copy the batch folder to a USB stick or SD card. Then open the MPC
  browser, load the samples, and drag them onto pads or into a Drum program.
* **MPC software or MPC 3:** drag the folder into the browser.

Because the root note is in each file, single-note samples can also be dropped into a
**Keygroup** program and played chromatically.

---

## Automation: `mpcforge`

```bash
mpcforge --list-styles
mpcforge --list-plugins

# 100 neo-soul samples from a VST3, using a sound saved from the app
mpcforge --sound rhodes.forgesound --style neo-soul --count 100 --out ~/MPC/NeoSoul

# Everything in Ab major, gospel style, 48 kHz / 16-bit
mpcforge --plugin "/Library/Audio/Plug-Ins/VST3/Keyscape.vst3" --style gospel --key Ab --rate 48000 --bits 16

# See what would be rendered without rendering it
mpcforge --style trap --key F --scale phrygian --seed 7 --plan-only
```

| Option | Meaning |
|---|---|
| `--plugin <file.vst3>` | VST3 instrument (its default preset) |
| `--sound <file.forgesound>` | Plugin plus the exact sound saved from the app |
| `--builtin` | Built-in FM keys |
| `--style <id>` / `--style-file <json>` | Built-in style, or your own |
| `--count <n>` | Number of samples (default 100) |
| `--key <C, F#, Bb, random>` | Lock to a key (default random) |
| `--scale <name>` | Override the style's scale |
| `--seed <n>` | Reproducible batches |
| `--prefix <text>` | File name prefix (default: the style name) |
| `--out <folder>` | Output folder |
| `--rate 44100\|48000`, `--bits 16\|24` | File format |
| `--no-normalize` | Keep the plugin's own level |
| `--tail <sec>` | Longest release tail kept after note-off (default 6) |
| `--warmup <sec>` | Time for sample-based plugins (Kontakt and similar) to load before rendering (default 1.5) |

A loop like this renders several genres overnight:

```bash
for s in neo-soul lofi gospel rnb house; do
  mpcforge --sound rhodes.forgesound --style $s --count 100 --out ~/MPC/Keys/$s
done
```

---

## Making your own style

Print a built-in style to use as a starting point:

```bash
mpcforge --dump-style neo-soul > my-style.json
```

Alternatively, copy `Examples/my-style.json`.

| Field | Meaning |
|---|---|
| `scale` | Scale used when a key is chosen |
| `chords` | Chord type and weight. A higher weight makes that type more frequent. |
| `voicings` | `close, inv1, inv2, drop2, open, rootless, shell, spread` with weights |
| `chordRatio` | Fraction of the batch that is chords (the rest are single-key one-shots) |
| `keyTypes` | Weights for single `note`, `octave`, `fifth` and short triad `stab` one-shots |
| `chordRootRange`, `keyRange` | MIDI note ranges (60 = C3) |
| `velocity` | Velocity range |
| `hold` | How long chords are held, in seconds |
| `stabChance`, `stabHold` | How often a chord is a short stab, and how long a stab lasts |
| `strumMs` | Maximum roll between the chord's notes (0 = all notes hit together) |

Chord types: `maj min dim aug sus2 sus4 5 add9 madd9 madd11 6 m6 69 m69 maj7 m7 7 m7b5 dim7
mmaj7 7sus4 maj9 m9 9 9sus4 mmaj9 m11 maj7s11 maj9s11 13 maj13 m13 7s9 7b9 7s5 13b9 quartal quintal`.

---

## Tips

* **Sample-based plugins** such as Kontakt, Keyscape or Spitfire may need a longer
  `--warmup`, so their samples have finished loading before rendering starts. If a
  sample comes out silent, the log says so.
* Rendering runs faster than real time. Nothing plays through your speakers while a batch renders.
* Plugins with authorisation dialogs or online checks should be set up once in the app first.
* Only use instruments whose licence lets you use rendered audio in your music. Most
  commercial instruments do, but check yours.

## Project layout

```
Source/Theory.*          chords, scales, voicings
Source/Styles.*          built-in styles (JSON) + style file loader
Source/SamplePlanner.*   picks the unique chords / notes for a batch
Source/OfflineRenderer.* drives the plugin offline, trims, normalises, writes WAV
Source/PluginHost.*      VST3 scanning / loading, .forgesound files
Source/BatchJob.*        background rendering + manifest
Source/MainComponent.*   the app UI
Source/CommandLine.*     shared CLI logic; CliMain.cpp = mpcforge
Source/BuiltInKeys.h     fallback FM electric piano
Tests/TestPlugin.cpp     builds the FM keys as a VST3 (-DFORGE_BUILD_TEST_PLUGIN=ON)
```
