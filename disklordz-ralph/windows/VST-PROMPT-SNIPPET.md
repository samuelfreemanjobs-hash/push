# VST lane (DetroitPhonkTrapMaker)

Bootstrap Windows tree:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\disklordz-ralph\windows\bootstrap-DetroitPhonkTrapVST3.ps1
```

Architecture: `disklordz-ralph/vst/TRAP-PHONK-ARCHITECTURE.md`

Pamplejuce-style CI + `pluginval` recommended; JUCE 8 submodule in bootstrap.

| Ralph | Scope |
|-------|--------|
| DLZ-VST-001 | CMake + minimal plugin |
| DLZ-VST-002 | DrumEngine + voices + CLI WAV |
| DLZ-VST-003 | APVTS, UI, pluginval 5 |
| DLZ-VST-004 | Soft clip / sat |
| DLZ-VST-005 | 808 glide |
| DLZ-VST-006 | Hat choke + rolls |
| DLZ-VST-007 | User WAV pads + export |
| DLZ-VST-008 | FL Studio QA |

While iterating VST, also obey the JUCE block in `disklordz-ralph/PROMPT.md`. On pass, update **both** local `prd.json` (if present) and `disklordz-ralph/prd.json` in the push repo.