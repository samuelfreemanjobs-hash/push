# Elite frameworks (DiskLordz composition layer)

Composable **opt-in** patterns for VST architecture, agent quality, and long-horizon ops. Does **not** replace `STACK-FROZEN.md` or the `starter-cpp` ship path.

| Start here | File |
|------------|------|
| Operator map (all 14) | `docs/dev/elite-frameworks-implementation.md` |
| Enable flags | `manifest.yaml` |
| OpenClaw agents | `../agents/REGISTRY.yaml` |

## Quick commands

```bash
# EGM: challenge a spec before coding
python openclaw/elite-frameworks/egm/run_spec_duel.py \
  --spec disklordz-ralph/labs/gpu-inference/README.md

# Waza: agent artifact evals (no GPU)
bash openclaw/elite-frameworks/waza/evals/run_all.sh

# TokenEconomizer: shrink a handoff log
python openclaw/elite-frameworks/token-economizer/prune_handoff.py \
  --in /tmp/handoff.md --out /tmp/handoff.pruned.md

# DPP: chunk a JUCE PDF or manual
python openclaw/elite-frameworks/document-parsing-protocol/ingest.py \
  --in docs/manuals/example.md --out /tmp/chunks.json
```

See each subdirectory `README.md` for framework-specific notes.