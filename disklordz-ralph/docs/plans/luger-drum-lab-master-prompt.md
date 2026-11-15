# DISKLORDZ LUGER DRUM LAB — Master JUCE 8 / C++20 build prompt

> **Repo placement:** Genre engine inside **DiskLordz Drum Design Lab**, not a second ship product.  
> **Stack gate:** `STACK-FROZEN.md` — finish **Detroit Phonk Trap Maker Lite** before marketing or selling this SKU.  
> **Reuse:** `factory/drum-architecture/Source/{CinematicDSP.h,ThreeLayerDrum.h}`, `factory/cinematic-dsp/` optional JUCE shell, `vst/Modules/TrapCoreDSP/` for pad/glide ideas only.

Copy the block below into **Cursor or Claude Code** when starting a Luger-focused sprint (after M1 in `disklordz-ralph/luger-drum-lab/` is green on Windows).

---

## Paste-from-here (agent prompt)

```
DISKLORDZ LUGER DRUM LAB — MASTER AI CODING AGENT PROMPT — JUCE 8 / C++20

YOUR ROLE
You are an expert C++20 audio DSP engineer, JUCE 8 plugin developer, synthesizer designer,
drum-machine architect, and professional sample-library developer.

Build DISKLORDZ LUGER DRUM LAB: a production-grade, original drum synthesis instrument and
automatic drum sample factory inspired by explosive orchestral Southern trap aesthetics
(early 2010s production *characteristics only* — no samples from existing records).

Targets: Windows VST3, Standalone, offline WAV factory, MIDI instrument, sequencer (later milestones).
Stack: C++20, JUCE 8, CMake.

Before writing new code:
1. Read `disklordz-ralph/luger-drum-lab/README.md` and `MILESTONE-1-STATUS.md`.
2. Reuse `factory/drum-architecture/Source/ThreeLayerDrum.h` — do not fork Body/Attack/Texture.
3. Do not delete `TrapCoreDSP`, CinematicDSP, or Detroit Phonk Trap Maker sources.
4. Implement real C++; run `ctest` on Milestone 1 before claiming VST3 readiness.

SONIC DNA (5 recipes — original DiskLordz, not impersonation)
- BRICK SQUAD — explosive kick, heavy 808, hard snare, bright hats, punchy LF
- ORCHESTRAL WARFARE — cinematic kicks, synthetic timpani-like hits, dramatic impacts
- TRAP ANTHEM — clean power kicks, layered claps, fast hats, tight energy
- DARK SOUTHERN — deep sub kicks, gritty 808, dark snares, heavy rims
- ARENA TRAP — max attack, wide percussion, long cinematic tails, explosive fills

CORE: three-layer synthesis (Body / Attack / Texture) per existing `DrumDesign`.
Seven flagship engines (M2+): Luger Kick Cannon, Southern 808 Reactor, Snare Explosion,
Trap Clap Forge, Hi-Hat Warfare, Orchestral Percussion Forge, Cinematic FX Engine.

DNA morph (M4): BRICK → ORCHESTRAL → ANTHEM → DARK → ARENA on shared parameters; respect locks.

Factory target pack: DISKLORDZ LUGER DRUM LAB — WAR MACHINE VOL. 1 (450 one-shots + MIDI — M7).

MILESTONE 1 (do first)
- CMake + tests under `disklordz-ralph/luger-drum-lab/`
- `LugerIdentity.h` morph modifiers on `DrumDesign`
- Deterministic offline render for Kick, Snare, Bass808 × 5 identities
- Optional: extend `cinematic-dsp` JUCE processor with `lugerIdentity` APVTS param (OFF by default in Lite ship week)

RT rules: no alloc/locks in `processBlock`; Ponytail full on DSP; Superpowers owns failing tests first.

See full specification in git: disklordz-ralph/docs/plans/luger-drum-lab-master-prompt.md (appendix).
```

---

## Appendix — full specification (operator reference)

The user-supplied master prompt (parts 1–14) is preserved in version control as the canonical product spec. Milestone breakdown:

| Milestone | Scope |
|-----------|--------|
| **M1** | Buildable foundation — kick/snare/808 via ThreeLayer + Luger DNA, tests (`luger-drum-lab/`) |
| **M2** | Seven flagship engines + shared layers |
| **M3** | Filters, saturation, oversampling, dynamics, QA meters |
| **M4** | Five-point Luger DNA morph with locks |
| **M5** | Seeded mutation factory |
| **M6** | 16–64 step sequencer + groove families |
| **M7** | War Machine Vol. 1 batch export + manifest ZIP |
| **M8** | Crimson/graphite JUCE UI (not Web mock) |
| **M9** | pluginval, DAW matrix, repeatability report |

**Differentiator called out in brief:** Orchestral Percussion Forge — synthesized war drums, timpani-like hits, military snares, impacts (no orchestral sample libraries).

**Platform fit:** Same core as Cinematic Shadows / Hyper DNA; Luger is an installable **genre bank** + factory preset, not a parallel monorepo.

For the verbatim long-form sections (engines 01–07, QC tables, UI screen list, agent rules), extend this file in a follow-up commit or paste from your operator doc — the execution entry point for agents is the **Paste-from-here** block plus `luger-drum-lab/README.md`.