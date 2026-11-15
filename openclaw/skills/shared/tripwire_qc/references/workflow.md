# tripwire_qc workflow

## Inputs

- Tripwire pack folder or manifest from ops snapshot `tripwire.*`
- Reference: `openclaw/drum-kit-factory/handoff/kit-spec.schema.json`

## QC checks

- Loudness / clipping policy per pack brief
- Filename and folder layout match tripwire template
- No stray DAW project files in customer zip
- Manifest JSON valid

## Output

Pass/fail table + fix list; human approves before ship.