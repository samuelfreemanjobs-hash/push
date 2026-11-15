# Superpowers + DiskLordz VST

[Superpowers](https://github.com/obra/superpowers) turns the agent into spec → plan → TDD → review. Pair with **REA** (binary audit) and **Ralph** (one `prd.json` story).

## Install (Cursor, Windows)

1. Agent chat (**Ctrl+I**): `/add-plugin superpowers` or search Marketplace for **superpowers**.
2. Confirm session bootstrap mentions skill routing.
3. Keep project rules: `push/.cursor/rules/rea-juce-superpowers.mdc`, `push/.cursor/rules/ponytail-juce.mdc` (minimal diffs), + root `AGENTS.md`.

## Skills → your lane

| Superpowers skill | DiskLordz use |
|-------------------|---------------|
| `brainstorming` | Trap808 glide, clip, choke, drag-export specs |
| `writing-plans` | `docs/plans/*.md` before CMake/C++ |
| `test-driven-development` | Catch2 + `DrumTestCLI` before `trap-core-dsp/` |
| `subagent-driven-development` | Parallel: clip / glide / export worktrees |
| `systematic-debugging` | FL MIDI / pitch bend; then REA §3C |

## DRY: TrapCoreDSP

One static library for all plugins and genre skins:

- `Modules/TrapCoreDSP/` — shared glide, clip, choke, sample loader (headers in `Source/`)
- Manifest-driven kits — `kit-spec.yaml` / JSON; no duplicate voice classes per genre

## Sample pack pipeline (TDD)

Plan with Superpowers, implement CLI under `tools/validate-kit/` or reuse `openclaw/drum-kit-factory/scripts/validate-kit-spec.mjs` for YAML; extend with WAV loudness/zero-cross tests in Catch2 or Python.

## Starter prompts

**Plan 808 glide**

```text
Use Superpowers brainstorming then writing-plans for monophonic 808 glide with
SmoothedValue Multiplicative pitch ratio and legato. Output docs/plans/808-voice-spec.md
aligned with trap-core-dsp/Trap808Voice.h — no code until plan is saved.
```

**TDD glide**

```text
Use test-driven-development: add failing Catch2 test_pitch_glide_curve in tests/;
then implement minimal Trap808Voice until green. Zero heap allocations in render path.
```

**Ship gate**

```text
Run ctest -C Release, pluginval strictness 5, save REA rt-safety-latest.md;
spec-compliance review against docs/plans/808-voice-spec.md.
```