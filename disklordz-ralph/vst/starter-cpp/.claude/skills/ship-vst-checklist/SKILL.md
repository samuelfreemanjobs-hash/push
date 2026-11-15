---
name: ship-vst-checklist
description: Pre-ship checklist for Detroit Phonk Trap Maker — pluginval, license flag, no autoresearch bleed.
allowed-tools: Read, Bash
---

# Ship VST checklist

1. `docs/dev/vst-commercial-gates.md`
2. Windows: `scripts/vst-build-pluginval-gate.ps1` with `PLUGINVAL_STRICTNESS=8`
3. Confirm **no** autoresearch-only symbols in `PluginProcessor` / `TrapCoreDSP`
4. Core tier: `DDLZ_LICENSE_ED25519=ON`, replace `DlzEd25519PublicKey.h` before retail
5. Human sign-off on version string and installer path

Do not auto-merge `VoiceDSP.h` wins into ship DSP without review.