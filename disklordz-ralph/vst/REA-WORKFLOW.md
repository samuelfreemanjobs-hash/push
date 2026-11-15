# REA (Reverse Engineer Anything) — DiskLordz JUCE workflow

Treat REA as an **evidence-based auditor**, not a clone button. **Clean-room** your own binaries and permissive OSS only — never DRM circumvention or proprietary sample extraction.

**Your stack (default):**

| Layer | Tool |
|-------|------|
| OS / build | **Windows 11**, MSVC **VS 2022**, `C:\Dev\DetroitPhonkTrapVST3` |
| Plugin ship | Release `.vst3` + **RelWithDebInfo** or Debug+PDB for audits |
| Business repo | **Cursor** (cloud + desktop), branch `cursor/disklordz-command-center-634f` |
| Ralph loop | **Claude Code** (`claude -p`, `disklordz-ralph/ralph.ps1`) |
| REA backend | **Ghidra** (Windows/cross-platform) |

---

## 1. One-time REA setup (Windows)

```powershell
# Node 24+ required (same as OpenClaw)
npm install -g rea-agents@latest
npx rea-agents@latest setup
rea update

# Install Ghidra: https://ghidra-sre.org/
# Point REA at Ghidra when setup wizard asks (see rea doctor)

rea doctor --client claude_code --json
rea doctor --client cursor --json
```

**JUCE builds for REA:**

```powershell
cmake --build build --config RelWithDebInfo --parallel
# PDB + symbols: audit build\...\Detroit Phonk Trap Maker.vst3
# Release: pluginval + customer smoke only
```

Compile flags: `starter-cpp/CMakeLists.txt` (Release `/Zi` + link `/DEBUG`). Prefer **RelWithDebInfo** for Ghidra. Paths: `WINDOWS-BUILD.md`. Register in `rea-targets.json` (copy from `rea-targets.example.json`).

---

## 2. Scope rules (avoid context death)

Never feed the whole `.vst3` bundle as “analyze everything.”

| Do | Don’t |
|----|--------|
| Filter symbols: `*processBlock*`, `*renderNextBlock*`, `*Trap808*`, `*SoftClip*` | Full-binary decompile of static JUCE |
| Start at exports: `GetPluginFactory`, module entry | PNG/zlib/JUCE graphics paths |
| One subsystem per session | FL bug + clipper + browser in one prompt |
| Write results to `vst/rea-notes/YYYY-MM-DD-<topic>.md` | Re-decompile same SHA every Ralph iteration |

Artifact registry (optional): record SHA-256 of audit `.vst3` in `vst/rea-notes/artifacts.yaml`.

---

## 3. Starter prompts (copy/paste)

### A. Real-time safety (your binary)

```text
Target: C:\Dev\DetroitPhonkTrapVST3\build\...\Detroit Phonk Trap Maker.vst3
Scope: audio thread only — processBlock, renderNextBlock, and callees one level deep.
Evidence format: observation / inference / unknown.

Tasks:
1. List any code paths that call malloc, operator new, free, CriticalSection, mutex, or file I/O.
2. Flag Objective-C / COM sends on the audio thread (should be none on Windows).
3. Confirm oversampler processSamplesUp/Down are not allocating per block.

Output: bullet table with symbol name, address if known, severity, suggested Source/ fix.
Save summary to disklordz-ralph/vst/rea-notes/rt-safety-latest.md
```

### B. Oversample + clip order (your binary or ChowDSP OSS)

```text
Scope: functions matching *Oversampl*, *WaveShaper*, *SoftClip*, *tanh*.
Question: Is oversampling strictly before the waveshaper on all channels?
Compare signal-flow order to starter-cpp/TrapSoftClipChain.h — diff in prose only, no copied ASM.
```

### C. FL Studio / MIDI (your binary)

```text
Scope: pitch wheel, note on/off, legato, IMidiMapping-related strings and handlers.
Question: Where is pitch-bend range applied? Any host-specific branches for FL?
Cross-check disklordz-ralph/vst/docs/FL-STUDIO-QA.md.
```

### D. Sample browser (when you ship 10k WAV index)

```text
Scope: SQLite, waveform peak cache, background thread entry points only — not entire UI module.
Reference: permissive OSS preset browser (clean-room patterns only).
Deliver: thread model diagram + what to precompute on message thread.
```

---

## 4. Pair REA with Ralph (`prd.json`)

| After story | REA session |
|-------------|-------------|
| `DLZ-VST-004` clip | Prompt B on RelWithDebInfo binary |
| `DLZ-VST-005` glide | Prompt A on `Trap808Voice` symbols |
| `DLZ-VST-003` pluginval pass | Prompt A full RT audit |
| `DLZ-VST-008` | Prompt C + manual FL checklist |

Add to Ralph `PROMPT.md` iteration: if story is `DLZ-VST-*` and audit build exists, append REA summary path to `progress.txt`.

---

## 5. Client-specific hooks

### Claude Code (Ralph on Windows)

`%USERPROFILE%\.claude\settings.json` or project `.mcp.json` — add REA MCP per `npx rea-agents@latest setup`.

Ralph one-liner after build:

```powershell
$env:REA_TARGET_VST3 = "C:\Dev\DetroitPhonkTrapVST3\build\...\Detroit Phonk Trap Maker.vst3"
claude -p "Run REA scoped audio-thread audit per disklordz-ralph/vst/REA-WORKFLOW.md §3A on REA_TARGET_VST3"
```

### Cursor (push repo)

- MCP: enable REA from setup; workspace root `C:\Dev\push`.
- **Do not** attach competitor commercial VST3s — OSS only in cloud agents.
- Use `@disklordz-ralph/vst/starter-cpp/` + `rea-notes/` for context.

### OpenClaw / Telegram

- `DRUM:` / `DEV:` — “paste REA rt-safety-latest.md summary” → sage reads `hermes-out` + repo notes, does not run REA on VPS unless Ghidra installed.

---

## 6. Pitfalls (short)

1. **Huge binaries** → symbol filter first (`GetPluginFactory` → your processor class).
2. **Token burn** → cache in `vst/rea-notes/`; commit notes, not binaries.
3. **Legal** → your builds + permissive OSS (Surge, Vital, ChowDSP **concepts**); no DRM, no rival pack ripping.
4. **False positives** → JUCE nested templates; confirm in `Source/` before “fixing” phantom allocs.

---

## 7. Files in repo

| Path | Role |
|------|------|
| `starter-cpp/` | Clean-room reference implementation |
| `rea-notes/README.md` | Where agents write REA summaries |
| `rea-targets.example.json` | Local paths (copy to `rea-targets.json`, gitignored) |
| `TRAP-PHONK-ARCHITECTURE.md` | DSP + product map |