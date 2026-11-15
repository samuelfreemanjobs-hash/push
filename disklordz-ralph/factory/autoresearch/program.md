# Multi-Voice Drum Kit Autoresearch Agent

You are an autonomous DSP engineer optimizing `drum_engine.py` to match:

- `targets/kick.wav`
- `targets/snare.wav`
- `targets/hat.wav`

(Legacy: `target.wav` counts as kick only if `targets/kick.wav` is absent.)

## Rules & constraints

1. **Editable file:** **ONLY** `drum_engine.py`. Do **NOT** touch `evaluate.py` or anything in `targets/`.
2. **Safety:** Each voice: no NaN/Inf, **|DC| ≤ 0.005**, peak **≤ 1.0**.
3. **Speed:** `evaluate.py` completes in **under 5 seconds** (set `KIT_VELOCITY_TEST=1` only when chasing velocity curves — slower).
4. **API:** Keep `synthesize_kick`, `synthesize_snare`, `synthesize_hat` with signature  
   `(num_samples, sample_rate, velocity) -> np.ndarray`.

## Synthesis strategy

- **Kick:** Sub phase continuity, pitch envelope, transient punch.
- **Snare:** Dual body oscillators + band-passed noise snap.
- **Hi-hat:** Detuned metallic bank + high-pass + short decay.

## Workflow loop

1. Run `python evaluate.py` → `TOTAL_LOSS`, `KICK_LOSS`, `SNARE_LOSS`, `HAT_LOSS`.
2. Attack the **highest** per-voice loss, or shared saturation/glue if balanced.
3. One hypothesis → edit `drum_engine.py` → `python evaluate.py`.
4. Log `results.tsv`: `iteration`, `total_loss`, `kick_loss`, `snare_loss`, `hat_loss`, `status`, `description`.
5. **If `TOTAL_LOSS` improves:** `git add drum_engine.py && git commit -m "Loss <score>: <description>"`.
6. **Else:** `git checkout -- drum_engine.py`, log `status=reverted`.
7. Repeat until interrupted.

New best scores write **`outputs/best_kit.wav`** (8-step demo pattern).

## DiskLordz setup

```powershell
cd disklordz-ralph\factory\autoresearch
pip install -r requirements.txt
mkdir targets, outputs 2>$null
copy C:\exports\kick.wav targets\kick.wav
copy C:\exports\snare.wav targets\snare.wav
copy C:\exports\hat.wav targets\hat.wav
python evaluate.py
```

Optional velocity layers: `$env:KIT_VELOCITY_TEST=1; python evaluate.py`

C++ export: `docs/dev/autoresearch-cpp.md` (offline render bridge — not agent-edited).