# C++ autoresearch (JUCE headless + pluginval)

| File | Role |
|------|------|
| `VoiceDSPSandbox.h` | **Agent sandbox** |
| `HeadlessMain.cpp` | WAV render CLI (not agent-edited) |
| `evaluate_vst.py` | Build + pluginval + acoustic loss |
| `program_cpp.md` | Agent playbook |

## Windows setup

```powershell
cd disklordz-ralph\vst\starter-cpp
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DDLZ_AUTORESEARCH=ON
cmake --build build --config Release

# target kick (shared with Python kit autoresearch)
copy ..\..\factory\autoresearch\targets\kick.wav .\..\autoresearch-cpp\targets\kick.wav

cd ..\autoresearch-cpp
python evaluate_vst.py
```

Env:

| Variable | Default |
|----------|---------|
| `DLZ_BUILD_DIR` | `starter-cpp/build` |
| `PLUGINVAL_STRICTNESS` | `5` (use `8` pre-release) |
| `DLZ_MAX_RENDER_MS` | `15` |
| `MIN_SPECTRAL_CONVERGENCE` | `0.08` originality floor |
| `DLZ_PLUGINVAL` | path to `pluginval.exe` |

## Python kit loop (same business)

`factory/autoresearch/` — multi-voice kits.  
This folder — **VST DSP sandbox** before TrapCoreDSP merge.

Full stack: `docs/dev/hybrid-agent-stack.md`.