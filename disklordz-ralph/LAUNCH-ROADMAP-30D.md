# First kit + Lite VST3 → first sales (30-day milestones)

Technical milestones only — adjust calendar on your Windows build machine.

## Phase A — Engine + drag-export (ship blocker)

| # | Deliverable | Done when |
|---|-------------|-----------|
| A1 | `TrapCoreDSP` Release + Catch2 green on Windows | `ctest -C Release` passes |
| A2 | `DetroitPhonkTrapMaker` `.vst3` + pluginval level 5 | Strict validation pass |
| A3 | **DLZ-VST-007** wired | Click pad → WAV on FL Playlist; Shift+click → `.mid`; drop WAV on pad loads sample |
| A4 | REA smoke on `processBlock` only | Notes in `rea-notes/` — no full decompile |

**Copy pack:** `vst/starter-cpp/*` + `integration/Source/*` → `C:\Dev\DetroitPhonkTrapVST3\Source\`.

## Phase B — First Louisiana kit (revenue asset)

| # | Deliverable | Done when |
|---|-------------|-----------|
| B1 | 24-bit master WAVs in `Assets/RawMasters/` | Peak/headroom checked |
| B2 | Generator run → `Assets/Releases/` + `kit_manifest.json` | Matches schema |
| B3 | **Lite** tier: 25 WAVs + Lite `.vst3` zip | Download page draft in `docs/copy/` |
| B4 | FL QA doc signed | `vst/docs/FL-STUDIO-QA.md` checklist |

## Phase C — Lite in producers’ hands

| # | Deliverable | Done when |
|---|-------------|-----------|
| C1 | Landing copy from market snapshot | `docs/copy/landing-page.md` operator-approved |
| C2 | OpenClaw Thu: leads + price intel | Cron live; you approve drafts only |
| C3 | 10–15 personalized Lite outreach sends | Secondary domain, human-approved |
| C4 | One public cook-up clip (808 + bounce) | Proves drag-export UX |

## Phase D — Paid Core drop

| # | Deliverable | Done when |
|---|-------------|-----------|
| D1 | Core tier ($49) storefront **draft** | AutoGPT or manual; you approve |
| D2 | Gumroad/Lemon/Shopify link live | Lite still $0 lead magnet |
| D3 | First paying customer + install FAQ | Support answers from `vst/docs/` only |
| D4 | Friday AutoGPT kit job optional | `Assets/Releases/` trigger only |

## Daily default (after stack frozen)

1. **Build lane (Cursor):** one `prd.json` story — test first, Ponytail **full** on implementation.  
2. **No new tools.** If tempted, run `/ponytail-audit` equivalent prompt from `ponytail-juce.mdc`.  

## Success metrics at day 30

- Lite VST3 installed on ≥5 FL producers (verified replies or analytics)
- ≥1 paid Core or kit sale **or** validated waitlist with deposit intent
- Zero audio-thread allocations in REA-reviewed build