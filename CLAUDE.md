# Claude Code / Ruflo — DiskLordz workspace

**Repo:** `push` — Detroit Phonk Trap Maker (JUCE), Drum Forge, storefront, OpenClaw.  
**Ship policy:** `disklordz-ralph/STACK-FROZEN.md` — Lite VST3 first; no new frameworks on critical path without operator opt-in.

Full engineering rules: **`AGENTS.md`**. Hybrid stack map: **`docs/dev/hybrid-agent-stack.md`**.

---

## Ruflo swarm (optional — operator runs `npx ruflo@latest init` locally)

After init, use a **hierarchical, specialized** swarm (max **6** agents). Do **not** duplicate work across agents; one owner per file class.

```bash
npx ruflo@latest swarm init --topology hierarchical --max-agents 6 --strategy specialized
```

### Agent roles

| Role | Scope | May edit | Must run |
|------|--------|----------|----------|
| **DSP architect** | C++ synthesis | `disklordz-ralph/vst/autoresearch-cpp/Source/VoiceDSP.h` only (autoresearch loop); production merges via human + `TrapCoreDSP` | `python disklordz-ralph/vst/autoresearch-cpp/evaluate_vst.py` |
| **QA / pluginval** | VST stability | **No** DSP — fix only if gate fails point to DSP agent | `scripts/vst-build-pluginval-gate.ps1` |
| **Sample kit** | WAV kits | `disklordz-ralph/factory/autoresearch/drum_engine.py` only | `python factory/autoresearch/evaluate.py` |
| **Storefront / docs** | Copy | `storefront/`, `PRODUCT.md`, `docs/factory/staged/` — **not** `TrapCoreDSP` | Impeccable audit optional |
| **Intel** (read-mostly) | Market | `openclaw/` skills outputs — no unsolicited outreach sends | OpenClaw compliance refs |
| **Coordinator** | Plan | `prd.json` story pick; routes tasks; **no** parallel edits on same file |

**Forbidden for all swarms:** `evaluate.py`, `evaluate_vst.py`, `PluginProcessor::processBlock` during autoresearch, customer PII, copyright samples.

### Autoresearch autopilot (C++)

Coordinator dispatches loop to **DSP architect** until interrupted:

1. Read `disklordz-ralph/vst/autoresearch-cpp/program_cpp.md`
2. `python disklordz-ralph/vst/autoresearch-cpp/evaluate_vst.py`
3. If `TOTAL_LOSS` improves → `git add Source/VoiceDSP.h && git commit`
4. Else → `git checkout -- Source/VoiceDSP.h`
5. **QA agent** runs pluginval gate on kept commits only

### Autoresearch autopilot (Python kit)

1. Read `disklordz-ralph/factory/autoresearch/program.md`
2. `python evaluate.py` in that folder
3. Same keep/revert for `drum_engine.py`

### Vector memory (Ruflo)

Store **numeric** wins only (no raw WAV):

- `TOTAL_LOSS`, filter coeffs, envelope taus, genre tag (`southern_trap`, `bounce`)
- Link to git SHA + `results.tsv` row

Query memory before proposing duplicate hypotheses.

---

## Paths (Windows primary)

| Task | Path |
|------|------|
| JUCE build | `disklordz-ralph/vst/starter-cpp/` + `-DDLZ_AUTORESEARCH=ON` |
| Ship plugin | `Detroit Phonk Trap Maker` |
| Shared DSP | `disklordz-ralph/vst/Modules/TrapCoreDSP/` |
| REA / pluginval | `disklordz-ralph/vst/WINDOWS-BUILD.md`, `scripts/vst-build-pluginval-gate.ps1` |

---

## Security (post-init)

Review `.claude/settings.json` — restrict shell to:

- `disklordz-ralph/vst/`
- `disklordz-ralph/factory/autoresearch/`
- `scripts/`
- `storefront/`

Deny network except documented MCP (RAGFlow, if configured). Details: `docs/dev/ruflo-opt-in.md`.