---
description: One C++ autoresearch cycle — VoiceDSP.h hypothesis + judge.
allowed-tools: Bash, Read, Edit, Agent
---

# /disk-autoresearch-cpp

1. Read `disklordz-ralph/vst/autoresearch-cpp/results.tsv` last line.
2. `Agent(subagent_type="dsp-architect", prompt="One hypothesis for VoiceDSP.h to beat TOTAL_LOSS")`
3. `Agent(subagent_type="qa-validator", prompt="Run evaluate_vst.py; keep or revert")`
4. If keep and user asked for kit: suggest `/disk-export-kit`.

Skills: `cpp-voice-dsp-loop`, `pluginval-gate`