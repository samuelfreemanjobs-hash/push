# 808 voice spec (Superpowers / writing-plans seed)

**Status:** draft — extend via `brainstorming` in Cursor before implementation.

## Goal

Monophonic 808 sampler voice with multiplicative pitch glide, legato retrigger, 12-semitone pitch wheel.

## Acceptance (Catch2)

- Given two overlapping MIDI note-ons, pitch ratio reaches target within `glideSeconds` ± 5%.
- `renderNextBlock` produces no NaN/Inf on 512-sample buffer at 44.1 kHz.
- No allocation counted in voice render path (manual or REA audit).

## References

- `starter-cpp/Trap808Voice.h`
- `docs/FL-STUDIO-QA.md` pitch bend 12 st
- Ralph: `DLZ-VST-005`

## Out of scope (YAGNI)

- Polyphonic 808 stacks
- MPE zones