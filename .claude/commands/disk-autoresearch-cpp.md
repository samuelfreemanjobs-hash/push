---
description: One C++ autoresearch cycle — VoiceDSP.h hypothesis + judge.
allowed-tools: Bash, Read, Edit, Agent
---

# /disk-autoresearch-cpp

1. `Agent(subagent_type="memory-curator", prompt="Recall Mem0 / results.tsv for VoiceDSP prior wins")` or `/disk-mem0-recall`.
2. Read `disklordz-ralph/vst/autoresearch-cpp/results.tsv` last line.
3. `Agent(subagent_type="dsp-architect", prompt="One hypothesis for VoiceDSP.h to beat TOTAL_LOSS")`
4. `Agent(subagent_type="qa-validator", prompt="Run evaluate_vst.py; keep or revert")`
5. On keep: `memory-curator` → `record_autoresearch_win.py --track cpp` if `MEM0_API_KEY` set.
6. If keep and user asked for kit: suggest `/disk-export-kit`.

Skills: `cpp-voice-dsp-loop`, `pluginval-gate`, `mem0-dsp-memory`