---
name: vst_factory
description: JUCE/VST factory — CMake, pluginval, GitHub Actions on Instruments.
user-invocable: true
---

- Repos: Instruments, DiskLordz-Audio
- CI workflows: `drumcrush.yml`, `disklordz-audio-windows.yml`, fleet nightly
- Prefer Windows CI green before claiming release-ready
- Changes: small diffs; match existing CMake patterns
- Escalate signing/notarization to operator