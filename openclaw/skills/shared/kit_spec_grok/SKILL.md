---
name: kit_spec_grok
description: Validate and ingest Grok Drum Kit Architect kit-spec.yaml for OpenClaw factory.
user-invocable: true
metadata:
  {"openclaw":{"requires":{"bins":["node"]}}}
---

1. Require `architect.system: grok-bot-drum-kit-architect`.
2. Validate: `openclaw/drum-kit-factory/scripts/validate-kit-spec.mjs <file>`.
3. Store under `kits/<kit_id>/kit-spec.yaml` in drum-lead workspace.
4. Emit factory work order for QC → manifest → pack.
5. Creative changes go back to Grok, not rewritten here.