# JUCE + CMake — Windows first-build troubleshooting

DiskLordz builds from **`disklordz-ralph/vst/starter-cpp/`** (ship plugin + optional autoresearch). Most first failures are **toolchain**, **missing JUCE**, or **wrong CMake target/path names** — not DSP code.

---

## 1. Prerequisite toolchain

| Check | Command / action |
|-------|------------------|
| **CMake 3.22+** | `cmake --version` |
| **MSVC + Windows SDK** | Visual Studio 2022 → workload **Desktop development with C++** |
| **Developer shell** | **Developer PowerShell for VS 2022** (so `cl.exe` is on PATH) — not plain PowerShell if CMake says “no CMAKE_CXX_COMPILER” |
| **JUCE present** | `Test-Path disklordz-ralph\vst\starter-cpp\externals\JUCE\CMakeLists.txt` must be **True** |

### Install JUCE (not vendored in git)

```powershell
cd C:\Dev\push\disklordz-ralph\vst\starter-cpp
New-Item -ItemType Directory -Force -Path externals | Out-Null
git clone --depth 1 --branch 8.0.4 https://github.com/juce-framework/JUCE.git externals\JUCE
```

If you use a submodule elsewhere, `git submodule update --init --recursive` from repo root.

**Wrong layout:** `externals\JUCE\JUCE\CMakeLists.txt` — CMake expects `add_subdirectory(externals/JUCE)` exactly as in `starter-cpp/CMakeLists.txt`.

---

## 2. Error → cause → fix (DiskLordz paths)

| Message | Root cause | Fix |
|---------|------------|-----|
| `add_subdirectory given source "…/JUCE" which is not an existing directory` | JUCE not cloned | Section 1 above |
| `Cannot open include file: 'juce_audio_processors/...'` | Target not linking JUCE modules | See `target_link_libraries` in `starter-cpp/CMakeLists.txt` / `autoresearch-cpp/CMakeLists.txt` (`juce::juce_audio_utils`, `juce::juce_dsp`, …) |
| `Cannot open include file: 'crypto/monocypher.h'` | Wrong path (not `Source/crypto/`) | Monocypher lives at **`starter-cpp/Modules/License/crypto/`**; autoresearch VST lists it in `autoresearch-cpp/CMakeLists.txt` via `DLZ_STARTER_DIR` |
| `Cannot open include file: 'LicenseVerifier.h'` | Missing include dir | Autoresearch plugin adds `${DLZ_STARTER_DIR}/Source` — rebuild after pull |
| `LNK2019: unresolved external symbol createPluginFilter` | Processor `.cpp` not in `target_sources` | Ensure `PluginProcessor.cpp` is listed for your target |
| AU / AudioUnit failures on Windows | AU is macOS-only | Our targets use `FORMATS VST3 Standalone` only — no AU on Windows |
| Wrong target name in `cmake --build --target` | JUCE CMake naming | Use **`DrumSynthAutoresearch_VST3`**, not `DrumSynth_VST3` |

---

## 3. Clean configure and staged build

Run from **`starter-cpp`** (single build tree for ship + autoresearch):

```powershell
cd C:\Dev\push\disklordz-ralph\vst\starter-cpp
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DDLZ_AUTORESEARCH=ON `
  -DDLZ_AUTORESEARCH_VST=ON `
  -DDLZ_LICENSE_ED25519=ON

# Isolate DSP (no UI) first
cmake --build build --config Release --target HeadlessRender

# Preview VST (VoiceDSP + license UI)
cmake --build build --config Release --target DrumSynthAutoresearch_VST3

# Ship SKU
cmake --build build --config Release --target DetroitPhonkTrapMaker_VST3
```

One-shot (DrumSynth + pluginval strictness 8):

```powershell
cd C:\Dev\push
.\scripts\vst-build-drumsynth-autoresearch.ps1
```

Diagnostics only:

```powershell
.\scripts\vst-diagnose-windows-build.ps1
```

---

## 4. Expected artifacts (Release)

Paths are under **`starter-cpp\build\autoresearch-cpp\`** for autoresearch targets:

| Binary | Typical path |
|--------|----------------|
| Headless render | `build\autoresearch-cpp\HeadlessRender_artefacts\Release\HeadlessRender.exe` |
| Autoresearch VST3 | `build\autoresearch-cpp\DrumSynthAutoresearch_artefacts\Release\VST3\DrumSynth Autoresearch.vst3` |
| Autoresearch Standalone | `build\autoresearch-cpp\DrumSynthAutoresearch_artefacts\Release\Standalone\DrumSynth Autoresearch.exe` |
| Ship VST3 | `build\DetroitPhonkTrapMaker_artefacts\Release\VST3\Detroit Phonk Trap Maker.vst3` |

Then:

```powershell
python disklordz-ralph\vst\autoresearch-cpp\evaluate_vst.py
$env:PLUGINVAL_STRICTNESS = "8"
.\scripts\vst-build-pluginval-gate.ps1
```

---

## 5. Ed25519 keypair (Core / license UI smoke test)

Plugin verifies against **`Modules/License/DlzEd25519PublicKey.h`**. Server signs with the matching private key.

```powershell
pip install pynacl
python disklordz-ralph\business\reference\generate_ed25519_keypair.py
# Writes keypair hex; paste PUBLIC into DlzEd25519PublicKey.h — never commit PRIVATE
```

Activate flow: open Standalone → copy **Machine ID** from demo banner → sign with `sign_license_payload.py` → paste JSON into license bar. Details: `docs/dev/juce-ed25519-offline-license.md`.

---

## Related

- `disklordz-ralph/vst/WINDOWS-BUILD.md` — REA / FL Studio
- `disklordz-ralph/vst/autoresearch-cpp/README.md`
- `docs/dev/vst-commercial-gates.md`