# Memphis Dust

A drum-bus / master-bus plugin (VST3 / AU / Standalone, JUCE 8) that turns clean, modern drum samples into dusty 90s Memphis cassette-tape drums. It combines the classic Memphis workflow with a model of a worn Tascam Portastudio 424/414: head saturation, capstan wow & flutter, head bandwidth roll-off, and **dbx Type II** noise-reduction pumping.

![Memphis Dust UI](docs/screenshot.png)

| # | Section | What it does | Key DSP |
|---|---------|--------------|---------|
| 01 | **Sampler** | 12-bit / low-rate grit (SP-1200, S900, SP-202) | Fractional zero-order-hold decimation (2–48 kHz), mid-tread quantiser with full-scale hard clip (4–16 bit). No reconstruction filter, so you get real aliasing. Optional anti-alias pre-filter for the smoother S900 sound. Switch it **off** for a pure-analog Portastudio sound. |
| 02 | **Cassette EQ** | Head bandwidth + chassis colour | 12 dB/oct low cut (default 40 Hz, *before* the tape so the subs don't bog the head down) → "box" bell (150–600 Hz) → **3.5 kHz dip** (default −1.5 dB, softens digital harshness) → 24 dB/oct high cut (default 11 kHz) |
| 03 | **Tape** | Heads, transport, hiss | 2× oversampled biased-tanh head saturation with pre-/de-emphasis, so highs compress first (the shelves are exact inverses). Wow (0.55 Hz + random drift) and flutter (6.5 + 11.3 Hz + noise) through a Hermite-interpolated delay. **Dropouts**: random level and HF losses from worn oxide. Pink **hiss**, calibrated so the knob reads RMS dBFS, with RC-20-style **Hiss Duck** (dips under hits and breathes in the gaps). |
| 04 | **dbx II NR** | Compander pumping | A real 2:1 encoder before the tape and a 1:2 decoder after it, with pre-/de-emphasis and HF-weighted RMS detectors. With Breath at 0, encode→decode is transparent (tested to 0.00 dB), so the pumping comes from what the tape adds in between: hiss, saturation and dropouts get expanded, just like on the deck. **Breath** = decoder mistracking (steeper expansion, faster decoder release): transients jump, tails and hiss duck hard, cymbals swish. **Tilt** = dynamic de-emphasis mismatch: highs brighten on hits and darken as they decay. |
| 05 | **808 Clip** | Mono, clipped sub that still shows up on phones and car speakers | LR4 crossover (flat when recombined) → lows summed to mono → tanh/hard-clip blend → 24 dB/oct LPF at the same frequency (default 120 Hz) → added back to the untouched highs |
| 06 | **Bus Comp** | 1176-style slam (**FET**) or SSL-style glue (**VCA**) | **VCA** mode: 5 ms RMS detector, wide 10 dB knee, no colour, for the master-bus "4:1 / 10 ms / 100–200 ms / 3–5 dB" glue. **FET** mode: stereo-linked peak detector, 20 µs–30 ms attack, program-dependent release, 4/8/12/20:1 + **ALL** (all-buttons), FET colour that scales with gain reduction, parallel mix. The meter marks the 6–10 dB "Memphis zone". For the dbx-style pump, use 4:1, 3–5 ms attack, 80–150 ms release and 4–6 dB of GR. |

| 07 | **Master** | Underground-tape mastering utilities | **Mono Bass**: LR4 split, everything below 40–300 Hz (default 120) summed to true mono, with no distortion. **Limiter**: 1.5 ms lookahead brickwall (sliding-min gain + box-filtered attack, so it never overshoots). **Push** drives into it, **Ceiling** defaults to −0.3 dBFS. The meter shows limiter GR next to the comp GR. |

**Signal flow:** `IN → 07 mono bass → 01 → 02 → [04 encode → 03 tape (sat → wow/flutter → dropouts → hiss) → 04 decode] → 05 → 06 → dry/wet → Auto Level → output gain → soft clip → 07 limiter`

The 808 clipper runs *before* the compressor so the bus pumps against the knock. Hiss sits in front of both the dbx decoder and the comp, so it breathes twice.

### Globals
- **Input**, **Dry/Wet** (latency-compensated), **Output**
- **CLIP**: a soft clipper at 0 dBFS (the "mixer pushed into the red" sound). It also acts as a safety ceiling.
- **AUTO LEVEL**: slowly matches the processed loudness to the dry input (±12 dB, ~1 s), so bypass comparisons judge tone rather than volume. It only adapts while program material is present, so it doesn't pull the hiss up in silence.
- **Tape input meter**: peak level hitting the tape, with the **−12…−10 dBFS sweet spot** shaded. Use **Input** to make transients tickle the green zone. Amber means saturating, red means squish.

### Factory presets
`Memphis '94 Bus` (default) · `Portastudio Slam` · `SP Crunch (26k/12)` · `8-Bit Basement` · `808 Car Knock` · `Parallel Smash` · **`Portastudio 424`** (your RC-20 / stock-chain recipe: no digital stage, 45 Hz–11.5 kHz, +2 dB @ 250 Hz, −1.5 dB @ 3.5 kHz, flutter 40 / wow 25 / dropouts 15 %, ducked hiss, 4:1 / 3 ms / 120 ms comp) · **`dbx Breather (mistracked)`** · **`Dubbed Tape Master`** (gentle, for the master bus) · **`Memphis Master Bus`** (the 6-step master chain: HPF 32 Hz + mono < 120 Hz → +3 dB tape → 12 kHz LPF, wide +1.75 dB @ 300 Hz, narrow −1.5 dB @ 3.5 kHz → VCA 4:1 / 10 ms / 150 ms → 0.6 Hz wow → brickwall at −0.3 dBFS) · `Clean (Init)`
All presets are gain-staged to land within about ±2 dB of the input loudness.

## Build

You need CMake ≥ 3.22 and a C++17 compiler (Xcode on macOS, Visual Studio 2022 on Windows, gcc/clang on Linux).

```bash
cd memphis-dust
cmake -B build -DCMAKE_BUILD_TYPE=Release          # downloads JUCE 8.0.4 automatically
# or: cmake -B build -DJUCE_DIR=/path/to/JUCE      # use a local JUCE checkout
cmake --build build --config Release -j
```

Builds end up in `build/MemphisDust_artefacts/Release/{VST3,AU,Standalone}/`.
To install, copy the `.vst3` into your plugin folder:
- macOS: `~/Library/Audio/Plug-Ins/VST3` (AU: `~/Library/Audio/Plug-Ins/Components`)
- Windows: `C:\Program Files\Common Files\VST3`
- Linux: `~/.vst3`

Or set `COPY_PLUGIN_AFTER_BUILD TRUE` in `CMakeLists.txt` to have the build install it for you.

Linux needs these packages: `libasound2-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxcomposite-dev libfreetype-dev libfontconfig1-dev libgl1-mesa-dev`.

### Tests
`tests/DspTests.cpp` runs 32 numerical checks on the DSP stages without JUCE: quantiser levels, EQ response, exact pre-/de-emphasis inverse, the clean W/F delay, hiss calibration and ducking, mono sub, compressor gain reduction, dbx transparency / 2:1 encoding / noise reduction / breathing, dropouts, mono bass, VCA glue, and that the limiter never exceeds its ceiling.
```bash
cmake --build build --target dsp_tests && ./build/dsp_tests
```

## Tips
- **Put it on the drum bus.** For the full blueprint, put a second instance on the 808 with only 05 (and maybe 03) switched on.
- Rate **26.04 kHz / 12 bit** is the SP-1200. **22.05 kHz / 12 bit** is the default Memphis grit. **11 kHz / 8 bit** is the Basement crunch.
- Push the comp until the meter sits in the shaded 6–10 dB zone, then raise Hiss until you can just hear it breathe between hits.
- With dbx on, hiss gets ~17 dB quieter in the gaps (that's what NR does), so set Hiss higher (−55…−45) and use **Breath** to choose how hard it pumps.
- **Portastudio 424** on the drum bus, then **Memphis Master Bus** (or the softer **Dubbed Tape Master**) on the master bus, gives the cumulative "dubbed tape" sound. On the master, raise **Push** until the limiter readout shows 3–6 dB on the loudest hits.
- The window is resizable (50–150 %), and it remembers its size with your session.
- Latency is about 1 ms of oversampling filters + 3 ms of tape transport + 1.5 ms of limiter lookahead (264 samples at 48 kHz), and it is reported to the host, so PDC keeps everything aligned.
