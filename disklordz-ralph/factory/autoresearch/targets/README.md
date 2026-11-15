# Reference targets (optional)

Drop **your** rendered reference here — do not commit copyrighted kit WAVs.

| File | Use |
|------|-----|
| `reference_808.wav` | STFT timbre loss anchor (mono, 44.1k or 48k) |

**Suggested source:** export from `TrapCoreDSP` / `starter-cpp` 808 voice or Louisiana Forge QC hit, 1–2 s, root C1.

Without a reference, `evaluate.py` uses band-energy **PROFILE_808** only.