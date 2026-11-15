# Multi-Agent Studio Coordination Protocol (`herdr-mesh`)

You operate in a 3-pane **Herdr** workspace (`DrumKitStudio` / `DiskLordzStudio`) building **JUCE 8 VST3**, **WAV sample packs**, and preset banks for **DiskLordz** (original synthesis — no copyrighted samples).

Repo anchor: set `DLZ_REPO_ROOT` to the `push` git root. Factory outputs live under `disklordz-ralph/factory/audio-factory/` unless copied here.

---

## 1. Strict pane ownership (prevent file conflicts)

### Pane 0 — `DSP_Architect` (Claude Code + Context7)

**May write:**

- This folder: `CMakeLists.txt`, `src/**`, `ui/**`, `vst_rd_swarm.py`
- Good Money / lab headers only when task explicitly targets those trees

**Rules:**

- Query **Context7** before JUCE 8 API or CMake macro edits.
- Zero heap allocation, locks, or `juce::String` work in `processBlock()`.
- On compile-ready change, emit `HANDOFF_QA:build` and mesh-send build line to **Pane 2**.

### Pane 1 — `Factory_Runner` (Goose / AutoGen / Python)

**May write:**

- `disklordz-ralph/factory/audio-factory/raw_audio/*`
- `.../dist_sample_pack/*`, `.../dist_presets/*`
- `docs/launch/*` (marketing drafts only)

**Runs:**

- `python factory_swarm.py` or `process_and_master_sample_pack`
- APVTS XML presets, `DrumSampleMap.xml`, `velocity_layers.json`
- Future: MPC `.xpm`, FFmpeg promos — **only after** QA success

**Must not** package SKUs until Pane 2 prints `PLUGINVAL_PASS` or `HANDOFF_FACTORY:presets` acknowledged.

### Pane 2 — `QA_And_Release_Gate` (CMake + pluginval)

**May write:** `build/*` under this plugin dir, test logs, `release_manifest.json` stub under `dist_release/` (create if packaging).

**Runs:**

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DDLZ_USE_MONOREPO_JUCE=ON
cmake --build build --config Release --parallel
$Pv = $env:DLZ_PLUGINVAL ?? "..\..\..\vst\tools\pluginval\pluginval.exe"
& $Pv --strictness-level ($env:PLUGINVAL_STRICTNESS ?? "10") --validate `
  "build\DrumKitPlugin_artefacts\Release\VST3\DrumKit Pro.vst3"
```

**Shopify / Gumroad / Stripe:** draft-only via `HUMAN_GATE:storefront_approved` — never live publish from this pane.

---

## 2. Automated cross-pane handoff (`herdr-mesh` MCP)

Tools: `list_panes`, `read_pane`, `send_to_pane`.

| Step | From | To | Action |
|------|------|-----|--------|
| 1 | Pane 0 | `DrumKitStudio:0.2` | After C++ change → cmake build + pluginval command above |
| 2 | Pane 2 | `DrumKitStudio:0.1` | On `SUCCESS` / `HANDOFF_FACTORY:presets` → presets + pack from latest APVTS |
| 3 | Pane 2 | `DrumKitStudio:0.0` | On failure → last **40** lines of MSVC/pluginval log, `HANDOFF_DSP:fix` |

**Failure rule:** Pane 1 must **not** export commercial ZIPs if Pane 2 failed.

---

## 3. Token vocabulary

| Token | Meaning |
|-------|---------|
| `HANDOFF_QA:build` | DSP ready for compile/validate |
| `HANDOFF_FACTORY:presets` | pluginval passed; factory may run |
| `HANDOFF_DSP:fix` | Build/validate failed |
| `HANDOFF_MARKETING:drafts` | `manifest.json` ready → `/disk-marketing-kit` |
| `HUMAN_GATE:*` | Stop; wait for operator (mobile or desktop) |

---

## 4. Ship path firewall

Do **not** edit `disklordz-ralph/vst/starter-cpp/` (Detroit Phonk ship) unless `DLZ_SWARM_ALLOW_SHIP=1` and operator approval.

---

## 5. More docs

- Monorepo: `/AGENTS.md` at repo root
- Herdr install: `docs/dev/herdr-opt-in.md`
- Elite DSP: `docs/dev/elite-dsp-agent-context7.md`