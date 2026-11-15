# Weekly Beatmaker Content Brief

Actionable **short-form video** workflow: real producer trends → **your catalog** → ready-to-film scripts.

## Monday 10:00 ritual

```bash
cd disklordz-agent-stack
npm run beatmaker-brief          # scaffold + catalog into hermes-out
```

OpenClaw / Telegram:

```text
MKT: complete Monday beatmaker brief from hermes-out
```

Outputs:

| File | Role |
|------|------|
| `hermes-out/research/beatmaker-brief/LATEST.md` | Fill-in template |
| `hermes-out/research/beatmaker-brief/intake.json` | Catalog + research targets |
| `hermes-out/research/beatmaker-brief/COMPLETE-YYYY-MM-DD.md` | Finished brief (agent-written) |

## Catalog (edit before filming)

`catalog/product-catalog.yaml` — tripwire, VST fleet, preset labels, CTAs, delivery channels.

Cheat sheet: `catalog/PRODUCT-CATALOG-CHEATSHEET.md`.

## Cron

```cron
0 10 * * 1 cd $DLZ_REPO_ROOT/disklordz-agent-stack && npm run beatmaker-brief
```

Then OpenClaw Monday automation → `MKT:` with skill `beatmaker_content_brief`.

## Superagent paste

`openclaw/skills/shared/beatmaker_content_brief/references/superagent-scheduled-task.md`