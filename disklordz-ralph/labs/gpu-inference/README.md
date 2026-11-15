# GPU inference lab (TensorRT Model Connect) — R&D only

## Goal

Generate 8-second stereo phonk/trap loops via local TensorRT **off the audio thread**, with factory WAV fallback when no GPU engine is loaded.

## Scope

- Headers: `include/NeuralLoopGeneratorThread.h`, `TrtAudioRunner.h`
- Prototype host: `vst/templates/drumkit-plugin` or separate plugin ID
- Operator-side engine export (ACE-Step XL or licensed checkpoint)

## Non-goals

- Not linked to `starter-cpp` Lite ship path
- No cloud inference in RT or UI hot paths
- No SKU/marketing claims until commercial model license is recorded

## Architecture

UI / `PluginEditor` → `triggerGeneration()` → `NeuralLoopGeneratorThread` → `TrtAudioRunner` → atomic ready flag → playback buffer read from `processBlock()` (copy only).

## RT safety

- **Never** call TensorRT inside `processBlock()`.
- Worker: `juce::Thread` only; preallocated `AudioBuffer`; `std::atomic<bool>` handoff.
- REA: no CUDA sync on audio thread.

## Step A — Export engine (operator Windows + CUDA)

```powershell
pip install tensorrt-model-connect  # NVIDIA distribution — verify license
trt-model-connect export --hf-model "ACE-Step/ACE-Step-XL" --fp16 --output ./build/phonk_engine
trt-model-connect build-cpp --engine ./build/phonk_engine --target-arch sm_89 --out-dir ./Source/AIInference
```

Adjust `sm_XX` for target GPU. Store `.trt` under `%ProgramData%\DiskLordz\engines\` for installers.

## Step B — JUCE contract

- Use `NeuralLoopGeneratorThread` on `juce::Thread` (see `include/NeuralLoopGeneratorThread.h`).
- **CPU fallback:** 8-bar WAV stems in `Assets/FactoryFallback/` when `TrtAudioRunner::isLoaded()` is false.

## Acceptance

- [ ] EGM duel pass: `python openclaw/elite-frameworks/egm/run_spec_duel.py --spec <this file>`
- [ ] Waza eval green after editor wiring
- [ ] pluginval with thread stress
- [ ] Offline latency < 8s for 8-bar gen on RTX-class GPU (operator benchmark)

## Test plan

- pluginval thread stress + REA audit of `processBlock`
- Manual: trigger gen while audio plays — no xruns
- Fallback path when `.trt` missing

## Rollout

Opt-in lab SKU only; document engine path in installer; Helix node `spec:neural-trt-thread` updated on merge.

Commercial model license required before any SKU claims. See `docs/plans/ai-native-growth-roadmap.md`.