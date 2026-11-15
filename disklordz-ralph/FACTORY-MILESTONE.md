# Factory milestone M1 (opportunities 1–3)

Single epic spanning **Immediate** ranks 1–3 from [OPPORTUNITIES.md](../disklordz-agent-stack/OPPORTUNITIES.md):

| Opp | Application | M1 deliverable |
|-----|-------------|----------------|
| 1 | AI-assisted drum sample factory | Validated Grok `kit-spec` → automated **factory work order** |
| 2 | Automated JUCE VST development system | **vst_factory** skill + Ralph **DLZ-VST-002** pluginval gate (Windows) |
| 3 | Automated audio QC pipeline | **QC manifest** schema + validator wired before pack |

## Exit criteria (milestone complete)

All of the following are true:

1. `openclaw/drum-kit-factory/scripts/emit-factory-work-order.mjs` runs clean on the kit-spec template.
2. `openclaw/drum-kit-factory/scripts/validate-qc-manifest.mjs` runs clean on the QC template.
3. OpenClaw skills `kit_spec_grok`, `tripwire_qc`, `vst_factory` are present and referenced in this doc.
4. Ralph stories **DLZ-FACTORY-M1-1** … **M1-3** and capstone **DLZ-FACTORY-M1** have `passes: true`.
5. **DLZ-DRUM-002** (tripwire pack) and **DLZ-VST-002** (pluginval 5) remain the *shipping* gates; M1 is the automation spine feeding them.

## Verify (repo / CI)

```bash
node openclaw/drum-kit-factory/scripts/verify-factory-milestone.mjs
```

## Verify (founder Windows VST)

```bash
node disklordz-ralph/scripts/verify-story.mjs DLZ-VST-002
```

## Ralph pick order

When any `factory-m1` story has `passes: false`, prefer it over unrelated P1 work (see `PROMPT.md`).

## Agents

| Prefix | Role in M1 |
|--------|------------|
| DRUM / grok-bridge | kit-spec ingest + work order |
| FORGE | VST build + pluginval |
| drum-qc | QC manifest + tripwire_qc |
| SAGE | One brief line when M1 sub-story lands |