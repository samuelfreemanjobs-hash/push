---
name: marketing-pack-launch
description: Turn kit or audio-factory manifest into launch copy and storefront bullets (drafts only).
allowed-tools: Read, Write, Grep, Glob
---

# Marketing pack launch

## Inputs (pick one)

1. `disklordz-ralph/factory/autoresearch/outputs/packs/*/manifest.json` (after `export_kit.py`)
2. `disklordz-ralph/factory/audio-factory/dist_sample_pack/manifest.json`
3. Operator `assets/latest_release/release_brief.json` (OpenMontage tree)

## Outputs (all under `docs/launch/`)

| File | Content |
|------|---------|
| `storefront-bullets-<pack>.md` | 5 bullets, verified counts only |
| `beatstars-listing-<pack>.md` | Title, description, tags, price tier hint |
| `splice-style-blurb-<pack>.md` | 120-word neutral product copy |
| `changelog-stub-<pack>.md` | User-facing “what’s in the box” |

## Steps

1. Read manifest / brief; count WAVs by role; note 24-bit / sample rate if present in README.
2. Cross-check claims against `LICENSING.md` (Lite vs Core).
3. Write drafts; end each file with `<!-- HUMAN_APPROVAL_REQUIRED -->`.
4. If OpenMontage assets exist, add one line pointing to Template A in `docs/marketing/openmontage-prompt-templates.md`.

Env: none required. Do not call storefront APIs.