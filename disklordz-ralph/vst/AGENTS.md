# DetroitPhonkTrapMaker — Cursor / Superpowers / REA

Copy this file to **`C:\Dev\DetroitPhonkTrapVST3\AGENTS.md`** (alongside `CMakeLists.txt`).

## Product

Trap / phonk / Detroit drum rompler + WAV export. Architecture: `TRAP-PHONK-ARCHITECTURE.md`. Ralph stories: `DLZ-VST-*` in `C:\Dev\push\disklordz-ralph\prd.json`.

## Superpowers workflow

1. **Plan:** `brainstorming` → save plan under `docs/plans/<feature>-spec.md` (create `docs/plans/` if missing).
2. **Test:** Add failing Catch2 in `tests/` or extend `DrumTestCLI` **before** `Source/` changes.
3. **Implement:** Edit `trap-core-dsp/` first, then `PluginProcessor`.
4. **Review:** `spec-compliance` / code-review skill; run `ctest` + `pluginval` 5.
5. **Debug:** `systematic-debugging` → REA scoped audit → fix → re-run tests.

Install Superpowers in Cursor: Agent chat → `/add-plugin superpowers` or Marketplace ([obra/superpowers](https://github.com/obra/superpowers)).

## REA (after RelWithDebInfo build)

- Targets: `C:\Dev\push\disklordz-ralph\vst\rea-targets.json`
- Prompts: `REA-WORKFLOW.md` §3
- Notes: commit `rea-notes/*.md` in **push** repo only

## Build

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
ctest -C Release --test-dir build
```

## Layout

```text
trap-core-dsp/     # TrapCoreDSP static lib (DRY)
Source/            # Processor, editor, voices wiring
tests/             # Catch2
docs/plans/        # Superpowers writing-plans output
```