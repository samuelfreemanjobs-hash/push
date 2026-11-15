---
name: tripwire_qc
description: Tripwire WAV QC checklist aligned with qc_tripwire_pack.py and kit-spec qc block.
version: 1.0.0
user-invocable: true
homepage: https://github.com/samuelfreemanjobs-hash/push/tree/main/openclaw/skills/shared/tripwire_qc
---

Before acting, read `references/workflow.md` (and sibling files under `references/`) via skills_read.


For SKU `tripwire-sample-v1`:

- WAV 24-bit 44.1kHz, peak ≤ spec `peak_db_max` dBTP
- One-shots within `max_duration_seconds`
- MANIFEST hashes required before pack
- `qc_status: approved` only by **operator** after founder ears (G3)
- Output per-file pass/fail table; suggest `python3` QC command when path known