# C++ VST Autoresearch Agent (JUCE + pluginval)

Optimize **`Source/VoiceDSP.h` only** against `targets/kick.wav`. After wins, human ports constants to `TrapCoreDSP` with Catch2 — do not edit `PluginProcessor::processBlock` in the autoresearch loop.

## Rules

1. **Editable:** `Source/VoiceDSP.h` only.
2. **Forbidden:** `evaluate_vst.py`, `HeadlessMain.cpp`, `CMakeLists.txt`, JUCE processor/editor.
3. **After each edit (Windows):**
   ```powershell
   cd disklordz-ralph\vst\autoresearch-cpp
   pip install -r ..\..\factory\autoresearch\requirements.txt
   python evaluate_vst.py
   ```
4. **Keep** if `TOTAL_LOSS` improves and no `ERROR:` lines; else `git checkout -- Source/VoiceDSP.h`.

Configure once:

```powershell
cd disklordz-ralph\vst\starter-cpp
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DDLZ_AUTORESEARCH=ON
```

Or use `scripts\vst-build-pluginval-gate.ps1` for full gate.

## Hardware research rule (RAGFlow MCP)

If `TOTAL_LOSS` fails to improve for **5** consecutive iterations, query **`KB_DSP_Hardware_Vault`** (RAGFlow MCP) for synthesis topology — e.g. bridged-T decay, soft-clip asymmetry — then propose **one** parameter change in `Source/VoiceDSP.h`. Do not paste copyrighted sample data.

**Processor wiring reference:** `examples/PluginProcessorAutoresearch.example.cpp` (not built).

Business copy / manuals: use **`KB_VST_Business_Ops`** via RAGFlow canvas (`docs/dev/ragflow-knowledge-base.md`), not this loop.

## Merge path

1. Autoresearch lowers loss on sandbox header.  
2. Operator maps fields → `Trap808Voice` / `SoftClipper`.  
3. `ctest` + `pluginval` on full plugin.  
4. Delete or freeze sandbox divergences.