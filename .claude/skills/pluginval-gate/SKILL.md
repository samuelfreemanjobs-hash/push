---
name: pluginval-gate
description: Run DiskLordz pluginval and autoresearch judge gates on Windows. Use when validating VST3 ship or autoresearch wins.
disable-model-invocation: false
allowed-tools: Bash, Read
---

# Pluginval gate

1. Read `docs/dev/vst-commercial-gates.md`.
2. C++ autoresearch: `python disklordz-ralph/vst/autoresearch-cpp/evaluate_vst.py`
3. Ship tree (operator): `scripts/vst-build-pluginval-gate.ps1`
4. Strictness: 5 daily, **8** pre-release (`PLUGINVAL_STRICTNESS=8`).
5. On failure: `git checkout --` autoresearch sandbox only — never disable the judge.