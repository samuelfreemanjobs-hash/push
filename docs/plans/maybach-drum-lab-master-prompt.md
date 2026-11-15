# DISKLORDZ MAYBACH DRUM LAB PRO — master development prompt

Luxury hip-hop · cinematic soul · orchestral trap · live-feeling drum synthesis.

**Legal:** Original DiskLordz sound banks only. Not affiliated with any artist or label. No commercial samples or extracted recordings.

**Codebase:** `disklordz-ralph/maybach-drum-lab/` — M1 landed; see `MILESTONE-1-STATUS.md`.

---

## Mission

Build **MAYBACH DRUM LAB PRO**: warm, expensive, musically composed drums (orchestral + soul + trap) via physical-modeling-inspired **BODY / ATTACK / TEXTURE** (`ThreeLayerDrum`), luxury room (M4), groove engine (M6), and **Luxury Drum Collection Vol. 1** factory (M9: 500 one-shots).

Reuse: `factory/drum-architecture/`, Percussion Forge, Hyper DNA patterns from other labs — **do not duplicate** working DSP.

---

## Six luxury identities (implemented M1)

| # | Identity | Character |
|---|----------|-----------|
| 01 | Maybach Soul | Warm kicks, thick snares, rich claps |
| 02 | Cinematic Empire | Massive bass drums, dramatic tails |
| 03 | Luxury Trap | Deep 808s, polished claps |
| 04 | Yacht Club | Soft transients, R&B pocket |
| 05 | Teflon Drums | Hard street kicks, analog color |
| 06 | Royal Orchestra | Timpani-like tails, metallic swells |

**Signature axis:** `AcousticMorph` **Live → Hybrid → Trap** (`MaybachAcousticMorph.h`).

---

## Flagship modules (priority)

1. **Luxury Kick Designer** — M2 (`luxuryKickForPreset` started M1)
2. **Live Snare Forge** — M2–M3 (modal + wire network)
3. **Luxury Room Engine** — M4 (FDN / early reflections)

Then: Cinematic 808, Clap Factory, Cymbal model, Orchestral percussion, Soul percussion, Cinematic impacts.

---

## Three-layer synthesis

Independent **BODY** (resonators / 808 core), **ATTACK** (exciters), **TEXTURE** (noise + modes). Per-voice state in `ThreeLayerDrum::next()` — RT-safe in plugin; alloc only in `renderDesign()` offline.

---

## Sample pack target (M9)

**DISKLORDZ LUXURY DRUM COLLECTION — VOL. 1**

- 500 one-shots (kicks, 808s, snares, claps, hats, orchestral, soul, impacts, vintage hits)
- 100 MIDI grooves, 50 loops, stems, 40 kit presets, MPC layouts, JSON manifests, commercial ZIP

Integrate with `package_release.py` and `export_kit.py` patterns (`-0.3 dBFS`, 24-bit/48 kHz default).

---

## Milestones

| M | Scope |
|---|--------|
| 1 | CMake, identities, morph, kick/snare/clap, tests, offline render, optional JUCE |
| 2 | Eight instrument engines (DSP modules) |
| 3 | Modal / membrane / velocity articulation |
| 4 | Luxury Room Engine |
| 5 | DNA morph UI + automation |
| 6 | Live Groove Engine |
| 7 | Mutation Factory |
| 8 | Premium GUI (champagne gold / matte black) |
| 9 | Sample factory batch + ZIP |
| 10 | pluginval 8, QA metrics, commercial validation |

---

## RT rules

- ≥32 voices, no alloc/lock/file I/O in `processBlock`
- Export/render on background thread
- Gain-matched audition presets (no “louder wins” bias)

---

## Cursor / Claude execution

```text
Read docs/plans/maybach-drum-lab-master-prompt.md and maybach-drum-lab/MILESTONE-1-STATUS.md.
Work one milestone. Run MaybachMilestone1Tests after DSP edits.
Do not modify vst/starter-cpp ship path without human review.
```

Full product brief (engines §5–§22, GUI §20) lives in product planning threads; implement in milestone order above.