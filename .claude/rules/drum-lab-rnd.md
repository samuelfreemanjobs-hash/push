---
paths:
  - "disklordz-ralph/luger-drum-lab/**"
  - "disklordz-ralph/tm88-drum-lab/**"
  - "disklordz-ralph/factory/drum-architecture/**"
---

# Drum Design Lab (R&D)

Genre labs (**Luger**, **TM88**, etc.) share `factory/drum-architecture/Source/ThreeLayerDrum.h`.

- **Original synthesis only** — no producer name clones or commercial sample theft
- `renderDesign()` — offline only (allocates); `ThreeLayerDrum::next()` — RT-safe
- M1 pattern: identity header + CMake tests + optional `-D*_BUILD_JUCE=ON`
- TM88 master spec: `docs/plans/tm88-drum-lab-master-prompt.md`

Does **not** replace `vst/starter-cpp/` ship priority.