---
name: egm_spec_challenge
description: Elephant-Goldfish spec duel before VST or factory architecture commits.
version: 1.0.0
user-invocable: true
---

# EGM spec challenge

## When to use

- New GPU / TensorRT / multi-thread audio path
- New agent framework touching `vst/` or factory pipelines
- PRD > 1 screen without acceptance tests

## Workflow

1. Copy `openclaw/elite-frameworks/egm/vst-architecture-spec.template.md` if greenfield.
2. Run:
   ```bash
   python openclaw/elite-frameworks/egm/run_spec_duel.py --spec <your-spec.md>
   ```
3. Fix **error** findings before C++.
4. Link spec path in PR body.

Helix node kind `spec` should reference the same path in `helix-foundry/seed/`.