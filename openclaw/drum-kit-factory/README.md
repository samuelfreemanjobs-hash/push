# OpenClaw drum kit factory team (Grok + VPS)

**Grok Bot “Drum Kit Architect”** owns creative direction (concept, sound design brief, sample map, genre/BPM/key).  
**OpenClaw on your VPS** owns factory execution (QC, manifest, zip, tripwire steps, Telegram, GitHub handoff to Instruments).

You get one team on the phone without replacing Grok.

## Division of labor

| Layer | Where | Responsibility |
|-------|--------|----------------|
| **Architect** | Grok Bot (persistent bot) | Kit vision, layer list, references, iteration with you |
| **Bridge** | OpenClaw `grok-bridge` | Validate `kit-spec.yaml`, store in shared memory, open factory job |
| **Factory floor** | OpenClaw `drum-qc`, `drum-pack`, `drum-manifest` | QC rules, MANIFEST hashes, `tripwire-sample-v1.zip` |
| **Conductor** | OpenClaw `drum-lead` | Tripwire steps G1–G6, delegates, reports to Telegram |
| **Launch** | `trip` / `flow` (elite roster) | Instruments push, checkout, vault PR notes |

```text
You ↔ Grok Drum Kit Architect  →  kit-spec.yaml (export)
You ↔ Telegram DRUM:          →  drum-lead → grok-bridge → qc → pack
Founder ears (G3)             →  Operator updates MANIFEST (human gate)
Cron                          →  KPI + tripwire step status digest
```

## Quick install

```bash
# 1) Install workspaces + register agents
./scripts/install-drum-workspaces.sh

# 2) Merge drum team into ~/.openclaw/openclaw.json
#    See openclaw.drum-kit-team.json5.example

openclaw gateway restart
```

Register agents (after script prints paths):

```bash
openclaw agents add drum-lead --workspace ~/.openclaw/workspace-drum-lead --non-interactive
openclaw agents add grok-bridge --workspace ~/.openclaw/workspace-grok-bridge --non-interactive
openclaw agents add drum-qc --workspace ~/.openclaw/workspace-drum-qc --non-interactive
openclaw agents add drum-pack --workspace ~/.openclaw/workspace-drum-pack --non-interactive
openclaw agents add drum-manifest --workspace ~/.openclaw/workspace-drum-manifest --non-interactive
```

## Grok → OpenClaw handoff

1. Finish an architect session in **Grok Bot** (Drum Kit Architect).
2. Ask Grok to emit a spec matching `handoff/kit-spec.template.yaml` (or paste `handoff/grok-architect-prompt.md`).
3. Save as `kit-spec.yaml` in your pack folder or paste in Telegram.

Telegram:

```text
DRUM: ingest kit-spec for tripwire-sample-v2
```

Attach file or paste YAML. `grok-bridge` validates with:

```bash
node scripts/validate-kit-spec.mjs path/to/kit-spec.yaml
```

4. `drum-lead` assigns QC → manifest → pack. You stay at **G3 founder ears** before zip ships.

## Telegram prefixes

| Prefix | Agent | Use |
|--------|--------|-----|
| `DRUM:` | drum-lead | Default factory conductor |
| `GROK:` | grok-bridge | Spec ingest / export prompts for Grok |
| `QC:` | drum-qc | WAV checks, `qc_tripwire_pack.py` alignment |
| `PACK:` | drum-pack | Zip + tripwire bundle |
| `MANIFEST:` | drum-manifest | Naming, hashes, `qc_status` |

Route via mention or instruct `sage` to delegate to `drum-lead` for DRUM lane (vault `drumkit-factory-agent`).

## Shared memory

Point all drum workspaces at:

- `handoff/` (contracts + templates in this repo)
- Vault clone or export: tripwire MANIFEST conventions
- Last accepted `kit-spec.yaml` per SKU under `~/.openclaw/workspace-drum-lead/kits/<kit_id>/`

Do not store Grok API keys in shared memory.

## Cron (examples)

| Schedule | Agent | Job |
|----------|--------|-----|
| Daily 7am | drum-lead | Tripwire step status vs Command Center snapshot |
| After ingest | drum-qc | Queue QC checklist from spec |
| Fri 4pm | drum-lead | Weekly kit factory KPI (packs in progress, G3 blocked) |

See `cron/drum-factory.cron.yaml` for OpenClaw automation stubs.

## Tripwire alignment

Steps match Command Center tripwire panel:

1. Generate one-shots (Grok architect + your generator)
2. Automated WAV QC + MANIFEST (`drum-qc`, `drum-manifest`)
3. **Founder ears** — operator only
4. Package zip (`drum-pack`)
5. Instruments + checkout (`flow` / vault PR)
6. Smoke purchase — operator

## Related repos

- Elite ops roster: `openclaw/disklordz-elite-team/`
- Tripwire UI: `disklordz-command-center` → Tripwire panel
- Vault PR #126 — DRUM QC + tripwire Instruments bundle