# DiskLordz TM88 Drum Lab PRO (R&D)

**Dark melodic · emo trap · space trap** drum synthesis lab — original three-layer DSP shared with Luger / South / Forge.  
**Not** an official TM88 product, collaboration, or sample recreation.

**Ship path unchanged:** Detroit Phonk Trap Maker → `vst/starter-cpp/`.

## M1 (this folder)

| Piece | Status |
|-------|--------|
| Six DNA identities | `Source/Tm88Identity.h` |
| 808 preset offsets (subset) | `bass808ForPreset()` — M2 expands to 12 |
| Movement / groove placeholders | `Source/Tm88Movement.h` (M5 engine) |
| Offline renderer | `Tm88M1Renderer` |
| Tests | `Tm88Milestone1Tests` |
| Optional JUCE | `-DDISKLORDZ_TM88_BUILD_JUCE=ON` → VST3 + Standalone |

## Build (portable core, no JUCE)

```bash
cd disklordz-ralph/tm88-drum-lab
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
./build/Tm88M1Renderer ./renders
```

## Signature vs South / Luger

| South / Luger | TM88 lab |
|---------------|----------|
| Aggressive impact | Expressive 808 **glide** + space |
| Dense patterns | **Rhythmic sparsity** targets (M5) |
| Punch / distortion focus | Emotional percussion + atmosphere |

## Roadmap

Full product spec: `docs/plans/tm88-drum-lab-master-prompt.md` (M2–M9: Hat Lab, Factory 500, GUI).

Proposed kit: **NIGHTMARE VOL. 1** — 500 one-shots (factory milestone M8).