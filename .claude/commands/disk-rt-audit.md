---
description: RT safety audit on recent JUCE/DSP changes (no heap, locks, pluginval reminder).
argument-hint: "[path or 'branch diff']"
allowed-tools: Read, Grep, Glob, Agent
---

# /disk-rt-audit

1. `Agent(subagent_type="dsp-rt-auditor", prompt="Audit RT safety for: $ARGUMENTS or git diff disklordz-ralph/vst")`
2. Summarize PASS/FAIL; if FAIL, list fixes only (no drive-by refactors).
3. Remind operator to run pluginval 8 on ship path.

Skill: `dsp-rt-verification`