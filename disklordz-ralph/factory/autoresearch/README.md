# DiskLordz Autoresearch (Python)

Karpathy-style loop: edit **`drum_engine.py`** only → **`python evaluate.py`** → git commit or revert.

## Minimal folder (standalone or in repo)

| File | Role |
|------|------|
| `target.wav` | Reference kick/808 (you provide; gitignored) |
| `program.md` | Agent system prompt |
| `evaluate.py` | Immutable judge → `TOTAL_LOSS` |
| `drum_engine.py` | Agent sandbox |
| `results.tsv` | Experiment log |

Also supported: `targets/reference_808.wav` if `target.wav` is absent.

On each **improving** `TOTAL_LOSS`, `evaluate.py` writes **`best_attempt.wav`** for listening.

## Quick start

```powershell
cd disklordz-ralph\factory\autoresearch
pip install -r requirements.txt
copy C:\exports\my_kick.wav target.wav
python evaluate.py
```

```
RENDER_TIME: 0.0412s
SPECTRAL_LOSS: 1.234567
ENVELOPE_LOSS: 0.045678
TOTAL_LOSS: 1.462957
```

## Agent launch

```
Read program.md and start running the autoresearch loop indefinitely.
```

Optional wrapper:

```powershell
python run_harness.py --log --status kept --description "asymmetric tanh knee"
```

## Multi-voice

**Kick only** in this template. Snare/hat = separate targets + functions later (`synthesize_snare`, etc.).

## C++ port

`docs/dev/autoresearch-cpp.md` — same playbook, offline JUCE render, 5 ms RT budget.