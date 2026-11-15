# DetroitPhonkTrapMaker — trap / phonk / rap DSP & product architecture

Positioning: **beatmaker-first** (not acoustic drums). Compete on **speed, sub punch, built-in dirt, mix-ready kits**, and **WAV + VST bundle** — aligned with tripwire (`DLZ-DRUM-*`) and Instruments lane (`DLZ-VST-*`).

## Repo & toolchain

| Piece | Choice |
|-------|--------|
| Windows workspace | `C:\Dev\DetroitPhonkTrapVST3` (`disklordz-ralph/windows/bootstrap-DetroitPhonkTrapVST3.ps1`) |
| JUCE | 8.0.4 submodule `externals/JUCE`, **no FetchContent** |
| CMake baseline | Start from [Pamplejuce](https://github.com/sudara/pamplejuce) patterns: C++20, `pluginval` CI, `COPY_PLUGIN_AFTER_BUILD FALSE` |
| References | [awesome-juce](https://github.com/sudara/awesome-juce), [CTAG-JUCE-Sampler](https://github.com/NiklasWan/CTAG-JUCE-Sampler), [duck-sampler](https://github.com/jsvaldezv/duck-sampler), [chowdsp_utils](https://github.com/Chowdhury-DSP/chowdsp_utils) (permissive DSP utils) |
| Agent / REA | Point agents at **Release `.vst3`**, `DrumTestCLI` WAV output, and `Source/` — flag `processBlock` allocations, locks, file I/O |

## Source layout (`Source/`)

```text
Source/
  PluginProcessor.cpp/h      # APVTS, MIDI map, render threads
  PluginEditor.cpp/h         # Pads, genre, export, drag-drop
  DrumEngine.h                # Voice pool + genre routing (header-mostly)
  voices/
    Kick808Voice.h
    RimShotVoice.h
    HatVoice.h               # open/closed + choke group
    CowbellLaserVoice.h
  dsp/
    SoftClipper.h            # Fruity-style soft clip + tape saturation
    Glide808.h               # Mono portamento, pitch envelope
    HatChokeGroups.h           # closed cuts open
    RollEngine.h             # 1/16, 1/32, triplet from velocity/keyswitch
  export/
    WavRenderer.h              # 24-bit, background juce::Thread only
    KitExporter.h
  cli/
    DrumTestCLI.cpp            # Offline verify for Ralph
```

**Real-time rules:** preallocate voices + buffers in `prepareToPlay`. `processBlock` only: atomic parameter reads, `processSample` math, no `juce::String`, no disk.

## Must-have DSP (trap / phonk)

### 1. Soft clipper & saturation (P0)

Phonk/trap: harmonic density on kick/808 bus **with 2x oversampling** to limit aliasing on cowbell/808 harmonics.

Production starter: `starter-cpp/TrapSoftClipChain.h` (`WaveShaper` + `Oversampling`, prepare in `prepareToPlay` only).

**Beat competitors:** ROMplers ship macro “distortion” knobs; you expose **post-808 clip** tuned for FL clipping workflow.

### 2. Monophonic glide & pitch envelope (P0)

808 slides: `SmoothedValue<double, Multiplicative>` on **playback step** (pitch ratio), legato retrigger.

Production starter: `starter-cpp/Trap808Voice.h` + `TrapSamplerSound.h`.

MIDI: pitch wheel; **FL note:** wrapper pitch bend default 2 st — set **12 st** (`FL-STUDIO-QA.md`).

### 3. Hi-hat choke & roll engine (P1)

- **Choke:** voice group ID on hat voices; `noteOn(closed)` calls `killVoicesInGroup(openHatGroup)`.
- **Rolls:** `RollEngine` reads sidechain note or velocity tier → schedules sub-hits at 1/16, 1/32, triplet grid from host BPM (cache BPM in `processBlock` from playhead when available).

Reference: duck-sampler / CTAG choke patterns; keep logic in headers for Ralph iteration.

### 4. Sampler pads + user WAV (P1, beats XO/Battery on price)

- `juce::SamplerSound` + per-pad drag-drop (`FileDragAndDropTarget`) → load into preallocated `AudioBuffer` on **message thread**, swap pointer atomically for audio thread.
- Factory kits from **tripwire** (`kit-spec.yaml`); unlock folder of 24-bit WAVs in every purchase (your table row 4).

## Competitor wedge (what you ship)

| Segment | They win | DiskLordz wedge |
|---------|----------|-----------------|
| XO / Battery / Triaz | Visual sorting, deep layering | **$29–59**, curated Detroit/phonk kits, **no learning curve** |
| Kick 3 / SubLab | Sub sculpt | **808 + full kit in one UI**, phonk clip + glide baked in |
| UJAM / Raptor / Drum Weapons | Cheap presets | **User WAV on pads + export kit + tripwire updates** |
| BVKER / Cymatics WAV | Folder-only | **VST + same WAVs unlocked** (tripwire checkout) |

## Product tiers (ship order)

1. **Tripwire** — sample pack + light standalone renderer (`DLZ-DRUM-002`).
2. **Plugin v1** — 5 pads, genre sat, MIDI map, WAV export (`DLZ-VST-002`).
3. **Plugin v1.5** — glide, choke, rolls (`DLZ-VST-004`–`006`).
4. **FL polish** — drag to playlist, pitch bend doc, note-off QA (`DLZ-VST-007`).

Copy-protection: defer until revenue; JUCE **Starter** splash sub-$20k revenue, upgrade before $20k ([JUCE license](https://juce.com/legal/juce-8-licence/)).

## Commercial checklist

- [ ] JUCE tier vs annual revenue
- [ ] `pluginval` strictness 5 Windows + macOS smoke before sale
- [ ] FL Studio: MIDI off, pitch bend range, PDC at 64–512 buffer
- [ ] Windows code signing + Apple notarization before wide release
- [ ] EULA + royalty-free kit license in tripwire manifest

## Ralph story map

See `disklordz-ralph/prd.json` entries `DLZ-VST-004`–`DLZ-VST-008`. Each story = one DSP or UX module + CLI or pluginval subset.

## REA / agent workflow

1. Implement one module under `Source/dsp/`.
2. `cmake --build build --config Release`.
3. `DrumTestCLI.exe` → compare RMS/peak of golden WAV fixtures.
4. REA prompt: “Scan `processBlock` and voice render for allocations; list line numbers.”
5. `pluginval --strictness-level 5` before merging story in `prd.json`.