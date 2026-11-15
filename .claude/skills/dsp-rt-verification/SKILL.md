---
name: dsp-rt-verification
description: Grep-based RT safety review for JUCE processBlock and Drum Lab next() paths.
allowed-tools: Read, Grep, Glob
---

# DSP RT verification

## Search patterns (in diff or file)

In `processBlock`, `renderNextBlock`, `next()`:

| Pattern | Risk |
|---------|------|
| `new `, `delete`, `malloc`, `push_back`, `emplace_back` | Heap |
| `juce::String`, `std::string`, `toString()` | Alloc |
| `CriticalSection`, `std::mutex`, `ScopedLock` | Block |
| `loadFileAsString`, `File::`, `InputStream` | I/O |
| `verifyLicenseJson`, `JSON::parse` | Wrong thread |
| `DBG(` | Logging |

## Required positives

- `ScopedNoDenormals` in `processBlock`
- Continuous APVTS params use `SmoothedValue` or block-rate smoothing
- MIDI: sub-block render at event offsets (see `MaybachPluginProcessor` / ship processor)

## Build gate reminder

Operator: `scripts/vst-build-pluginval-gate.ps1` with `PLUGINVAL_STRICTNESS=8`.

Docs: `docs/dev/elite-dsp-agent-context7.md`, `.cursor/rules/audio-dsp.mdc`.