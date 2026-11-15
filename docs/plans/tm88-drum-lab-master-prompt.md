# DISKLORDZ TM88 DRUM LAB PRO — master build prompt

**Legal:** Independent DiskLordz Audio product. **Original synthesis only.** Not an official TM88 / 808 Mafia collaboration, endorsement, or recreation of proprietary sounds.

**Ship priority:** `vst/starter-cpp/` (Detroit Phonk Trap Maker) remains first. TM88 lab is **R&D** under `disklordz-ralph/tm88-drum-lab/`.

**M1 code:** `tm88-drum-lab/MILESTONE-1-STATUS.md`

---

## Product thesis

Dark melodic trap · emo trap · space trap · experimental 808s — emphasis on **808 movement**, **rhythmic space**, and **percussion placement** (vs South/Luger impact focus).

Proposed commercial kit: **NIGHTMARE VOL. 1** — 500 synthesized one-shots + MIDI/loops (M8 factory).

---

## Six sound DNA banks (editable profiles)

| ID | Profile | Character |
|----|---------|-----------|
| 01 | **Nightmare 88** | Dark 808s, hollow snares, ominous digital percussion |
| 02 | **Emo Trap** | Tuned glides, crisp hats, spacious claps |
| 03 | **Space Mafia** | FM lasers, metallic hits, drifting texture |
| 04 | **56 Nights** | Hard kicks, aggressive 808 motion, rolling hats |
| 05 | **Crash Dummy** | Distorted digital, experimental accents |
| 06 | **After Hours** | Minimal, deep sub, sparse percussion |

Implemented M1: `dl::tm88::designForTm88()` in `Tm88Identity.h`.

---

## Engine map (M2–M8)

1. **88 Bass Reactor** — glide, sub/harmonic split, movement (M3)  
2. **Dark Kick Forge**  
3. **Emo Trap Snare Designer**  
4. **Clap Factory**  
5. **Precision Hat Lab** — rolls, probability, choke  
6. **Space Percussion Forge** (reuse Percussion Forge / FM lib)  
7. **Dark Atmosphere FX**  
8. **88 DNA Morph** (Hyper DNA integration, M4)  
9. **88 Groove Intelligence** + **Rhythmic Space** + **Percussion Priority** (M5) — signature differentiator  
10. **Mutation Factory** (M6)  
11. **Automated Sample Factory** — 500 WAV manifest (M8)  
12. **GUI** — gunmetal / midnight purple / violet (M7)

---

## Integration rule

Reuse `factory/drum-architecture` three-layer core. **Do not** fork a second allocator-heavy engine. New behavior = recipes + modules + presets.

---

## Real-time rules (all milestones)

- No heap / locks / disk / network in `processBlock`  
- `ThreeLayerDrum::next()` RT-safe; `renderDesign()` offline only  
- 32-voice polyphony target; pluginval before any paid SKU  

---

## Milestone checklist

| M | Scope | Status |
|---|--------|--------|
| M1 | CMake, 6 DNA, kick/snare/808, tests, offline WAV, optional JUCE shell | **Done** (portable core) |
| M2 | Seven instrument engines (stub → full params) | Planned |
| M3 | Emotional glide, sub split, 808 movement | Planned |
| M4 | DNA morph surface | Planned |
| M5 | Sequencer, rhythmic density, percussion priority | Planned |
| M6 | Mutation + breeding | Planned |
| M7 | Production GUI | Planned |
| M8 | NIGHTMARE VOL. 1 factory + ZIP | Planned |
| M9 | Commercial QA, pluginval 8 | Planned |

---

## Cursor / Claude execution directive

1. Read `tm88-drum-lab/` and `luger-drum-lab/` patterns before new code.  
2. Implement **one milestone per PR** with tests.  
3. Do not claim Windows/JUCE validation until operator runs gates.  
4. For full-stack prompts (MetaGPT portal, OpenMontage, Strix), use existing `docs/dev/*` playbooks.

---

## South vs TM88 positioning

| South / Luger lab | TM88 lab |
|-------------------|----------|
| Aggressive low-end impact | Expressive low-end **movement** |
| Hard transients | Dark, emotional percussion |
| Punch / distortion | Glide, rhythm, atmosphere |
| Dense patterns | Sparse patterns + detailed accents |
| Kick/808 clash tools | 808 melody + percussion interplay |