# Ruflo Audio DSP & Drum Kit Swarm — DiskLordz

**Stack policy:** `disklordz-ralph/STACK-FROZEN.md` (Lite VST3 ship first). Engineering: **`AGENTS.md`**.

## Project mission

Build, validate, and optimize:

1. **C++ VST path** — commercial JUCE stack + autoresearch sandbox `disklordz-ralph/vst/autoresearch-cpp/Source/VoiceDSP.h`
2. **Python kit path** — `disklordz-ralph/factory/autoresearch/drum_engine.py` → royalty-free 24-bit WAV kits

**Source of truth for C++ iterations:** `python disklordz-ralph/vst/autoresearch-cpp/evaluate_vst.py` (build + pluginval + `TOTAL_LOSS`).  
**Source of truth for Python kit iterations:** `python disklordz-ralph/factory/autoresearch/evaluate.py`.

---

## Swarm agent roles (hierarchical topology, max 4–6)

1. **Queen coordinator** — Reads `results.tsv` (per track) + Ruflo vector memory; assigns **one** hypothesis per cycle; enforces git keep/revert; never edits DSP.
2. **DSP engineer (`dsp-architect`)** — **ONLY** edits:
   - `disklordz-ralph/vst/autoresearch-cpp/Source/VoiceDSP.h` (C++20), **or**
   - `disklordz-ralph/factory/autoresearch/drum_engine.py` (Python)  
   Real-time rules: zero heap allocs in `processBlock`, no locks, no `std::cout` on audio thread, `juce::ScopedNoDenormals`.
3. **QA & pluginval gatekeeper (`qa-validator`)** — Runs `evaluate_vst.py`; ensures `pluginval` pass (`PLUGINVAL_STRICTNESS` default 5, pre-release **8**); `CPU_RENDER_MS < 15`; no `ERROR:` in judge output.
4. **Sample pack producer (`kit-producer`)** — After a **kept** Python or C++ DSP commit:
   - **C++ velocities:** `scripts/export_kit_velocities.ps1` → `autoresearch-cpp/outputs/kits/`
   - **Commercial pack:** `python disklordz-ralph/factory/autoresearch/export_kit.py` (add `--sfz` for sampler patch)
   - **Trigger (coordinator):** when `TOTAL_LOSS` beats prior best **or** kick-only `SPECTRAL_LOSS < 0.35` (Python kit path), run `export_kit.py` (auto-stages OpenMontage unless `SKIP_OPENMONTAGE_PUBLISH=1`); pass `release_brief.json` to docs agent (Gumroad/Lemon copy) and optional `media-studio` — human approves storefront and video checkpoints.
5. **Docs / storefront** (optional 5th) — `storefront/`, `PRODUCT.md` only; human approves paid copy.
6. **Media launch (`media-studio`, optional)** — After `export_kit.py`, run `python disklordz-ralph/factory/autoresearch/publish_to_openmontage.py` (or `export_kit.py --handoff` / `scripts/handoff-pack-to-openmontage.ps1`). Open sibling `../marketing-studio` and execute **`assets/latest_release/AGENT_PROMPT.md`** (Templates A/B in `docs/marketing/openmontage-prompt-templates.md`); **human approves** storyboard and draft render before final MP4. See `docs/dev/openmontage.md`.

### Marketing handoff rule (`kit-producer`)

Immediately after `python disklordz-ralph/factory/autoresearch/export_kit.py` succeeds, run `python disklordz-ralph/factory/autoresearch/publish_to_openmontage.py` to stage `demo_beat.wav`, hero one-shots, `manifest.json`, and `release_brief.json` into `../marketing-studio/assets/latest_release/`.

Ship plugin merge target: `TrapCoreDSP` / `PluginProcessor` — **not** in autoresearch loop. Wiring reference: `autoresearch-cpp/examples/PluginProcessorAutoresearch.example.cpp`.

---

## Hard guardrails

| Never edit | Reason |
|------------|--------|
| `evaluate_vst.py`, `factory/autoresearch/evaluate.py` | Immutable judges |
| `Source/HeadlessMain.cpp`, `autoresearch-cpp/CMakeLists.txt` | Harness |
| `targets/*.wav` | Reference masters |
| `PluginProcessor.cpp`, `TrapCoreDSP/**` | Ship path — human merge after wins |

**Originality floor:** env `MIN_SPECTRAL_CONVERGENCE=0.08` (judge penalizes cloning). Synthesized only — no copyrighted samples.

**Git discipline (C++ sandbox, from repo root):**

```bash
# WIN — from repo root
git add disklordz-ralph/vst/autoresearch-cpp/Source/VoiceDSP.h disklordz-ralph/vst/autoresearch-cpp/results.tsv
git commit -m "DSP win: <TOTAL_LOSS> — <summary>"
```

```bash
# FAIL
git checkout -- disklordz-ralph/vst/autoresearch-cpp/Source/VoiceDSP.h
```

**Python kit** — same pattern for `factory/autoresearch/drum_engine.py` + `factory/autoresearch/results.tsv`.

---

## Autopilot prompt (Claude Code)

```text
Read CLAUDE.md. Query Ruflo memory for dsp_constraints and sonic_targets.
Spawn dsp-architect + qa-validator. Loop on disklordz-ralph/vst/autoresearch-cpp/Source/VoiceDSP.h
using: python disklordz-ralph/vst/autoresearch-cpp/evaluate_vst.py
Log disklordz-ralph/vst/autoresearch-cpp/results.tsv. On keep, store winning formulas in Ruflo memory.
kit-producer runs scripts/export_kit_velocities.ps1 only after qa-validator passes pluginval.
```

---

## Paths

| Item | Path |
|------|------|
| CMake + VST3 | `disklordz-ralph/vst/starter-cpp/` + `-DDLZ_AUTORESEARCH=ON` |
| Kick target | `autoresearch-cpp/targets/kick.wav` or `factory/autoresearch/targets/kick.wav` |
| Best render | `autoresearch-cpp/outputs/rendered_kick.wav` |
| Kit velocities | `autoresearch-cpp/outputs/kits/` |
| Full launch | `docs/dev/ruflo-launch.md` |
| OpenMontage | `docs/dev/openmontage.md`, `docs/marketing/openmontage-prompt-templates.md` |

---

## Security

Copy `.claude/settings.json.example` → `.claude/settings.json` after `npx ruflo@latest init`. See `docs/dev/ruflo-opt-in.md`.