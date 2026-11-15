---
name: tripwire_qc
description: Tripwire WAV QC checklist aligned with qc_tripwire_pack.py and kit-spec qc block.
user-invocable: true
---

For SKU `tripwire-sample-v1`:

- WAV 24-bit 44.1kHz, peak ≤ spec `peak_db_max` dBTP
- One-shots within `max_duration_seconds`
- MANIFEST hashes required before pack
- `qc_status: approved` only by **operator** after founder ears (G3)
- Output per-file pass/fail table; suggest `python3` QC command when path known