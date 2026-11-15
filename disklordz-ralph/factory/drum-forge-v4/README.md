# DiskLordz DRUM FORGE 4

An offline, synthesis-only drum sample factory for trap, Memphis, drift phonk, cloud trap, and underground music. This release is a Python sample manufacturing engine—not yet a JUCE plugin.

## Requirements

Python 3.10+; `pip install -r requirements.txt`.

## Quick start

```powershell
py -m pip install -r requirements.txt
py drum_forge_v4.py --genres memphis drift_phonk --variations 4 --notes C1 D1 F1 G1 --out DiskLordz_Kits
```

Full set (7 genres × 8 families, plus 4 notes for 808 per genre at 5 variations = up to 385 files):

```powershell
py drum_forge_v4.py --variations 5 --sr 48000 --bits 24 --out DiskLordz_Full
```

## Flags

- `--genres`: `atl_trap`, `memphis`, `drift_phonk`, `space_trap`, `southern_trap`, `cloud_trap`, `raw_underground`
- `--families`: `808`, `kick`, `snare`, `clap`, `closed_hat`, `open_hat`, `cowbell`, `perc`
- `--variations`: 1–100 accepted variants attempted per combination
- `--notes`: 808 target keys, e.g. `C1 D1 F1 G1`; root pitch set at synthesis and checked after generation
- `--sr`: 44100, 48000, 96000
- `--bits`: 16 or 24-bit PCM WAV
- `--strength`: DNA parameter variation from 0 to 1
- `--similarity`: cosine threshold for heuristic descriptors; 0.8–1.0
- `--max-retries`: retries to find QC-passing varied sounds
- `--ceiling`: estimated max dBTP, from -12 to -0.1
- `--seed`: deterministic seed used to reproduce runs
- `--out`: destination directory

## Contents of exported kit

- `genre/family/*.wav`: actual playable WAVs
- `manifest.csv`: sortable metadata
- `manifest.json`: settings, seed, sound DNA and QC scores
- `catalog.html`: searchable browser-readable sample index; open locally
- `rejected.csv`: generation attempts excluded by QC, when applicable
- `README.txt`: export technical notes

## Tested

The included Demo_Kit was rendered with `--genres memphis drift_phonk --variations 1 --notes C1 D1 --sr 44100`. 18 WAVs were generated and passed the implemented checks. Saved WAV files were reopened and peak verified.

## Important engineering limitations

- True-peak measurement is a 4× oversampling approximation; no dynamic lookahead limiter is implemented.
- The reverb is an algorithmic comb/allpass network, not an accurate physical plate simulation.
- Similarity filtering uses spectral and envelope features and is not a perceptual uniqueness guarantee.
- Root verification checks stable 808 tail, not pitch tracking over the whole note.
- This is not yet a VST3, standalone GUI, SaaS product, nor a fully listening-tested commercial release.

## Next engineering milestone

Port common DSP to C++20 with JUCE 8 as a standalone and VST3; isolate real-time-safe voice renderers and command queues, match offline golden WAV tests, add persistent parameter presets, MIDI note support, envelope editors, oversampled nonlinear effects, and asynchronous WAV batch export. Keep audio callback free from allocation, disk access, locks, and UI calls.
