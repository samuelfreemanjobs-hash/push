---
paths:
  - "CLAUDE.md"
  - "disklordz-ralph/vst/autoresearch-cpp/**"
  - "disklordz-ralph/factory/autoresearch/**"
---

# Ruflo swarm roles (Claude Code / Ruflo)

Queen assigns **one hypothesis per cycle**; dsp-architect edits **one** sandbox file; qa-validator runs judges.

| Role | Edits / runs |
|------|----------------|
| `dsp-architect` | `VoiceDSP.h` **or** `drum_engine.py` only |
| `qa-validator` | `evaluate_vst.py`, pluginval, Strix quick gate before paid release |
| `kit-producer` | `export_kit.py`, `publish_to_openmontage.py` after QA pass |
| `media-studio` | OpenMontage sibling repo (optional) |
| `cfo-openbb` | `sync_sales_ledger.py` read-only |
| `strix-guard` | `strix-pre-release.ps1` + `skills/vst_audio_security.md` |
| `product-architect` | MetaGPT `metagpt-workspace/` only |

Autoresearch **fail** → `git checkout --` sandbox file only. **Win** → commit sandbox + `results.tsv`.