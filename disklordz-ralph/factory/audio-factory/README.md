# Audio Factory (unified pipeline)

One tree for **DSP benchmark → mastered WAV pack → APVTS presets + velocity map**.

```
audio-factory/
├── factory_swarm.py       # AutoGen + Context7 (delegates to ms-agent-framework)
├── src/dsp/VintageSaturator.h   # JUCE DSP (swarm-editable)
├── dsp_harness/dsp_harness.cpp  # Offline 24-bit WAV CLI
├── raw_audio/
├── dist_sample_pack/
└── dist_presets/
```

## Prerequisites

```powershell
.\scripts\install-ms-agent-framework.ps1
pip install soundfile numpy scipy
```

## Windows

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DDLZ_USE_MONOREPO_JUCE=ON
cmake --build build --config Release --target dsp_harness

.\build\dsp_harness_artefacts\Release\dsp_harness.exe raw_audio\test_loop.wav dist_sample_pack\test_out.wav 0.65

$env:OPENAI_API_KEY = "..."
# optional: $env:CONTEXT7_API_KEY = "..."

python factory_swarm.py
```

Outputs feed `business/release/package_release.py` when you copy packs into `factory/autoresearch/outputs/packs` or ship `dist_sample_pack` directly.

## Agents

| Agent | Tools |
|-------|--------|
| DSP_Loop_Engineer | `update_and_compile_dsp`, `benchmark_dsp_audio` |
| Sample_Pack_Factory | `process_and_master_sample_pack` |
| VST_Preset_Factory | `write_and_validate_vst_preset`, `generate_velocity_layer_map` |

Termination: `ALL_FACTORIES_COMPLETE` or `DLZ_FACTORY_MAX_TURNS` (default 18).

## Preset XML (drumkit-plugin)

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Parameters>
  <PARAM id="outputGain" value="1.15"/>
</Parameters>
```