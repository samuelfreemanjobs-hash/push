---
name: cpp-voice-dsp-loop
description: Single-hypothesis edit loop for autoresearch VoiceDSP.h with immutable evaluate_vst.py judge.
allowed-tools: Read, Edit, Bash
---

# C++ voice DSP loop

**Editable:** `disklordz-ralph/vst/autoresearch-cpp/Source/VoiceDSP.h`  
**Judge:** `python disklordz-ralph/vst/autoresearch-cpp/evaluate_vst.py`  
**Never edit:** `evaluate_vst.py`, `HeadlessMain.cpp`, harness CMake.

RT rules: no heap in render path, `juce::ScopedNoDenormals`, originality `MIN_SPECTRAL_CONVERGENCE=0.08`.

Win commit (from repo root):

```bash
git add disklordz-ralph/vst/autoresearch-cpp/Source/VoiceDSP.h disklordz-ralph/vst/autoresearch-cpp/results.tsv
git commit -m "DSP win: <TOTAL_LOSS> — <summary>"
```