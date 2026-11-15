# Claude-Mem seed — DiskLordz push repo

Paste into a new Cursor chat or the Claude-Mem viewer after first install.

## Stack (frozen)

- **Build:** Cursor, Superpowers TDD, Ponytail **full** (`ponytail-juce.mdc`), REA on Release `.pdb`
- **Intel:** OpenClaw + Firecrawl (`vst_market_intel`)
- **Launch:** AutoGPT on `Assets/Releases/` drops only
- **No new agent frameworks** until Lite VST3 ships (`STACK-FROZEN.md`)

## Active engineering

- **Product:** Detroit Phonk Trap Maker — BR 808, NOLA bounce, phonk clip
- **DSP:** `disklordz-ralph/vst/Modules/TrapCoreDSP/`
- **Plugin UI:** `disklordz-ralph/vst/starter-cpp/Source/` — `Trap808Voice`, pad drag WAV/MIDI (`DLZ-VST-007`)
- **Windows tree:** `C:\Dev\DetroitPhonkTrapVST3` (JUCE 8, Catch2, pluginval 5)
- **Installer:** `disklordz-ralph/vst/installer/*.iss`

## RT rules (non-negotiable)

- No heap/locks/file I/O in `processBlock`
- Oversampled clip on nonlinear 808 path before v1 Core (see `shortcut:` in processor)
- Catch2 before production DSP changes

## Paths

- `DLZ_REPO_ROOT` = git root (`push`)
- Kits → `Assets/Releases/` + `kit_manifest.json`
- Market → `docs/market-research/`, outreach → `docs/outreach/`