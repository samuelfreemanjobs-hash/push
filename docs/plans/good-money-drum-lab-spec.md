# Good Money Drum Lab PRO — product spec (index)

**Status:** M1 playable foundation in `disklordz-ralph/good-money-drum-lab/`.  
**Stack:** JUCE 8 · C++20 · `ThreeLayerDrum` · separate product ID from Maybach / ship Lite.

## Positioning

- **Player Bounce:** rounded 808s, snappy percussion, Cardo-style warmth (original DSP).
- **Luxury Soul:** dusty minimal drums, Alchemist-**type** restraint (original DSP).
- Complements **Maybach Drum Lab** (cinematic luxury) and **Detroit Phonk Trap Maker** (ship).

## Eight DNA banks (implemented M1)

Good Money · Midnight Player · Orange Grove · Luxury Soul · Great Escape · Bay Area Funk · Night Drive · Premium Street.

## M1 shipped in repo

- `GoodMoneyIdentity.h`, `GoodMoneyPlayerMode.h`, `GoodMoneyKickForge.h`
- Offline renderer + CTest determinism
- Optional JUCE processor with DNA + mode APVTS

## Roadmap (from master prompt)

| Milestone | Focus |
|-----------|--------|
| M2 | Trunk Bass Reactor, Soul Snare Forge, clap/hat engines |
| M3 | Full three-layer UI + modulation |
| M4 | Good Money Pocket Engine + MIDI export |
| M5 | Minimalist Drum Intelligence |
| M6 | 8-profile DNA morph |
| M7 | Mutation factory |
| M8 | GUI (charcoal / forest / amber) |
| M9 | VOL.1 sample factory (500 one-shots) |
| M10 | pluginval + commercial QA |

## Commercial VOL.1 targets (not generated until M9)

500 synthesized one-shots, 120 MIDI grooves, 60 loops, 48 presets — see operator brief in chat / PR.

## Agent build directive

1. Inspect `good-money-drum-lab/` before duplicating oscillators.
2. Reuse `ThreeLayerDrum`; add personalities via data-driven headers.
3. No sample playback or Keygroups.
4. RT-safe `processBlock` when extending JUCE processor.
5. Run `factory/autoresearch` or lab renderer before marketing claims.

Full narrative master prompt is maintained in product design threads; this file is the **repo source of truth** for milestones.