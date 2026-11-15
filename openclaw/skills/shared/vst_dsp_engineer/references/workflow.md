# vst_dsp_engineer workflow

1. Confirm spec duel PASS for feature (ask `vst-architect` or run EGM locally).
2. Follow `openclaw/elite-frameworks/mcode/CHECKLIST-vst-dsp.md` on every commit.
3. No heap/locks/strings in `processBlock`.
4. Signal `HANDOFF_QA:build` to `vst-build` pod when compile-ready.