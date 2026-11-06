# MPC Sample Forge

Load any **VST3 instrument** and automatically render **100 (or any number of) one-shot
key and chord samples** in the music style you choose, ready to drop onto an **Akai MPC**.

Written in C++ with [JUCE](https://juce.com). It runs on Windows, macOS and Linux.

* **Hosts VST3 instruments** (pianos, Rhodes, synths, pads, organs and so on). Open the plugin's
  own window to pick or tweak a patch, then render with it.
* **46 built-in styles.** There are 14 genre styles: Neo-Soul, Lo-Fi Hip Hop, Dark Trap,
  Drill, Modern R&B, Gospel, Deep House, Jazz, Boom Bap, Afrobeats, Pop, Cinematic, Ambient
  and Synthwave. There are also 6 **one-shot recipe styles** for 16 Levels: Bell Tines,
  Organ Stabs, Plucks, Choir Notes, Detuned Synth and One-Shot Palette. You can also
  **write your own style** as a small JSON file.
* **16 Levels mode** (`--16levels`). Every sample is built on one root (C3), so
  *16 Levels → Tune* spreads it across the pads from a known pitch. Samples are cut short
  with a smooth fade, so no long sustains clutter the chops. Files can be grouped into
  folders of 16, one per pad bank.
* **The 9 recipes from the One-Shot Recipe Guide** (Jet Life EP Bell, Polyester Pluck, Memphis
  Organ, Phonk Cowbell, Zaytoven Organ, Luxury Harp, Screwed Choir, Trap Brass, Rhodes Bass).
  There are also 3 genre kits (West Coast, Memphis / Phonk, Atlanta) and an all-nine palette.
  Each recipe's trim, envelope, filters and FX are applied offline.
  Every batch also writes an `_MPC_Setup.txt` sheet with the pad settings.
* **Built-in synth**, so you get usable material with no VST3. It has 12 engines:
  * Electric piano.
  * EP + bell layer.
  * FM bell.
  * Drawbar organ.
  * Transistor combo organ.
  * Triangle pluck.
  * Karplus-Strong pluck.
  * Concert harp.
  * 808-style cowbell.
  * Formant choir.
  * Supersaw.
  * Analog brass.

  Every sample gets its own timbre variation, so 100 renders give 100 different sounds.
  Recipe tuning was measured within a few cents of the target note.
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

## Better selections: Smart Pick + Review & Keep

**Smart Pick** is the "Smart Pick (best of 3x)" checkbox in the app, or `--smart-pick` in
`mpcforge`. It renders 3× as many samples as you asked for, listens to every one, and keeps
only the best and most varied. It marks a sample down for:

* **Muddy voicings:** intervals stacked too low (the arranger's *low interval limits*, e.g. a
  third below C3 or a second below E3), or a sample clearly muddier than the rest of the batch.
* **Unclear pitch:** single notes whose pitch doesn't read cleanly. This matters most for
  16 Levels → Tune.
* **Harshness:** too much 2.5–5 kHz.
* **Low rumble**, **mostly-click** samples, and **near-silence**.
* **Near-duplicates:** each next pick is the best-scoring sample that also sounds most
  different from everything already chosen.

The batch keeps its balance of chords vs single keys, and of each recipe in a kit. Every
decision (score, reasons, closest match) is written to `_smart_pick.csv`. Even without Smart
Pick, the planner now avoids muddy low voicings when it has other options.

**Review & Keep** opens automatically after each batch. You can also open any folder with
**Review...**:

* **Up/Down** moves through the list and plays each sample. **Space** plays or stops.
* **K** keeps, **X** rejects, **U** clears. Marks are saved in the batch folder (`_review.json`).
* **Export Keepers** copies only your keepers into `_Keepers/Bank_A…`, numbered in your pad
  order, with the setup sheet.
* **Move Rejects to Trash** asks first, and the files can be restored from the Trash.

## Scale banks: every pad in key, no 16 Levels needed

Choose **Scale banks** in the app, or `--scale-bank` with `--key` / `--scale`. **Each `Bank_X`
folder is one sound laid out across 16 pads.** Load file 01 on Pad 1 … file 16 on Pad 16.

* **Note banks:** pads 1→16 climb the scale, e.g. C minor from C2 to D4. Every pad is a real
  render at that pitch, so there's none of 16 Levels' grain or dullness.
* **Chord banks:** the diatonic chord of each scale degree, labelled with its roman numeral,
  e.g. `i-Cm7`, `ii-Dm7b5`, `III-Ebmaj7`. Pads 8–14 repeat the chords in another voicing, and
  pads 15–16 go an octave up. You can play progressions straight off the pads, e.g. ii–V–I.
* `Samples 64` = 4 banks, each a new sound (or, with a kit, the next recipe), all in the same key.
* `_MPC_Setup.txt` prints a pad map of each bank, laid out like the MPC's 4×4 grid.
* Recipes that need it (pitched down with varispeed) are planned so they still *sound* in your key.
* Smart Pick and register sets are skipped for banks, since each bank is already a complete set
  across the scale.

### More recipes

| Recipe (`--recipe`) | Bank | Sound |
|---|---|---|
| `lead-synth` | notes | Mono synth lead (saw + pulse, filter blip, delayed vibrato), 1/8-dotted delay |
| `gfunk-lead` | notes | G-funk "whistle": near-sine lead that scoops into each note, vibrato, minor pentatonic |
| `bassline` | notes | Round bass (sine + filtered saw), tube warmth |
| `moog-bass` | notes | Moog-style resonant 4-pole bass with filter snap and drive (Dorian) |
| `soul-chords` | chords | Rhodes maj7 / m7 / m9 / 9 / 6-9, tape warmth |
| `ninth-chords` | chords | 9ths only (where a 9th doesn't fit the key, e.g. iii and vii in major, a 7th is used) |
| `scale-notes` | always a note bank | Clean keys, one bank per sound |
| `screw-rnb` | chords | Slowed R&B (−4 st varispeed), dark filter, wobble, dark hall |
| `trap-scale` | always a note bank | Harmonic-minor bell melodies, short reverb |
| `gospel-rnb-90s` | chords | maj9 / 13 / m11 / 9sus4 / 7#9 / 6-9, spread and rootless |
| `vaporwave` | chords | DX-style EP slowed −3 st, thick chorus, hall |
| `chillwave` | chords | Soft detuned synth, tape, wobble, hall |
| `phonkwave` | chords | Phrygian minor, −3 st, 12-bit, wow & flutter, hiss |

New built-in engines: `lead`, `whistle`, `bass` and `moogbass`. All four are in tune to within a few cents.

## MPC Sample friendly output

* **Register sets:** use **Registers** in the app, or `--registers octaves` / `--registers -12,12`.
  Every kept sample is also rendered an octave down and/or up, with the same timbre, into
  `Octave_Down/` and `Octave_Up/`. With 16 Levels → Tune, load the register nearest the
  notes you'll play: re-pitching a one-shot far makes it grainy and short (up) or dull and
  long (down). Copies that would land below MIDI 24 or above 108 are skipped.
* **Loudness matching:** **Level: loudness-matched** in the app, or `--loudness`. Every file is
  measured with BS.1770 K-weighting (the measure behind LUFS), then turned down to a common
  level. Pads play evenly, so you don't need to set pad levels by hand. Nothing is turned up
  past −1 dB peak, so nothing clips. The few naturally quieter samples stay at full level,
  and the summary says how many. The meter agrees with `pyloudnorm` to within 0.2 dB.
* **Short names:** the **Short names** checkbox, or `--short-names`. For example
  `014_EPBell_C3`, `002_Ebm9_d2` or `005_Cowbell_G4`. The number comes first, so the MPC's
  alphabetical list matches pad order. Voicings are abbreviated: `cl` close, `d2` drop-2,
  `rl` rootless, `sh` shell, `sp` spread, `op` open, `i1`/`i2` inversions.
* **16 Levels preview in Review:**
  * **Left/Right** re-pitches the selected sample a semitone at a time, **Shift** for an
    octave, and **0** resets.
  * It plays tape-style (faster = higher and shorter), the way MPC Tune does.
  * **R** runs a range check, from an octave down to an octave up.
  * The pitch readout turns red past a fifth: that's where a register set usually sounds better.
* `mpcforge --analyse file.wav` prints a file's loudness, peak, root and Smart Pick measurements.

## Download

Every push builds Windows, macOS (Apple Silicon and Intel) and Linux versions.

1. Open the repository's **Actions** tab.
2. Open the latest green **MPC Sample Forge build** run.
3. Under **Artifacts**, download `MPCSampleForge-Windows`, `-macOS` or `-Linux`.
4. Unzip it. Inside is a second zip (`.tar.gz` on Linux): unzip that too.
5. Read `QUICKSTART.txt`. The apps aren't code-signed yet, so Windows and macOS ask you to
   confirm the first launch. QUICKSTART explains how.

Each package also contains `Demo-Samples`, which that exact build rendered during its own
self-test.

To publish a proper Release with direct download links, push a tag such as
`git tag forge-v1.2.0 && git push origin forge-v1.2.0`.

## Diagnostics: finding bugs and errors

* **In the app:** click **Diagnostics** (top right). The window shows:
  * the system report: OS, CPU, audio device, MIDI inputs, VST3 folders and the plugins found;
  * details of the loaded instrument: maker, version, latency, channels, state size.

  **Run Self-Test** then:
  * renders every built-in engine and checks it for silence, NaN, level and tuning;
  * renders every recipe and checks its FX chain and end-point length;
  * writes a WAV file and reads it back;
  * test-renders the loaded VST3, measuring speed, pitch and state save/restore.

  **Copy Report** puts all of it on the clipboard.
* **From the command line:** `mpcforge --diagnose [--plugin X.vst3]` prints the same report.
  The exit code is 3 if anything failed, so it can be used in scripts and CI.
* **Log file:** plugin loads, batches, errors and self-test results are written to
  `forge.log` (Windows `%APPDATA%\MPCSampleForge\Logs`, macOS `~/Library/MPCSampleForge/Logs`,
  Linux `~/.config/MPCSampleForge/Logs`). If the app crashes, for example inside a plugin, a
  stack trace is written there too.
* **CI:** every build runs the full self-test on Windows, macOS and Linux and renders a demo kit.

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

### 16 Levels one-shot kits (drum program workflow)

The workflow: put short one-shots on drum-program pads and play them chromatically with
**16 Levels → Tune**.

1. Choose one of the one-shot styles:
   * **Bell Tines**
   * **Organ Stabs**
   * **Plucks**
   * **Choir Notes**
   * **Detuned Synth**
   * **One-Shot Palette**, which mixes all the recipes
2. Pick a sound source:
   * Keep **Built-in Synth**: the style picks the recipe, or you can override it with **Recipe**.
   * Or load a VST3 and dial in a bell, organ, pluck, choir or synth patch.
3. Set **16 Levels root** to **C3** and tick **16-pad banks**.
4. Optional: set a **Max length**. Each style already has a short default, e.g. 1 s for
   organ stabs and 1.5 s for plucks.
5. Generate. You get `Bank_A`, `Bank_B` and so on, each with 16 files whose root is C3.

On the MPC:
1. Load a bank into a drum program, one sample per pad.
2. Select a pad, turn on **16 Levels**, and choose **Tune**.
3. The 16 pads now play that sound chromatically. Down-pitching the choir notes works especially well.

With the built-in synth, every sample is a new timbre on the same root, so even 100
samples on one note are all different. A VST3 on a single root can only vary in velocity,
length and voicing. If that isn't enough variety for the count you asked for, the tool
says so. In that case, render a few patches as separate smaller batches.

### The One-Shot Recipe Guide (9 recipes + 3 kits)

Each recipe from *Technical Manual and Sound Recipe Guide for MPC One-Shot Melodic Synthesis*
is a built-in style. The recipe's trim, fade-in, One-Shot amp decay, filter and FX inserts are
**baked into the WAV**, in the guide's signal-flow order:

source → (varispeed) → trim to end point → amp envelope → filters & FX

| Recipe (`--recipe`) | Sound source (built-in engine) | End point | Shaping and FX | Root |
|---|---|---|---|---|
| `jetlife-ep-bell` | Rhodes tine + soft 2-op sine bell (`epbell`) | 350 ms | Decay 300 ms, 2-pole LP 4.2 kHz (res 12 %), tape saturation, chorus 0.8 Hz / 25 % | MIDI 60 |
| `polyester-pluck` | Triangle + sub-octave sine, 4-pole LP 2.8 kHz with a 180 ms filter envelope (`triangle`) | 280 ms | HP 200 Hz, 1/16 tape delay at 15 % feedback | MIDI 48 |
| `memphis-organ` | Transistor combo organ, minor stab (`comboorgan`) | 500 ms | 12 ms fade-in, decay 450 ms, 4-pole LP 2.4 kHz, wow & flutter 45 %, cassette hiss, 12-bit crush. Pad tune −3. | MIDI 60 |
| `phonk-cowbell` | TR-808 style cowbell (`cowbell`) | 180 ms | Decay 150 ms, BP 800 Hz–3.5 kHz, warm overdrive, tape noise | MIDI 79 (784 / 529 Hz pair) |
| `zaytoven-organ` | Drawbar organ with key click (`organ`) | 320 ms | Decay 300 ms, HP 150 Hz, +3 dB at 1.8 kHz, fast rotary, plate 0.6 s / 15 % | MIDI 60 |
| `luxury-harp` | Concert harp pluck (`harp`) | 450 ms | Decay 400 ms, HP 300 Hz, stereo expander, +12 st shimmer at a low mix | MIDI 60 |
| `screwed-choir` | Choir chord stab (`choir`), varispeed −5 st | 600 ms | 25 ms fade-in, decay 550 ms, 24 dB/oct LP 2.8 kHz, dark hall 2.2 s / 30 %, tape saturation | sounds at MIDI 55 |
| `trap-brass` | Horn + 80s analog brass (`brass`) | 300 ms | Decay 280 ms, +4 dB at 1.2 kHz, 4:1 fast compressor, short room | MIDI 60 |
| `rhodes-bass` | Low Rhodes note E1 (`epiano`) | 500 ms | Decay 480 ms, LP 1.8 kHz, tube overdrive | MIDI 28 (E1) |

Kits mix several recipes, round-robin, so every 16-pad bank gets some of each:

* `westcoast-kit`: Jet Life EP Bell, Polyester Pluck and Rhodes Bass.
* `memphis-kit`: Memphis Organ, Phonk Cowbell and Screwed Choir.
* `atlanta-kit`: Zaytoven Organ, Luxury Harp and Trap Brass.
* `guide-palette`: all nine, for the guide's "puzzle-piece" layering.

```bash
mpcforge --recipe jetlife-ep-bell --16levels --count 32 --out ~/MPC/JetLifeBell
mpcforge --recipe memphis-kit --16levels --count 64 --out ~/MPC/Memphis
mpcforge --recipe polyester-pluck --16levels --tempo 84    # 1/16 delay synced to 84 BPM
```

**Root notes.** The guide gives pitches in scientific notation, where C4 = middle C. So "tuned
to C4" is MIDI 60 and "E1" is MIDI 28. File names use the MPC convention instead, where
MIDI 60 is called **C3**. A Jet Life bell file is therefore named `..._Key_C3.wav`, and the
exact root is also stored inside the WAV.

**FX tails.** Some recipes put a reverb or delay on the pad. For those, the file may run
100–250 ms past the end point (`fxTailMs`) so the effect can decay before a clean fade. In
practice, a Mute Group choke cuts the tail on the next note anyway.

**`_MPC_Setup.txt`.** Every batch includes this sheet. It lists the pad settings for each recipe
(One-Shot, decay, pad tune, Mute Group 1, Full Level, 16 Levels → Tune), the guide's phrasing
strategy, and which files are in which bank.

**Using your own VST3 with a recipe.** Load a matching patch, e.g. a Rhodes for `jetlife-ep-bell`
or a B3 for `zaytoven-organ`, then pick the recipe. Its trim, envelope and FX are applied to
the plugin's sound. Recipes play at fixed velocity (Full Level), so one VST3 patch on one root
gives only a few distinct files. Use a smaller `--count`, or render several patches.

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

# A 16 Levels kit from the built-in synth: 64 one-shots on C3, in four 16-pad banks
mpcforge --style one-shot-palette --16levels --count 64 --out ~/MPC/Palette

# Organ stabs from your own VST3 organ, all on C3, max 0.5 s each
mpcforge --sound b3.forgesound --style organ-stabs --16levels --max-length 0.5 --count 48

# See what would be rendered without rendering it
mpcforge --style trap --key F --scale phrygian --seed 7 --plan-only
```

| Option | Meaning |
|---|---|
| `--plugin <file.vst3>` | VST3 instrument (its default preset) |
| `--sound <file.forgesound>` | Plugin plus the exact sound saved from the app |
| `--builtin` | Built-in synth (default when no plugin is given) |
| `--recipe <id>` | A guide recipe or kit (same as `--style`) |
| `--engine <name>` | Override the built-in synth engine: `epiano, bell, organ, pluck, choir, supersaw, epbell, triangle, cowbell, comboorgan, harp, brass, mixed` |
| `--16levels` | Same as `--root recipe --banks` |
| `--root <C3, Eb2, 60, recipe>` | Build every sample on this note (for 16 Levels → Tune); `recipe` uses each recipe's own root |
| `--tempo <bpm>` | Tempo for synced delays (default 90) |
| `--banks` | Write files in folders of 16 (`Bank_A`, `Bank_B` …) |
| `--max-length <sec>` | Override every recipe's end point (default: the recipe's; 0 = no cap) |
| `--smart-pick` | Render 3× the samples and keep only the best, most varied ones (writes `_smart_pick.csv`) |
| `--pick-factor <2-6>` | Candidates per kept sample for Smart Pick (default 3) |
| `--scale-bank` | 16 pads per sound climbing the key's scale (use with `--key`, `--scale`) |
| `--bank-type <notes\|chords>` | Note or chord banks (default: the style's) |
| `--registers <list>` | Also render every kept sample at these offsets in semitones, e.g. `-12,12` (`octaves` = `-12,12`) |
| `--loudness` | Match perceived loudness (LUFS) across the batch instead of peak level |
| `--short-names` | Short, pad-ordered file names such as `014_EPBell_C3` |
| `--analyse <file.wav>` | Print loudness, peak, root and Smart Pick measurements of a file |
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
| `engine` | Built-in synth engine (see `--engine`) or `mixed` |
| `root` | MIDI root used in 16 Levels mode |
| `processing` | `maxLength` (end point, s), `attackMs`, `ampDecayMs`, `fxTailMs`, `pitchShift` (varispeed semitones), `tempo`, and `fx`: an ordered list of effects |
| `pad` | Notes for `_MPC_Setup.txt`: `tune`, `muteGroup`, `fullLevel`, `phrasing` |
| `kit` | Mix other built-in styles by weight instead of planning chords |

Effects for `processing.fx`, with their parameters:

* `lowpass` / `highpass`: `freq`, `poles` (2 or 4), `resonance`
* `bandpass`: `low`, `high`
* `peak`: `freq`, `gainDb`, `q`
* `tape`, `tube`: `drive`
* `overdrive`: `drive`, `tone`
* `bitcrush`: `bits`, `downsample`
* `wowflutter`: `amount`
* `hiss`: `levelDb`
* `chorus`: `rate`, `depth`, `mix`
* `widen`: `width`
* `compressor`: `thresholdDb`, `ratio`, `attackMs`, `releaseMs`
* `reverb`: `kind` (plate, room, hall, darkhall), `decay`, `mix`
* `delay`: `sync` ("1/16", "1/8d") or `timeMs`, `feedback`, `mix`, `tape`
* `shimmer`: `timeMs`, `semitones`, `feedback`, `mix`
* `rotary`: `speed` (fast or slow)
* `gain`: `db`

See `mpcforge --dump-style jetlife-ep-bell` for a complete example.

Chord types (`5` is a power chord, named "power" in file names): `maj min dim aug sus2 sus4 5 add9 madd9 madd11 6 m6 69 m69 maj7 m7 7 m7b5 dim7
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
Source/Diagnostics.*     system report, self-test, log file + crash handler
Source/DiagnosticsPanel.* the Diagnostics window
Source/Analysis.*        Smart Pick: audio analysis, scoring and variety-aware selection
Source/ReviewPanel.*     the Review & Keep window
Source/BuiltInSynth.*    built-in synth (12 engines)
Source/Effects.*         offline FX: filters, EQ, saturation, crush, wow/flutter, chorus, reverb, delays, rotary ...
Tests/TestPlugin.cpp     builds the built-in synth as a VST3 (-DFORGE_BUILD_TEST_PLUGIN=ON)
```
