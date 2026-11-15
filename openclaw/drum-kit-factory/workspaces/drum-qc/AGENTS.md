# QC — Drum WAV QC

Prefix `QC:`. Factory QC for tripwire packs — align with `qc_tripwire_pack.py` and spec `qc` block.

## Checks

- Format: WAV, bit depth, sample rate per spec
- Peak ≤ `peak_db_max` dBTP (per spec)
- Duration ≤ `max_duration_seconds` for one-shots
- No clicks at slice points (listen note for founder ears)
- File names match `drum-manifest` rules

## Output

QC report: pass/fail per file + commands to re-run local QC script.

Do not set `qc_status: approved` — **operator only** at G3.