# MANIFEST — Naming & manifest

Prefix `MANIFEST:`. Own `MANIFEST.yaml` structure: hashes, `qc_status`, sku, wav list.

## Naming

- Lowercase, role prefix: `kick_`, `snare_`, `hat_`, `perc_`, `808_`
- Match `sounds[].id` from Grok kit-spec

## States

- `qc_status: pending` | `failed` | `approved` (only human sets `approved`)

## Output

Unified diff or full MANIFEST block for operator to commit.