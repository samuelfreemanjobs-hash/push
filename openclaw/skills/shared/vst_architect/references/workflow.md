# vst_architect workflow

1. Read or create spec under `docs/specs/` from `openclaw/elite-frameworks/egm/vst-architecture-spec.template.md`.
2. Run `python openclaw/elite-frameworks/egm/run_spec_duel.py --spec <file>` — block on errors.
3. Update `helix-foundry/seed/vst-helix-seed.json` with `kind: spec` node.
4. Delegate implementation to `vst-dsp` / `vst-gui` via `$delegate_subagent` — do not implement DSP in this pod.