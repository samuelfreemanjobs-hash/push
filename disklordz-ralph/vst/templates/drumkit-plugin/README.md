# DrumKit Pro (JUCE 8 template)

RT-safe starter for **VST3 + WebView2** with `outputGain` on `juce::SmoothedValue` and embedded `ui/index.html`.

## Layout

```
drumkit-plugin/
├── CMakeLists.txt
├── vst_rd_swarm.py      # AutoGen closed loop (Windows operator)
├── src/
└── ui/
```

Built artefact: `build/DrumKitPlugin_artefacts/Release/VST3/DrumKit Pro.vst3`

## Windows build

```powershell
# JUCE at starter-cpp/externals/JUCE (clone 8.0.4+ if missing)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DDLZ_USE_MONOREPO_JUCE=ON
cmake --build build --config Release
```

## Swarm

```powershell
$env:OPENAI_API_KEY = "..."
# optional: $env:CONTEXT7_API_KEY = "..."
# optional: $env:PLUGINVAL_STRICTNESS = "10"
# optional: $env:DLZ_PLUGINVAL = "C:\Tools\pluginval\pluginval.exe"

python vst_rd_swarm.py "Add drive saturation with SmoothedValue; audit, build, pluginval"
```

Or from repo root: `.\scripts\run-vst-rd-swarm.ps1 -ProjectRoot ...\drumkit-plugin`

## Next steps (pick one)

1. **DrumVoiceEngine** — velocity layers + round-robin sample playback (MIDI loop already stubbed in `processBlock`).
2. **Web UI** — drum pads, rotaries, and more `WebSliderRelay` / `WebSliderParameterAttachment` pairs.