# DiskLordz drum kit autoresearch

Multi-voice Karpathy loop under `disklordz-ralph/factory/autoresearch/` (same role as `drum-autoresearch/` in tutorials).

```
factory/autoresearch/
├── targets/
│   ├── kick.wav
│   ├── snare.wav
│   └── hat.wav
├── outputs/
│   └── best_kit.wav      # on new best TOTAL_LOSS
├── program.md
├── evaluate.py           # immutable judge
└── drum_engine.py        # agent sandbox
```

## Quick start

```powershell
pip install -r requirements.txt
# add the three targets (see targets/README.md)
python evaluate.py
```

## Outputs

| Metric | Meaning |
|--------|---------|
| `KICK_LOSS` / `SNARE_LOSS` / `HAT_LOSS` | Per-voice spectral + envelope |
| `TOTAL_LOSS` | Sum of the three |
| `outputs/best_kit.wav` | Audition loop when total improves |

## Velocity testing (optional)

```powershell
$env:KIT_VELOCITY_TEST=1
python evaluate.py
```

Blends full-velocity loss with hits at MIDI **~30, 80, 127** (as velocity 0.24, 0.63, 1.0).

## C++ / VST bridge

See `docs/dev/autoresearch-cpp.md` — promote winning constants into `TrapCoreDSP`; agents do not edit JUCE in the loop.

## RAGFlow (optional)

Visual loop + RAG over DSP manuals: `docs/dev/autoresearch-ragflow.md`.  
SSH runner on your machine + `tools/ragflow_persist_win.py` webhook. **CLI remains primary** for overnight Git iterations.