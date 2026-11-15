# DISKLORDZ GOOD MONEY DRUM LAB PRO (R&D)

Larry June–**influenced** West Coast drum synthesis lab: **Player Bounce** vs **Luxury Soul**, eight DNA banks, shared three-layer core.

> Original sounds only — not artist sample banks or endorsements.

| Shared core | `../factory/drum-architecture/Source/ThreeLayerDrum.h` |
| Sibling product | `../maybach-drum-lab/` (luxury cinematic) |
| Ship path (frozen) | `../vst/starter-cpp/` Detroit Phonk Lite |

## Build M1 (host / CI)

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/GoodMoneyM1Renderer renders
```

## Windows JUCE (optional)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DDISKLORDZ_GOODMONEY_BUILD_JUCE=ON
cmake --build build --config Release
```

MIDI: **C1/Kick 36**, **808 35/37**, **Snare 38**. APVTS: DNA bank + Player mode.

## Master spec

Full product blueprint: `../../docs/plans/good-money-drum-lab-spec.md`

## Marketing

After M9 factory output, use `/disk-marketing-kit` (Claude) or `@marketing-drum-commerce` (Cursor).