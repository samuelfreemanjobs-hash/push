# DiskLordz Autoresearch (Python)

Karpathy-style **edit → score → keep/revert** loop for **offline** drum voice math. Fits **Drum Forge** and feeds winners into `TrapCoreDSP` manually (Catch2 required).

## Layout

| File | Role |
|------|------|
| `drum_engine.py` | **Agent sandbox** (only editable file) |
| `evaluate.py` | **Immutable judge** — loss + safety |
| `program.md` | Agent system prompt |
| `run_harness.py` | CLI + optional `results.tsv` log |
| `results.tsv` | Experiment ledger |

## Quick start (Windows)

```powershell
cd disklordz-ralph\factory\autoresearch
pip install -r requirements.txt
python run_harness.py
```

With reference timbre:

```powershell
copy C:\path\to\your_808_render.wav targets\reference_808.wav
python run_harness.py
```

## Agent session (Cursor / Claude Code)

1. Open `program.md` as the task instruction.
2. Loop: edit `drum_engine.py` → `python run_harness.py` → compare `eval_loss` to `best_loss` in `results.tsv`.
3. On win: `git add drum_engine.py results.tsv` + commit; on loss: `git checkout -- drum_engine.py`.

Log a row:

```powershell
python run_harness.py --log --kept 1 --hypothesis "Asymmetric clip knee for vel 127"
```

## Loss ingredients

See `evaluate.py`: STFT or profile timbre, crest/attack targets, THD on fundamental, DC/peak/time penalties.

## C++ / JUCE path

Real-time plugin autoresearch is **phase 2**: `docs/dev/autoresearch-cpp.md` — render harness + immutable C++ judge, same `program.md` discipline.

**STACK-FROZEN:** do not block Lite VST ship on autoresearch merges.