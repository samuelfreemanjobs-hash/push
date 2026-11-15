# JUCE 8 CMake — WebView2 + pluginval template

Production boilerplate lives at:

**`disklordz-ralph/vst/templates/webview-drum-starter/`**

Adapted from JUCE 8 WebView UI blog/docs — DiskLordz defaults:

- **Local JUCE** from `starter-cpp/externals/JUCE` (STACK policy: no surprise FetchContent in ship tree)
- **`COPY_PLUGIN_AFTER_BUILD` OFF** unless operator opts in
- **pluginval** strictness **8** (env override; 10 optional)
- **Static WebView2**: `JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1`

---

## Windows setup

```powershell
cd C:\Dev\push
.\scripts\install-webview2-nuget.ps1

cd disklordz-ralph\vst\templates\webview-drum-starter
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
cmake --build build --config Release --target validate_vst3
```

Place `pluginval.exe` in `disklordz-ralph/vst/tools/pluginval/` or `PATH`.

---

## What the template includes

| Feature | Implementation |
|---------|----------------|
| `juce_add_plugin` VST3 + Standalone | `NEEDS_WEBVIEW2 TRUE` |
| Embedded UI | `juce_add_binary_data` → `ui/index.html`, `style.css`, `app.js` |
| ResourceProvider | `PluginEditor.cpp` serves `juce://local/` |
| Lock-free drum trigger | `std::atomic<bool> kickPending` + WebView `emitEvent("triggerKick")` |
| APVTS + smoothing | `outputGain` + `SmoothedValue` |
| MSVC `/W4` | CMake `target_compile_options` |
| `validate_vst3` | `add_custom_target` → `DiskLordzWebviewDrumStarter_VST3` |

---

## Agent verification (add to workflow)

```text
1. Context7 — WebBrowserComponent::Options, ResourceProvider, native integration
2. cmake --build build --config Release
3. cmake --build build --config Release --target validate_vst3
4. /disk-rt-audit on src/*.cpp
```

---

## Wiring APVTS relays (next step)

For sliders in HTML bound to automatable parameters, add JUCE 8 **WebSliderRelay** / **WebToggleButtonRelay** on the editor and expose names in `index.html` via `__JUCE__.backend`.

Keep relay updates on the message thread; never read APVTS from JavaScript inside `processBlock`.

---

## Related

- [JUCE 8 WebView overview](https://juce.com/blog/juce-8-feature-overview-webview-uis/)
- `docs/dev/elite-dsp-agent-context7.md`
- `storefront/plugin-faceplate/` — design mock (not embedded)