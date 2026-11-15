# vst_factory workflow

## Repos (typical)

- Plugin / JUCE or Rust VST in vault kanban `VST` lane
- CI via GitHub Actions (see `flow_github_actions` skill)

## Steps

1. Read kanban VST cards and ops snapshot `vst` block.
2. Propose build/PR steps; no unsigned binary distribution without gate.
3. Coordinate with overnight agent for long builds only when scheduled.