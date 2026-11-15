# JUCE 8 + Serena symbolic engineering SOP (DrumKit Pro)

**Context7:** query JUCE 8 signatures before API edits (`docs/dev/elite-dsp-agent-context7.md`).

## 1. Mandatory symbol-first workflow

- Do **not** load entire `src/PluginProcessor.cpp` to inspect one method. Use `get_symbols_overview`, then `find_symbol` with `include_body=True` (e.g. `DrumKitAudioProcessor::processBlock`).
- Before renaming an APVTS id, DSP method, or `WebSliderRelay` name, run **`find_referencing_symbols`** on `src/` **and** `ui/` (C++ + HTML/JS must stay aligned).

## 2. New APVTS parameter (3-symbol checklist)

1. **`createParameterLayout`** (`PluginProcessor.cpp`): Context7 → `juce::ParameterID` + `AudioParameterFloat`; edit via `replace_symbol_body`.
2. **Header + `prepareToPlay`:** `std::atomic<float>*` raw APVTS pointer + `juce::SmoothedValue<float>` via `insert_after_symbol` on the processor class.
3. **`processBlock`:** `load(std::memory_order_relaxed)` + per-sample smoothing. **No** heap, locks, `juce::String`, or `vector` growth on the audio thread.

## 3. WebView2 bridge

- C++: `WebSliderRelay` id must match HTML `id` and `ui/app.js` `CONTROL_IDS`.
- After C++ relay rename, re-run `find_referencing_symbols` on `ui/index.html` and `ui/app.js`.

## 4. Post-edit verification (Windows)

```powershell
cmake --build build --config Release --parallel
$env:DLZ_PLUGINVAL = "C:\Dev\push\disklordz-ralph\vst\tools\pluginval\pluginval.exe"
& $env:DLZ_PLUGINVAL --strictness-level 10 --validate `
  "build\DrumKitPlugin_artefacts\Release\VST3\DrumKit Pro.vst3"
```

Or: `python vst_rd_swarm.py` (RT auditor + build QA).

## 5. Forbidden paths

- Do not index or bulk-edit under `build/_deps/juce-src`.
- Ship tree `starter-cpp` requires `DLZ_SWARM_ALLOW_SHIP=1` + human review.