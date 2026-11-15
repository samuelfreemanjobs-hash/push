# Commercial VST autoresearch guardrails

For **paid** tiers, treat every agent DSP edit as a release candidate until gates pass.

## 1. pluginval gate (required)

Industry standard: [Tracktion pluginval](https://github.com/Tracktion/pluginval).

| Strictness | When |
|------------|------|
| **5** | Default — matches `DLZ-VST-003`, daily dev |
| **8** | Pre-release / agent “keep” commit (set `PLUGINVAL_STRICTNESS=8`) |

**Runner:** `scripts/vst-build-pluginval-gate.ps1` (Windows).

On failure: **`git checkout --`** the DSP files from the agent attempt; do not ship.

## 2. Real-time CPU budget (recommended)

Measure **wall time per `processBlock`** (not Python render time):

- Buffers: **64** and **128** samples  
- Sample rates: **48 kHz** and **96 kHz**  
- Scenario: worst-case voice count (all pads held + glide active)

**Example ceiling:** single-instance **&lt; 1.5%** of one core @ 128/48k on your reference laptop (document your machine in `vst/rea-notes/`).

Implementation path:

1. JUCE `AudioPerformanceCounter` or scoped timer in debug builds only  
2. `vst/Tests/` or standalone profiler target (future story)  

Until automated: REA manual pass after each agent merge.

## 3. Catch2 + REA (required for `TrapCoreDSP`)

- `ctest -C Release` green  
- No alloc/lock in `processBlock` (Ponytail full + REA)  

## 4. Royalty-free kit originality (Python)

When matching `targets/*.wav` for **commercial** kits:

- **Upper bound:** STFT + envelope loss vs *your* reference renders (not commercial masters)  
- **Lower bound (floor):** if spectral convergence **too low**, reject — likely cloning  

In `factory/autoresearch/evaluate.py`, a future `MIN_SPECTRAL_DISTANCE` env can penalize `sc_loss < floor`. Until wired, operator QC + Forge manifest review.

**Legal:** synthesis-only; references must be **your** TrapCoreDSP/Forge exports.

## 5. Strix / security red team (optional, paid tier)

Before **Core/Suite** checkout goes live:

- Run `scripts/strix-pre-release.ps1 -Quick` on `starter-cpp/Source`, `storefront/`, `business/openbb/`
- Staging URL scan per `docs/security/strix-vst-audit-playbook.md` (license API + signed kit downloads)
- No open **critical** findings on webhook forgery or unauthenticated ZIP access

Strix is **not** a substitute for pluginval or Catch2.

## 5. Agent loop integration

```text
Hypothesis → edit C++ OR drum_engine.py
    → build (C++ only)
    → pluginval (C++ only)
    → ctest (C++)
    → evaluate.py (Python kits)
    → keep commit only if all required gates pass
```

C++ agents must **not** skip pluginval for “audio-only” improvements.