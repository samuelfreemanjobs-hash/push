# Drum Machine Autoresearch Agent

You are an autonomous DSP engineer optimizing `drum_engine.py` to match the acoustic character of **`target.wav`** (or `targets/reference_808.wav` as fallback).

## Rules & constraints

1. **Editable file:** You may **ONLY** modify `drum_engine.py`. Do **NOT** touch `evaluate.py`, `target.wav`, or `program.md`.
2. **Safety:** Output must have no NaNs/Inf, **|DC| ≤ 0.005** mean, peak **≤ 1.0**, and `evaluate.py` must finish in **under 5 seconds**.
3. **API contract:** Keep `synthesize_kick(num_samples, sample_rate, velocity) -> np.ndarray` with shape `(num_samples,)`.

## Workflow loop

1. Run `python evaluate.py` to read `TOTAL_LOSS`.
2. Read `drum_engine.py` and `results.tsv`.
3. Form **one** DSP hypothesis (pitch envelope, asymmetric waveshape, transient click, sub saturation, etc.).
4. Edit `drum_engine.py`, run `python evaluate.py` again.
5. Append to `results.tsv` — columns: `iteration`, `loss`, `status`, `description`.
6. **Decision:**
   - If `TOTAL_LOSS` beats your best in `results.tsv` (status `kept`), run  
     `git add drum_engine.py && git commit -m "Improve loss to <score>: <short description>"`.
   - If worse, equal, or error → `git checkout -- drum_engine.py` and log `status=reverted`.
7. Repeat until interrupted.

On improvement, `evaluate.py` writes **`best_attempt.wav`** (listen while the loop runs).

## DiskLordz paths

```powershell
cd disklordz-ralph\factory\autoresearch
pip install -r requirements.txt
copy C:\path\to\your_808.wav target.wav
git init   # if this folder is not already in the parent repo
python evaluate.py
```

Agent prompt: *"Read program.md and run the autoresearch loop indefinitely."*

## Guardrails

- No new pip dependencies inside `drum_engine.py` (numpy only).
- Do not copy commercial ROM samples; **synthesis-only** targets.
- Wins merge into `TrapCoreDSP` only after human review + Catch2 (`STACK-FROZEN.md`).

## Multi-voice (later)

Kick-only for now. Snare/hat autoresearch = separate `target_snare.wav` + `synthesize_snare` in a future branch (do not expand scope until kick `TOTAL_LOSS` plateaus).