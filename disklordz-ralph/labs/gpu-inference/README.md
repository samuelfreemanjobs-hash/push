# GPU inference lab (TensorRT Model Connect) — R&D only

**Not** linked to `starter-cpp` ship path. Commercial model license required before any SKU claims.

## Step A — Export engine (operator Windows + CUDA)

```powershell
pip install tensorrt-model-connect  # NVIDIA distribution — verify license
trt-model-connect export --hf-model "ACE-Step/ACE-Step-XL" --fp16 --output ./build/phonk_engine
trt-model-connect build-cpp --engine ./build/phonk_engine --target-arch sm_89 --out-dir ./Source/AIInference
```

Adjust `sm_XX` for target GPU. Store `.trt` under `%ProgramData%\DiskLordz\engines\` for installers.

## Step B — JUCE contract

- **Never** call TensorRT inside `processBlock()`.
- Use `NeuralLoopGeneratorThread` (see `include/NeuralLoopGeneratorThread.h`) on `juce::Thread`.
- Playback: lock-free handoff via `std::atomic<bool>` + preallocated `AudioBuffer`.
- **CPU fallback:** ship 8-bar WAV stems in `Assets/FactoryFallback/` when `TrtAudioRunner::isLoaded()` is false.

## Integration target

Prototype host: `vst/templates/drumkit-plugin` or separate product `DiskLordz Neural Phonk` (new plugin ID).

## QA

- pluginval with thread stress
- Offline latency benchmark (target: < 8s for 8-bar gen on RTX class GPU)
- REA: verify no CUDA sync on audio thread

See `docs/plans/ai-native-growth-roadmap.md`.