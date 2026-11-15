---
description: Search Mem0 for prior DSP/kit wins before a new autoresearch hypothesis.
argument-hint: "[query]"
allowed-tools: Bash, Read, Agent
---

# /disk-mem0-recall

1. Invoke **memory-curator** with query: `$ARGUMENTS` or "trap 808 pitch envelope autoresearch".
2. Prefer `/mem0:search` when Mem0 Claude plugin is installed; else run `search_dsp_memory.py` if `MEM0_API_KEY` is set.
3. Return ≤5 bullets: constraints, last best `TOTAL_LOSS`, formulas to try or avoid.
4. If no API key: read last line of `results.tsv` (cpp + kit) only — do not claim Mem0 recall.

Skill: `mem0-dsp-memory`