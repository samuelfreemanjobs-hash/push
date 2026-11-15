# VST lane (DetroitPhonkTrapMaker)

Bootstrap Windows tree:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\disklordz-ralph\windows\bootstrap-DetroitPhonkTrapVST3.ps1
```

Map to business `prd.json`:

| Ralph | Old US-* |
|-------|----------|
| DLZ-VST-001 | US-001 CMake + minimal plugin |
| DLZ-VST-002 | US-002–US-006 engine, CLI, UI, pluginval |
| DLZ-VST-003 | US-007 drag-and-drop WAV |

While iterating VST, also obey the JUCE block in `disklordz-ralph/PROMPT.md`. On pass, update **both** local `prd.json` (if present) and `disklordz-ralph/prd.json` in the push repo.