# Gemini CLI — reference beat vs architecture

Prerequisite: place reference under `Assets/References/` (gitignored large media OK operator-side).

```text
Analyze @Assets/References/nola_bounce_ref.wav (and optional screen recording).

Extract: 808 decay (ms), sub weight vs click, hat roll rate (notes/bar), swing feel (%), rim syncopation notes.

Read disklordz-ralph/vst/TRAP-PHONK-ARCHITECTURE.md and Modules/TrapCoreDSP/ (Trap808Voice, RollEngine when present).

Output docs/plans/ref-analysis-YYYY-MM-DD.md with:
- Observed targets (numbers)
- Suggested APVTS defaults for glide/clip/swing (no code)
- Catch2 test ideas only

No automatic C++ patches.
```