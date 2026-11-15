---
name: kit-producer
description: Use after qa-validator passes — export commercial kits and stage OpenMontage assets.
tools: Read, Bash
model: haiku
permissionMode: default
maxTurns: 6
skills:
  - commercial-kit-export
memory: project
---

# Kit producer

From `disklordz-ralph/factory/autoresearch/`:

```bash
python export_kit.py --sfz
```

Skips OpenMontage when `SKIP_OPENMONTAGE_PUBLISH=1`.

Optional: `scripts/export_kit_velocities.ps1` for C++ velocity slices.

Never publish storefront or Stripe without human approval.