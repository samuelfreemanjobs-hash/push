# Autoresearch playbook — DiskLordz drum voice sandbox

You are an **overnight DSP autoresearch agent**. Your job is to lower `eval_loss` in `results.tsv` by editing **one file only**.

## Goal (default run)

Minimize weighted **eval loss** when synthesizing a **Southern trap 808** at 49 Hz (C1-ish), MIDI velocities **32, 64, 100, 127**, sample rate **48000 Hz**, while:

- Render time for the full harness stays **under 60 ms** (Python offline harness; **5 ms** budget applies only after C++ port — see `docs/dev/autoresearch-cpp.md`).
- Peak stays **≤ 0.999 linear** (no digital clip penalty).
- **No NaN/Inf**, **|DC| < 1e-4** after high-pass check.

Optional timbre anchor: if `targets/reference_808.wav` exists, minimize multi-resolution **STFT magnitude distance** to that file. If missing, optimize **band-energy profile** toward sub-heavy BR 808 (see `evaluate.py` → `PROFILE_808`).

## Files you may edit

| Allowed | Forbidden |
|---------|-----------|
| `drum_engine.py` | `evaluate.py`, `program.md`, `run_harness.py`, anything under `../` |

## Files you read

- `program.md` (this file)
- `results.tsv` — past runs; **never** duplicate a failed hypothesis without change
- `drum_engine.py` — current synthesis
- Latest line of `results.tsv` for `best_loss`

## Harness command (do not modify)

```bash
cd disklordz-ralph/factory/autoresearch
python run_harness.py
```

Prints one line: `eval_loss=<float>` plus JSON metrics on stderr.

## Git discipline (Karpathy-style)

1. Before first edit: `git status` must be clean on `drum_engine.py` or commit baseline.
2. Run harness. Parse `eval_loss`.
3. **If** `eval_loss < best_loss` in `results.tsv` **and** `safety_pass=1`:
   - Append row to `results.tsv`
   - `git add drum_engine.py results.tsv`
   - `git commit -m "autoresearch: loss <value> — <one-line hypothesis>"`
4. **Else** (worse loss, crash, or safety fail):
   - `git checkout -- drum_engine.py`
   - Log failure row with `kept=0` (append only, do not commit code)
5. Hypothesize again. Target **50–100** iterations per session; small diffs beat rewrites.

## Guardrails

- **No new dependencies** in `drum_engine.py` beyond `numpy`.
- **No file I/O** in `render_batch()` — evaluate owns I/O.
- **No randomness** unless `seed` argument is honored (deterministic renders).
- Do **not** claim real-time JUCE readiness from Python scores; C++ port is a separate milestone (`docs/dev/autoresearch-cpp.md`).
- Respect `STACK-FROZEN.md`: autoresearch does **not** replace `TrapCoreDSP` until a human merges winners with Catch2.

## Hypothesis examples (good)

- "Increase pitch sweep from 110→49 Hz in 18 ms; reduce THD by easing tanh drive at velocity < 80."
- "Add 2-pole one-zero emphasis at 2.2 kHz on attack layer only; tune crest without raising sub THD."
- "Asymmetric clip: softer positive knee to preserve 55 Hz fundamental at vel 127."

## Hypothesis examples (bad)

- "Rewrite entire engine as neural net."
- "Copy TR-808 ROM tables from the internet."
- "Edit evaluate.py to return 0.0."

## When to stop

- `best_loss` flat for **20** runs → change metric weighting in a **human** PR to `evaluate.py`, not in-agent.
- Loss improves but sounds wrong by ear → operator drops a new `targets/reference_808.wav` from **their** TrapCoreDSP render and restarts.