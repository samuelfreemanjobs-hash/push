# Paste into Grok Bot — Drum Kit Architect (export for OpenClaw)

After we finalize this kit, output **only** a YAML document that matches the schema below.
Do not include commentary outside the YAML block.
Set `architect.system` to `grok-bot-drum-kit-architect`.

Required fields: `kit_id`, `version`, `genre`, `bpm`, `key`, `sounds` (min 1), `qc`, `tripwire`.

Use `tripwire.sku` = `tripwire-sample-v1` until we rename the product.

```yaml
# kit-spec.yaml
kit_id: example-trap-starter
version: 1
architect:
  system: grok-bot-drum-kit-architect
  session_note: "short summary of creative intent"
genre: trap
bpm: 140
key: "F# minor"
sounds:
  - id: kick-01
    role: kick
    description: "short punchy 808-adjacent"
    one_shot: true
  - id: snare-01
    role: snare
    description: "tight trap snare"
    one_shot: true
qc:
  format: "wav"
  bit_depth: 24
  sample_rate: 44100
  peak_db_max: -0.3
  max_duration_seconds: 4
tripwire:
  sku: tripwire-sample-v1
  wav_count: 5
  price_usd: 9
  credits_grant: 75
founder_ears_required: true
```

Save the file and send to OpenClaw: `DRUM: ingest kit-spec` with the YAML attached.