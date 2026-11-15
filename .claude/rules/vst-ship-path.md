---
paths:
  - "disklordz-ralph/vst/starter-cpp/**"
  - "disklordz-ralph/vst/Tests/**"
---

# Ship path — Detroit Phonk Trap Maker

**STACK-FROZEN:** Lite VST3 ships before R&D labs or MetaGPT portal merge.

- RT-safe: no alloc/lock/network in `processBlock`
- Ponytail **full** on `TrapCoreDSP` (not ultra on hot path)
- License verify: message thread only (`LicenseVerifier`, Monocypher)
- Before Core/paid: `scripts/vst-build-pluginval-gate.ps1`, strictness 8 pre-release

Autoresearch wins in `VoiceDSP.h` merge to ship tree **only with human review** — not automatic agent merge to `PluginProcessor.cpp`.