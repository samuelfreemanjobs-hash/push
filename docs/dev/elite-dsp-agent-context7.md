# Elite DSP agent — Context7 + RT guardrails + pluginval

DiskLordz template for Claude Code and Cursor when editing **JUCE 8 / C++20** audio code.

| Surface | File |
|---------|------|
| **Cursor (VST globs)** | `.cursor/rules/audio-dsp.mdc` |
| **Claude Code (path rules)** | `.claude/rules/elite-dsp-juce8.md` |
| **RT reviewer subagent** | `.claude/agents/dsp-rt-auditor.md` |
| **Slash command** | `/disk-rt-audit` |
| **Skill** | `dsp-rt-verification` |

Root **`CLAUDE.md`** stays swarm-short; elite DSP detail lives here (do not duplicate into a second 200-line CLAUDE).

---

## 1. Context7 protocol

When Context7 MCP is connected:

1. Query JUCE 8 + CMake **before** new `juce_add_plugin` targets, `juce::dsp::` chains, APVTS attachments, WebView2 UI.
2. Never invent method names; confirm against live docs.
3. Re-query after JUCE submodule bumps (`externals/JUCE` tag 8.0.x).

**House library index** (reuse, don't reimplement):

| Library | Path |
|---------|------|
| Trap ship DSP | `disklordz-ralph/vst/starter-cpp/Modules/TrapCoreDSP/` |
| Three-layer lab core | `factory/drum-architecture/Source/` |
| Autoresearch kick | `vst/autoresearch-cpp/Source/VoiceDSP.h` |
| Genre labs | `tm88-drum-lab/`, `maybach-drum-lab/`, `luger-drum-lab/` |

Optional: add a private Context7 index repo for shared velocity curves and UI SVG knobs later.

---

## 2. Real-time rules (summary)

See `.cursor/rules/audio-dsp.mdc` for the full table. Agents must grep diffs for:

- `new `, `malloc`, `push_back`, `juce::String` inside `processBlock`
- `CriticalSection`, `std::mutex`, `loadFileAsString` on audio thread
- Raw `getRawParameterValue` without smoothing for audible params
- Missing `ScopedNoDenormals`

---

## 3. pluginval in the agent loop

| Strictness | When |
|------------|------|
| **5** | Daily / `DLZ-VST-003` |
| **8** | Pre-release, agent keep commits, OpenMontage capture builds |
| **10** | Optional extra fuzz (operator only; slower) |

```powershell
.\scripts\vst-build-pluginval-gate.ps1
# DrumSynth preview:
.\scripts\vst-build-drumsynth-autoresearch.ps1
```

Judge + pluginval: `evaluate_vst.py` (immutable).

---

## 4. Three upgrades (elite setup)

### A. Dedicated RT auditor

- **Claude:** `/disk-rt-audit` → `dsp-rt-auditor` agent
- **Cursor:** `@audio-dsp` + ask for RT diff review
- Skill: `.claude/skills/dsp-rt-verification/SKILL.md`

### B. Context7 + pluginval pairing

Agent workflow after C++ edits:

1. Context7 lookup (if MCP on)
2. `cmake --build` + `ctest`
3. `pluginval` at `PLUGINVAL_STRICTNESS`
4. Only then update PR summary

**AutoGen closed loop (operator):** `vst_rd_swarm.py` / `scripts/run-vst-rd-swarm.ps1` — see `docs/dev/ms-agent-framework-opt-in.md`.

**Serena symbolic refactor (operator):** `vst/templates/drumkit-plugin/setup-serena.ps1` + `.serena/memories/dsp_refactoring_sop.md` — see `docs/dev/serena-context7-juce.md`.

### C. Index proprietary DSP

Keep shared blocks in-repo (`TrapCoreDSP`, `ThreeLayerDrum`, lab `*Identity.h`). External private repo can be added to Context7 when you have one.

---

## 5. WebView2 / future UI

WebView rotary knobs or storefront embeds: Context7 **WebBrowserComponent** + isolate bridge on message thread; never eval JS from `processBlock`.

## 6. CMake reference (already in repo)

| Target | CMake entry |
|--------|-------------|
| Ship VST3 | `disklordz-ralph/vst/starter-cpp/CMakeLists.txt` |
| **WebView2 + pluginval template** | `vst/templates/webview-drum-starter/` — `docs/dev/juce8-cmake-webview-starter.md` |
| Autoresearch + DrumSynth preview | `-DDLZ_AUTORESEARCH=ON`, `autoresearch-cpp/CMakeLists.txt` |
| Drum labs | `maybach-drum-lab/CMakeLists.txt` |

WebView2 NuGet: `scripts/install-webview2-nuget.ps1`. pluginval: `validate_vst3` target or `vst-build-pluginval-gate.ps1`.

---

## Related

- `docs/dev/juce-cmake-windows-troubleshooting.md`
- `docs/dev/claude-code-best-practice.md`
- `disklordz-ralph/vst/SUPERPOWERS.md`