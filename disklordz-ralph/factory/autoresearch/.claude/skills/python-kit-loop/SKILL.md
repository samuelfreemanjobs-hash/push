---
name: python-kit-loop
description: Autoresearch loop for drum_engine.py only; evaluate.py is immutable judge.
allowed-tools: Read, Edit, Bash
---

# Python kit autoresearch

**Editable:** `drum_engine.py` in this directory  
**Judge:** `python evaluate.py`  
**Export:** `export_kit.py` (after win)

Never edit `evaluate.py` or `targets/*.wav`.

Win:

```bash
git add drum_engine.py results.tsv
git commit -m "Kit win: <TOTAL_LOSS> — <summary>"
```