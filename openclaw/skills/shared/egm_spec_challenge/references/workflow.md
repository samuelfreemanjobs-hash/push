# egm_spec_challenge workflow

## Triggers

- Greenfield VST, factory GPU path, or TensorRT audio feature
- Multi-thread DSP or agent framework touching `vst/` or factory pipelines
- Large PRD without acceptance tests

## Steps

1. Copy `openclaw/elite-frameworks/egm/vst-architecture-spec.template.md` when starting from scratch.
2. Run `python openclaw/elite-frameworks/egm/run_spec_duel.py --spec <your-spec.md>`.
3. Resolve all **error** findings before opening C++ implementation PRs.
4. Link the spec file in the PR body and in Helix `spec` nodes when using `helix-foundry`.

## Output

Documented spec duel result and a go/no-go for implementation.