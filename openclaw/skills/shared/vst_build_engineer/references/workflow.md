# vst_build_engineer workflow

1. Run `scripts/vst-build-pluginval-gate.ps1` (operator Windows) or document CI run.
2. Check `.github/workflows/` for regressions when touching CMake.
3. On failure: tail 40 lines to `vst-dsp` with `HANDOFF_DSP:fix`.
4. On pass: `HANDOFF_FACTORY:presets` or `vst-qa` Waza pipeline.