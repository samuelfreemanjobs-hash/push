# Ruflo Audio DSP & Drum Kit Swarm — DiskLordz

**Stack policy:** `disklordz-ralph/STACK-FROZEN.md` (Lite VST3 ship first). Cursor: **`AGENTS.md`**.  
**Claude Code layout:** `.claude/` (rules, agents, commands, skills) — see **`docs/dev/claude-code-best-practice.md`**.

## Project mission

1. **C++ VST path** — JUCE ship tree + autoresearch `disklordz-ralph/vst/autoresearch-cpp/Source/VoiceDSP.h`
2. **Python kit path** — `disklordz-ralph/factory/autoresearch/drum_engine.py` → 24-bit WAV kits

**Judges (immutable):**

- C++: `python disklordz-ralph/vst/autoresearch-cpp/evaluate_vst.py`
- Python: `python disklordz-ralph/factory/autoresearch/evaluate.py`

---

## Swarm roles

Detailed role matrix: **`.claude/rules/ruflo-swarm.md`**. Subagent defs: **`.claude/agents/`**.

| Role | Sandbox |
|------|---------|
| `dsp-architect` | `VoiceDSP.h` or `drum_engine.py` only |
| `qa-validator` | judges + pluginval (+ Strix quick before paid) |
| `kit-producer` | `export_kit.py`, OpenMontage handoff |
| `memory-curator` | Mem0 recall/store (`docs/dev/mem0-opt-in.md`) |

Slash workflows: `/disk-autoresearch-cpp`, `/disk-mem0-recall`, `/disk-pluginval-gate`, `/disk-export-kit`.

---

## Elite DSP (JUCE 8 + Context7)

Cursor: **`.cursor/rules/audio-dsp.mdc`**. Claude Code: **`.claude/rules/elite-dsp-juce8.md`**, **`/disk-rt-audit`**, agent **`dsp-rt-auditor`**.  
Playbook: **`docs/dev/elite-dsp-agent-context7.md`** — Context7-first, RT rules, pluginval 5/8/10.

---

## Hard guardrails

| Never edit | Reason |
|------------|--------|
| `evaluate_vst.py`, `factory/autoresearch/evaluate.py` | Judges |
| `HeadlessMain.cpp`, harness CMake | Harness |
| `targets/*.wav` | Reference masters |
| `PluginProcessor.cpp`, `TrapCoreDSP/**` | Ship path — human merge |

**Originality:** `MIN_SPECTRAL_CONVERGENCE=0.08`. Synthesized only.

**Git (win/fail):** skills `cpp-voice-dsp-loop`, `python-kit-loop` under `.claude/skills/` and `factory/autoresearch/.claude/skills/`.

---

## Autopilot (Claude Code)

```text
Read CLAUDE.md + .claude/rules for current paths.
Run /disk-autoresearch-cpp or spawn dsp-architect + qa-validator per claude-code-best-practice.
On kit win: /disk-export-kit. Recall Mem0 via memory-curator (or Ruflo memory if init locally).
```

---

## Paths

| Item | Path |
|------|------|
| Ship VST3 | `disklordz-ralph/vst/starter-cpp/` |
| Kick target | `autoresearch-cpp/targets/kick.wav` |
| Launch doc | `docs/dev/ruflo-launch.md` |
| Claude layout | `docs/dev/claude-code-best-practice.md` |
| OpenMontage | `docs/dev/openmontage.md` |
| Mem0 agent memory | `docs/dev/mem0-opt-in.md`, `.claude/agents/memory-curator.md` |
| Release packaging | `docs/dev/commercial-release-packaging.md`, `business/release/package_release.py` |
| Strix / MetaGPT / OpenBB | `docs/dev/*-opt-in.md` |

---

## Security

Copy **`.claude/settings.json.example`** → **`.claude/settings.json`**. See **`docs/dev/ruflo-opt-in.md`**.