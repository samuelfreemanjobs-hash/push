# Herdr pane coordination — DiskLordz

Paste into **each pane’s** agent system context (Claude Code `CLAUDE.md` snippet, Cursor rules, or Goose recipe header).

**Session name:** `DiskLordzStudio` (or `DLZ_HERDR_SESSION`)

---

## Pane roles

| ID | Role | CWD | Stop condition |
|----|------|-----|----------------|
| `0.0` | **DSP_Engineer** | `DLZ_VST_PROJECT_ROOT` (default `drumkit-plugin`) | `SWARM_VERIFIED_COMPLETE` or green cmake build |
| `0.1` | **Factory_Runner** | `DLZ_AUDIO_FACTORY_ROOT` | `ALL_FACTORIES_COMPLETE` or manifest written |
| `0.2` | **Build_QA** | Repo root or ship `starter-cpp` | pluginval exit 0 |

---

## Handoff tokens (grep-friendly)

Emit exactly one line when done so **herdr-mesh** or humans can route:

| Token | Meaning | Next pane action |
|-------|---------|------------------|
| `HANDOFF_QA:build` | DSP compiles | Run `vst-build-pluginval-gate.ps1` or `validate_vst3` |
| `HANDOFF_FACTORY:presets` | DSP benchmark OK | Run preset leg of factory or `/disk-marketing-kit` |
| `HANDOFF_DSP:fix` | pluginval failed | Paste log to pane 0.0 |
| `HANDOFF_MARKETING:drafts` | `dist_sample_pack/manifest.json` ready | `/disk-marketing-kit` in any pane |
| `HUMAN_GATE:*` | Blocked | Mobile approve; do not auto-send |

---

## Context7

DSP pane: enable Context7 MCP before JUCE edits (`docs/dev/elite-dsp-agent-context7.md`).

---

## Forbidden cross-pane writes

- Factory pane must **not** edit `vst/starter-cpp/` ship tree.
- Marketing pane must **not** commit `storefront/*.html` without `HUMAN_GATE:storefront_approved` cleared.
- QA pane: **read-only** except `build/` and test logs.

---

## Mesh delegation (optional)

When `herdr-mesh` MCP is available:

1. **Read** sibling pane output before duplicating cmake.
2. **Spawn** only with operator-approved recipes (no infinite pane fork).
3. Prefer repo scripts (`run-vst-rd-swarm.ps1`, `run-audio-factory-swarm.ps1`) over ad-hoc prompts.

---

## Repo anchor

`DLZ_REPO_ROOT` = git root of `push`. All relative paths resolve there.