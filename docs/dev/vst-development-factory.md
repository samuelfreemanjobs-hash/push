# DiskLordz VST Development Factory

A coordinated system for **architecture → implementation → proof** on the frozen Lite path (`vst/starter-cpp/`) and opt-in labs (neural GPU, drum factory). AI agents are specialized; humans keep merge and storefront gates.

**Phase 1 (investigate first):** [EGM](#phase-1-egm), [MCode](#phase-1-mcode), [Waza](#phase-1-waza) — spec quality, coding consistency, objective evals.

**Phase 2:** automated QA matrix + [Helix knowledge bank](#engineering-knowledge-bank).

Cross-links: `docs/dev/elite-frameworks-implementation.md`, `openclaw/elite-frameworks/manifest.yaml`, `disklordz-ralph/vst/templates/drumkit-plugin/AGENTS.md`.

---

## Agent roles (factory floor)

| Role | Owns | Repo anchor | Gate |
|------|------|-------------|------|
| **C++ architecture** | Threading, APVTS, module boundaries | `labs/gpu-inference/`, plugin `src/` | EGM duel **before** merge |
| **DSP implementation** | `processBlock`, oversampling, soft clip | `vst/starter-cpp/Source/`, Good Money lab | MCode `CHECKLIST-vst-dsp.md` + REA |
| **JUCE GUI** | Editor, WebView faceplate parity | `PluginEditor.cpp`, `ui/`, `storefront/plugin-faceplate/` | Agent Desktop smoke (opt-in) |
| **CMake / builds** | Targets, JUCE linkage, CI | `CMakeLists.txt`, `scripts/vst-build-pluginval-gate.ps1` | QA pod only |
| **Presets / kits** | APVTS XML, WAV maps, ZIP | `factory/audio-factory/`, `trap-phonk-factory/` | Factory Runner pane |
| **pluginval / scan** | VST3 validity, strictness | pluginval script | **Hard gate** — no commercial ZIP without pass |
| **Release packaging** | Installers, manifests, human storefront | `openduet/approval_gates.yaml` | `HUMAN_GATE:*` |

OpenClaw registry: `openclaw/agents/REGISTRY.yaml` (`dev`, `review`, `drum-lead`, subagents).

Herdr 3-pane protocol (Cursor/Claude Code): `disklordz-ralph/vst/templates/drumkit-plugin/AGENTS.md`.

---

## Phase 1 — EGM {#phase-1-egm}

**Problem:** AI over-builds neural paths or drifts RT rules.

**Use when:** New feature touches `processBlock`, threads, GPU, or factory manifests.

```bash
# Copy template for greenfield features
cp openclaw/elite-frameworks/egm/vst-architecture-spec.template.md docs/specs/my-feature.md

# Duel before any C++ branch
python openclaw/elite-frameworks/egm/run_spec_duel.py --spec docs/specs/my-feature.md
```

**Pass criteria:** No `error` findings; RT safety + acceptance + test plan present for VST specs.

**Reference spec (already PASS):** `disklordz-ralph/labs/gpu-inference/README.md`

OpenClaw: invoke skill `egm_spec_challenge`.

---

## Phase 1 — MCode {#phase-1-mcode}

**Problem:** Inconsistent diffs, skipped pluginval, scope creep on PRs.

**Every DSP session:**

1. `openclaw/elite-frameworks/mcode/workflows/cursor-session.md`
2. `mcode/CHECKLIST-vst-dsp.md` on commit
3. `mcode/CHECKLIST-pr.md` before push

**Ship path rule:** Smallest correct diff; match surrounding naming and types; no new frameworks on Lite path (`STACK-FROZEN.md`).

---

## Phase 1 — Waza {#phase-1-waza}

**Problem:** “Looks done” without contract checks.

**After every agent merge or local session:**

```bash
bash openclaw/elite-frameworks/waza/evals/run_all.sh
```

**Extend** `waza/evals/test_agent_artifacts.py` when you add new contracts, e.g.:

- Preset XML schema version
- `release_manifest.json` required fields
- Neural panel header still forbids RT inference

OpenClaw: skill `waza_agent_eval` on `dev` and `review` pods.

---

## Automated plugin QA (phase 2 roadmap)

| Layer | Today | Next increment |
|-------|--------|----------------|
| Compile + link | `vst-build-pluginval-gate.ps1` | CI matrix Win/mac |
| pluginval / scan | pluginval strictness 5–10 | Nightly on `main` opt-in |
| State save/restore | Manual DAW | Catch2 + `AudioProcessor` round-trip test |
| MIDI / MPE | Manual | Scripted MIDI inject in offline harness |
| Audio regression | `render_milestone1.cpp`, Python spectral judges | Golden WAV diff tolerances |
| Performance | Block timing harness (`vst-commercial-gates.md`) | Budget per plugin SKU |
| GUI | `storefront/plugin-faceplate/` + Agent Desktop README | computerUse smoke scripts |

Waza should gain one pytest/assert per new **machine-checkable** gate (not vibes).

---

## Engineering knowledge bank

**Goal:** One queryable graph from specs → DSP → bugs → checklists.

| Need | Tool in repo |
|------|----------------|
| Structured graph | `openclaw/elite-frameworks/helix-foundry/seed/vst-helix-seed.json` + schema |
| Ingest manuals / SDK docs | `document-parsing-protocol/ingest.py` → JSON chunks |
| Pane-local memory | `.serena/memories/` under drumkit template |
| Commercial gates | `docs/dev/vst-commercial-gates.md`, Strix skill |
| Lessons / bugs | Add Helix nodes `kind: bug` with `rel: blocks` to specs |

**Operator habit:** On every merged architecture PR, add or update one Helix node + edge (`implements` / `tests`).

RAGFlow opt-in: export Helix chunks + DPP output — `docs/dev/ragflow-knowledge-base.md`.

---

## Suggested week-one investigation

| Day | Focus | Done when |
|-----|--------|-----------|
| 1 | EGM on gpu-inference + one new `docs/specs/*.md` | Duel PASS saved in PR template |
| 2 | MCode checklists on a tiny DSP tweak (starter-cpp or lab) | pluginval + checklist cited in PR |
| 3 | Waza — add one custom assert for your next contract | `run_all.sh` green in CI or pre-push habit |
| 4 | Helix — 3 nodes (spec, dsp_ref, test) for that feature | Seed JSON updated |
| 5 | CoworkOS `projects.yaml` milestone tick + OpenDuet gate for merge | Human merge only |

---

## What not to automate first

- Live checkout / license keys (OpenDuet + Vercel security checklist first)
- DAW UI driving without operator consent
- RE of third-party plugins (`reverse-engineer-anything/LEGAL.md`)

Lite customers still ship from **starter-cpp + pluginval**; factory layers accelerate R&D and kits, not replace the gate.

---

## Automation (CI + cron)

| Trigger | Command |
|---------|---------|
| **Every PR / push** | GitHub Actions job `vst-factory-pipeline` |
| **Local / VPS cron** | `bash openclaw/elite-frameworks/automation/run_factory_pipeline.sh` |
| **Free agent-stack** | `disklordz-agent-stack` → `vst-factory-automation.mjs` inside `npm run all` |
| **OpenClaw** | Merge `openclaw/openclaw.disklordz.vst-factory.json5.example` — daily `vst-qa` message |
| **All pods finish** | `bash openclaw/elite-frameworks/automation/factory_finish_ship.sh` |

**Pods registry:** `openclaw/agents/vst-factory-pods.yaml`  
**Emit OpenClaw JSON:** `node openclaw/scripts/apply-vst-factory-agents.mjs`  

Delegation chain: architect → dsp → gui → build → preset → qa → release (human on release).