# JUCE 8 WebView2 drum starter (template)

R&D scaffold: **VST3 + Standalone**, embedded `ui/*`, static WebView2, **`validate_vst3`** CMake target.

**Not** the Lite ship SKU — fork or copy when building a WebView-heavy instrument (e.g. Maybach UI M8).

## Prerequisites (Windows)

1. Visual Studio 2022 + CMake 3.22+
2. JUCE: monorepo `starter-cpp/externals/JUCE` (default) or FetchContent
3. WebView2 SDK in NuGet cache:

```powershell
winget install Microsoft.NuGet
.\scripts\install-webview2-nuget.ps1
```

4. [pluginval](https://github.com/Tracktion/pluginval) — repo copy: `disklordz-ralph/vst/tools/pluginval/`

## Build

```powershell
cd disklordz-ralph\vst\templates\webview-drum-starter
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
cmake --build build --config Release --target validate_vst3
```

Options:

| CMake flag | Default |
|------------|---------|
| `DLZ_USE_MONOREPO_JUCE` | ON |
| `DLZ_COPY_VST3_AFTER_BUILD` | OFF |
| `DLZ_WEBVIEW_STATIC` | ON |
| `PLUGINVAL_STRICTNESS` env | 8 |

## Layout

```
src/          PluginProcessor / PluginEditor (RT-safe kick + atomic UI trigger)
ui/           Embedded via juce_add_binary_data → WebUIAssets.h
```

Native integration: HTML `emitEvent("triggerKick")` → `triggerKickFromUi()` (message thread / native bridge — not `processBlock`).

## Agent loop

See `docs/dev/juce8-cmake-webview-starter.md` and `.cursor/rules/audio-dsp.mdc`.