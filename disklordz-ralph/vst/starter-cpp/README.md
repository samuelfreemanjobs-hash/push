# Starter C++ — copy into `C:\Dev\DetroitPhonkTrapVST3\Source\`

Files are **header-heavy** and JUCE 8–oriented. Wire into your `juce_add_plugin` target after `DLZ-VST-001`.

| File | Role |
|------|------|
| `TrapSamplerSound.h` | Root note + sample buffer (message-thread load) |
| `Trap808Voice.h` | MultSmoothed pitch glide legato |
| `TrapSoftClipChain.h` | 2x oversampled cubic soft clip (prepare in `prepareToPlay`) |
| `PadDragExport.h` | Drag processed WAV to FL/Ableton (editor thread) |

REA: scoped audits only — see `../REA-WORKFLOW.md`. After RelWithDebInfo build, run §3A; save to `../rea-notes/`.

License tiers: `../LICENSING.md`. Agent workflow: `../AGENTS.md`, `../SUPERPOWERS.md`.