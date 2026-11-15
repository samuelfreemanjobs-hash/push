---
description: Run Windows pluginval gate on Detroit Phonk Trap Maker (operator).
argument-hint: "[strictness 5|8]"
allowed-tools: Bash, Read
---

# /disk-pluginval-gate

1. Invoke subagent **qa-validator** with: run `docs/dev/vst-commercial-gates.md` checklist.
2. On Windows: `powershell -File scripts/vst-build-pluginval-gate.ps1` (set `PLUGINVAL_STRICTNESS` if arg provided).
3. Summarize pass/fail; if fail, list first pluginval error block only.
4. Do **not** edit `starter-cpp` unless user explicitly asked to fix build.

Skill: `pluginval-gate`