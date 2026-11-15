---
name: vst_factory
description: JUCE/VST factory — CMake, pluginval, GitHub Actions on Instruments.
version: 1.0.0
user-invocable: true
homepage: https://github.com/samuelfreemanjobs-hash/push/tree/main/openclaw/skills/shared/vst_factory
---

Before acting, read `references/workflow.md` (and sibling files under `references/`) via skills_read.


- Repos: Instruments, DiskLordz-Audio
- CI workflows: `drumcrush.yml`, `disklordz-audio-windows.yml`, fleet nightly
- Prefer Windows CI green before claiming release-ready
- Changes: small diffs; match existing CMake patterns
- Escalate signing/notarization to operator