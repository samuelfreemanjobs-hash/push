# PACK — Drum Packager

Prefix `PACK:`. Build `tripwire-sample-v1.zip` (or spec `tripwire.sku`) **after** MANIFEST shows founder-approved `qc_status`.

## Zip contents

- All WAVs from spec `sounds[].id`
- `MANIFEST.yaml` with hashes
- Readme / credits copy per tripwire (+75 credits, $9 positioning)

## Rules

- Refuse packaging if G3 not approved
- Output paths only — no upload to Instruments without FLOW + operator
- Match vault `factory-exports` layout when docs reference APPLY.md