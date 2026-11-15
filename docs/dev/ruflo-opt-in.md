# Ruflo (optional multi-agent orchestrator)

[Ruflo](https://github.com/ruvnet/ruflo) coordinates Claude Code swarms locally. **DiskLordz treats Ruflo as opt-in** — it does not replace Cursor Cloud Agents, OpenClaw, or `STACK-FROZEN.md`.

**Already in repo:** swarm roles in root **`CLAUDE.md`**, **`.claude/settings.json.example`**, **`docs/dev/ruflo-launch.md`**, `scripts/ruflo-seed-memory.ps1`, `scripts/export_kit_velocities.ps1`, autoresearch harnesses, pluginval gate.

---

## When to use Ruflo vs Cursor

| Use Ruflo | Use Cursor / Cloud Agent |
|-----------|---------------------------|
| Parallel specialized workers on **your** Windows box | PRs, repo docs, cloud turns |
| Long local autopilot on `VoiceDSP.h` / `drum_engine.py` | `prd.json` single-story ship |
| Vector memory across Claude Code sessions | Supermemory / `AGENTS.md` |

---

## Initialize (operator only — do not commit secrets)

```powershell
cd C:\Dev\push
npx ruflo@latest init
```

Creates `.claude/`, `.claude-flow/`, registers MCP. **Review** generated files before committing any of them (prefer keeping Ruflo config **local** or use `.gitignore` for `.claude-flow/` secrets).

```powershell
npx ruflo@latest swarm init --topology hierarchical --max-agents 6 --strategy specialized
```

Paste swarm role table from **`CLAUDE.md`** into Ruflo agent definitions; **trim** default plugins per [community security notes](https://skillsllm.com/skill/ruflo).

---

## Harden `.claude/settings.json` (template)

After `init`, restrict tools:

```json
{
  "permissions": {
    "allow": [
      "Read(**/disklordz-ralph/vst/**)",
      "Read(**/factory/autoresearch/**)",
      "Edit(**/autoresearch-cpp/Source/VoiceDSP.h)",
      "Edit(**/factory/autoresearch/drum_engine.py)",
      "Read(**/storefront/**)",
      "Edit(**/storefront/**)",
      "Bash(cmake *)",
      "Bash(python disklordz-ralph/**)",
      "Bash(scripts/vst-build-pluginval-gate.ps1)"
    ],
    "deny": [
      "Edit(**/evaluate_vst.py)",
      "Edit(**/factory/autoresearch/evaluate.py)",
      "Edit(**/TrapCoreDSP/**)",
      "Edit(**/PluginProcessor.cpp)",
      "Bash(git push *)",
      "Bash(rm -rf *)"
    ]
  }
}
```

Adjust to your Ruflo schema; principle: **narrow shell + single-file DSP edit**.

---

## Autopilot loop commands

### C++ kick sandbox

```text
@coordinator Run C++ autoresearch: follow program_cpp.md until TOTAL_LOSS plateaus or 50 iterations.
DSP agent edits only Source/VoiceDSP.h.
After each keep commit, QA agent runs vst-build-pluginval-gate.ps1.
```

Manual one-liner:

```powershell
cd disklordz-ralph\vst\autoresearch-cpp
python evaluate_vst.py
```

### Python kit (3 voices)

```text
@coordinator Run kit autoresearch per factory/autoresearch/program.md.
Sample agent edits drum_engine.py only.
Log results.tsv each iteration.
```

### Memory hooks

```powershell
npx ruflo@latest memory
```

Store after **kept** commits:

- `genre=southern_trap`, `TOTAL_LOSS=1.23`, `sha=abc123`, `pitch_tau=0.055`

---

## Swarm ↔ existing stack

| Ruflo role | Repo harness |
|------------|----------------|
| DSP architect | `autoresearch-cpp/Source/VoiceDSP.h` |
| QA | `scripts/vst-build-pluginval-gate.ps1` |
| Sample kit | `factory/autoresearch/` |
| Docs | `storefront/`, Impeccable, RAGFlow MCP (`docs/dev/ragflow-knowledge-base.md`) |
| Intel | `openclaw/skills/shared/vst_market_intel/` (read-only; comply with outreach rules) |

**Do not** add a fifth orchestrator on top of AutoGPT for daily DSP — `STACK-FROZEN.md`.

---

## What not to do

- Run swarm without pluginval on **kept** C++ commits
- Let storefront agent change `LICENSING.md` without human review
- Upload copyrighted manuals into Ruflo memory (use RAGFlow KB with licensed docs only)
- Commit `.claude-flow/` API keys

---

## Quick checklist

- [ ] `npx ruflo@latest init` on Windows dev machine
- [ ] Harden permissions; read `CLAUDE.md` swarm table
- [ ] CMake once: `-DDLZ_AUTORESEARCH=ON`
- [ ] `targets/kick.wav` + kit `targets/*.wav` in place
- [ ] First manual `evaluate_vst.py` / `evaluate.py` green
- [ ] Start hierarchical swarm; coordinator owns iteration budget