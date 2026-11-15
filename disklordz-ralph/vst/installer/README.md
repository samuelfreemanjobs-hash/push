# Windows installers (Week 3)

## Prerequisites

- [Inno Setup 6](https://jrsoftware.org/isinfo.php)
- Release VST3 from CMake (`Detroit Phonk Trap Maker.vst3` bundle)
- Optional: EV code signing cert for SmartScreen (defer until Core ships)

## Lite (lead magnet)

```powershell
$build = "C:\Dev\DetroitPhonkTrapVST3\build\DetroitPhonkTrapMaker_artefacts\Release\VST3"
& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" `
  /DBuildRoot="$build" `
  "C:\Dev\push\disklordz-ralph\vst\installer\DetroitPhonkTrapMaker-Lite.iss"
```

Output: `Assets/Releases/installers/DetroitPhonkTrapMaker-Lite-1.0.0-win64.exe`

## Core (+ WAV pack)

```powershell
& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" `
  /DBuildRoot="$build" `
  /DWavPackRoot="C:\Dev\push\Assets\Releases\LouisianaCoreWavs" `
  "C:\Dev\push\disklordz-ralph\vst\installer\DetroitPhonkTrapMaker-Core.iss"
```

## DrumSynth Autoresearch (preview / NFR)

```powershell
$build = "C:\Dev\push\disklordz-ralph\vst\starter-cpp\build\autoresearch-cpp\DrumSynthAutoresearch_artefacts\Release\VST3"
& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" /DBuildRoot="$build" `
  "disklordz-ralph\vst\installer\DrumSynth-Autoresearch.iss"
```

## Code signing (operator)

After `ISCC`, sign the `.exe` (recommended). Optionally sign the `.vst3` bundle for strict DAW policies.

```powershell
signtool sign /fd SHA256 /tr http://timestamp.digicert.com /a `
  "Assets\Releases\installers\DetroitPhonkTrapMaker-Lite-1.0.0-win64.exe"
```

**macOS (future AU):** `codesign --deep --force --options runtime`, `pkgbuild`, `xcrun notarytool` — see `docs/dev/commercial-release-packaging.md`.

Moonbase / paid gate: `LICENSING.md` — integrate before shipping **Core** installer publicly.

## FL install path

VST3 installs to `{commoncf64}\VST3\` — FL Studio 21+ rescan: **Options → Manage plugins → Find plugins**.