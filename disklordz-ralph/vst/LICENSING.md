# Product tiers & copy protection — DetroitPhonkTrapMaker

Beatmakers resist dongles and always-online DRM. **Sound + frictionless license** beats romplers on UX.

## Tiers

| Tier | Price | Plugin | Sounds | Upsell |
|------|-------|--------|--------|--------|
| **Lite** | $0 | VST3/AU, 1 kit (~15–20 hits), **Soft Clip** knob on, no user import | Factory only | Email list; FL/Ableton compatibility test |
| **Core** | $39–49 | Full engine: glide, multi-out (v1.5), drag-drop pads, user WAV | 150+ factory + **unlocked 24-bit WAV folder** | Tripwire bundle same checkout |
| **Suite** | $79–99 | Core + 3–4 expansions | Memphis phonk, rage, dark drill presets | MIDI roll packs |
| **Expansions** | $19 | Encrypted `.blob` / archive in-app loader | — | Post-launch cadence |

**Wedge:** Core tier ships **VST + raw WAVs** (FL Channel Rack users) — see tripwire `DLZ-DRUM-*`.

## Copy protection

| Layer | Approach |
|-------|----------|
| **License** | [Moonbase Licensing](https://marketplace.juce.com/products/moonbase-cpp) JUCE module — browser activate, offline machine file, **lock-free audio gate** (no clicks on verify) |
| **Lite gate** | Disable `PadDragExport` + sample import in binary; clip + factory kit only |
| **Factory audio** | Pack as binary container or decrypt in `MemoryInputStream` on load — **not** loose WAV in installer |
| **Buyer WAV bonus** | Separate unlocked folder in Core/Suite (commercial promise, not DRM target) |

Defer Moonbase until **Core** tier ships; Lite can be unsigned/no DRM for list-building.

## JUCE license (verify annually)

- Track revenue against [JUCE license](https://juce.com/legal/juce-8-licence/) thresholds (Starter / Indie / Pro).
- Sub-$20k–$50k guidance changes by tier — confirm before launch.

## Ralph / gates

- `DLZ-VST-003` — pluginval before any paid tier
- `DLZ-VST-008` — FL Studio QA
- `DLZ-VST-009` — multi-out routing (Suite differentiator)
- `DLZ-BIZ-*` — Stripe + tripwire SKU mapping

## What not to ship in v1

- iLok / phone-home every session
- Encrypted expansions before Core plugin is stable
- Loose factory WAV inside plugin bundle (piracy magnet)