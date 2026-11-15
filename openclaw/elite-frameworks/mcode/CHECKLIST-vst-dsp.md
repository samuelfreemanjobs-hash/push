# MCode — VST DSP change checklist

- [ ] Read surrounding `processBlock` and parameter layout before editing
- [ ] No heap, locks, logging, or `juce::String` work on audio thread
- [ ] Smallest diff — no drive-by refactors
- [ ] `scripts/vst-build-pluginval-gate.ps1` (or local cmake + pluginval) pass
- [ ] REA / Ponytail rules from `docs/dev/vst-commercial-gates.md`
- [ ] EGM duel on spec if architecture changed (`egm/run_spec_duel.py`)
- [ ] Handoff pruned (`token-economizer/prune_handoff.py`) if >120 lines