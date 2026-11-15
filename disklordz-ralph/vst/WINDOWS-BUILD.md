# Windows MSVC build — REA + FL Studio

## Configure & build

```powershell
cd C:\Dev\DetroitPhonkTrapVST3
copy C:\Dev\push\disklordz-ralph\vst\starter-cpp\CMakeLists.txt .
# copy starter-cpp\*.h into Source\

cmake -B build -G "Visual Studio 17 2022" -A x64 -DDLZ_BUILD_TESTS=ON
cmake --build build --config Release --parallel
ctest -C Release --test-dir build --output-on-failure
cmake --build build --config RelWithDebInfo --parallel   # preferred for REA
```

## Where artifacts land

| Artifact | Path |
|----------|------|
| **VST3 (test)** | `build\DetroitPhonkTrapMaker_artefacts\Release\VST3\Detroit Phonk Trap Maker.vst3` |
| **PDB (REA)** | next to `.vst3` or under `build\...\*.pdb` — point Ghidra symbol server at `build\` |
| **pluginval** | `.\tools\pluginval\pluginval.exe --strictness-level 5 --validate "…\Detroit Phonk Trap Maker.vst3"` |

**FL Studio without admin copy:** add VST folder manually in FL → *Manage plugins* → path to `...\Release\VST3\` (not only Program Files).

## Optional: auto-copy to Program Files

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DDLZ_COPY_VST3_TO_PROGRAMFILES=ON
# Run PowerShell as Administrator
cmake --build build --config Release --parallel
```

Default `CMakeLists.txt` keeps `COPY_PLUGIN_AFTER_BUILD` **OFF** per Ralph `PROMPT.md`.

## REA pairing

1. Build **RelWithDebInfo** (or Release + `/Zi`).
2. Copy `rea-targets.example.json` → `rea-targets.json`; set paths.
3. Run prompt **§3A** in `REA-WORKFLOW.md`.
4. Save to `rea-notes\rt-safety-latest.md`.

## Next features (order)

1. **Drag-export** — `PadDragExport.h` → `DLZ-VST-007` (FL Playlist drops first).
2. **Multi-out** — `TrapMultiOutLayout.h` → `DLZ-VST-009` (Suite tier mixer buses).