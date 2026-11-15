# Hybrid agent stack — commercial VST + drum kits

**One sentence:** **Cursor (or Claude Code) owns C++/Git/pluginval**; **Python autoresearch owns batch drums**; **RAGFlow owns retrieval** (schematics, licensing copy, specs) — connected via MCP or export, not as a third build system.

Aligned with `disklordz-ralph/STACK-4-LAYER.md` and `STACK-FROZEN.md` (no new agent frameworks on the Lite ship path).

---

## Stack map

| Business area | Primary tool | Repo entry |
|---------------|--------------|------------|
| **C++ DSP, VST3 stability** | Cursor + Superpowers + REA + Ponytail | `vst/starter-cpp/`, `ponytail-juce.mdc` |
| **Build + pluginval gate** | Local terminal (Claude Code / Cursor) | `scripts/vst-build-pluginval-gate.ps1` |
| **Kit synthesis + matching** | Python autoresearch + Drum Forge | `factory/autoresearch/`, `factory/drum-forge-v4/` |
| **Circuit / textbook research** | RAGFlow (optional MCP) | `docs/dev/autoresearch-ragflow.md` |
| **Storefront, manuals, support drafts** | RAGFlow or Gemini CLI | `storefront/`, `GEMINI.md`, OpenClaw copy |
| **Launch cadence** | AutoGPT (opt-in Friday) | `openclaw/autogpt/` |
| **Local multi-agent (Claude Code)** | Ruflo (opt-in) | `CLAUDE.md`, `docs/dev/ruflo-opt-in.md` |
| **Finance & market ops** | OpenBB Workspace (opt-in) | `docs/dev/openbb-opt-in.md`, `business/openbb/` |
| **Video / social launch** | OpenMontage (opt-in) | `docs/dev/openmontage.md` |
| **Security red team** | Strix (opt-in) | `docs/dev/strix-opt-in.md`, `docs/security/strix-vst-audit-playbook.md` |
| **PRD & web scaffold** | MetaGPT (opt-in) | `docs/dev/metagpt-opt-in.md`, `docs/plans/metagpt-license-portal-blueprint.md` |

---

## Why not RAGFlow for C++?

RAGFlow’s code sandbox is **ephemeral** and not your Git checkout. It cannot reliably:

- Run MSVC + JUCE CMake on your Windows tree
- Execute **pluginval** on a real `.vst3`
- Enforce Catch2 / REA RT rules

Use RAGFlow to **retrieve** (filter topology, licensing clause, FAQ from specs), then let the **CLI agent** apply the smallest C++ diff and run the gate script.

---

## Commercial C++ guardrails (paid plugin)

Expand any “autoresearch” loop beyond timbre:

| Gate | Tool | Fail action |
|------|------|-------------|
| **VST3 validity** | pluginval (default **5**, stricter **8** optional) | Revert DSP commit |
| **RT safety** | REA + Ponytail on `processBlock` | No merge |
| **CPU budget** | Block timing harness (below) | Revert if over ceiling |
| **Kit originality** | Spectral distance floor in Python judge | Reject batch export |

Details: `docs/dev/vst-commercial-gates.md`.

---

## Drum kit commercial path

1. **Python autoresearch** → `drum_engine.py` winners  
2. **Drum Forge v4** / future render CLI → 24-bit WAV + manifest  
3. **Spectral floor** — targets similarity *and* minimum distance to references (no cloning)  
4. **OpenClaw / AutoGPT** — human-approved storefront copy only  

---

## Operator weekly rhythm

| Days | Agent |
|------|--------|
| Mon–Wed | Cursor: one `prd.json` story → `vst-build-pluginval-gate.ps1` → commit |
| Ongoing (optional) | Python autoresearch overnight on kit voices |
| Thu | OpenClaw intel |
| Fri | AutoGPT staged copy (approve) |
| Research spikes | RAGFlow 10–30 iteration canvas with SSH runner |

---

## Choose your next doc

| Need | Read / run |
|------|------------|
| **C++ build + pluginval loop** | `scripts/vst-build-pluginval-gate.ps1` + `docs/dev/vst-commercial-gates.md` |
| **RAGFlow DSP + business index** | `docs/dev/ragflow-knowledge-base.md` |
| **Python kit autoresearch** | `factory/autoresearch/program.md` |