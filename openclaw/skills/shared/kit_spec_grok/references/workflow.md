# kit_spec_grok workflow

## Ingest

Grok export YAML/JSON must include:

```yaml
architect:
  system: grok-bot-drum-kit-architect
```

## Validate

Run mentally against `kit-spec.schema.json` fields: tempo range, genres, sample count, naming.

## Handoff

Write or propose `handoff/kit-spec.yaml` for `openclaw/drum-kit-factory/` agents.