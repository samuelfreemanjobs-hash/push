---
paths:
  - "disklordz-ralph/vst/starter-cpp/**"
  - "disklordz-ralph/vst/autoresearch-cpp/**"
  - "disklordz-ralph/vst/Modules/**"
  - "disklordz-ralph/maybach-drum-lab/**"
  - "disklordz-ralph/tm88-drum-lab/**"
  - "disklordz-ralph/luger-drum-lab/**"
---

# Elite JUCE 8 DSP (Claude Code)

Full playbook: **`docs/dev/elite-dsp-agent-context7.md`**. Cursor mirror: **`.cursor/rules/audio-dsp.mdc`**.

## Before coding

- **Context7** (`use context7`) for JUCE 8 + CMake when MCP is available.
- Read callers; prefer `TrapCoreDSP` / `ThreeLayerDrum` over new oscillators.

## processBlock contract

- `ScopedNoDenormals`
- No heap, locks, strings, file I/O, license JSON parse on audio thread
- `SmoothedValue` / atomics for parameters and UI triggers (`uiTriggerPending` pattern)
- Sample-accurate MIDI block splits

## After coding

1. `cmake --build` + `ctest` (Windows operator)
2. `PLUGINVAL_STRICTNESS=8` for ship-path keeps
3. Invoke **`dsp-rt-auditor`** or `/disk-rt-audit` on the diff

Autoresearch sandbox `VoiceDSP.h` must stay RT-safe — same rules apply inside `processBlock` helpers.