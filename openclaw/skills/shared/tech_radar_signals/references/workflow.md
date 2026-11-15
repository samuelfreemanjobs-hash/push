# tech_radar_signals workflow

## Source

Primary: [AI Dev Signals](https://www.aidevsignals.com/) (Marktechpost Beehiiv).

Ingest: `disklordz-agent-stack` → `npm run tech-radar` → `hermes-out/research/tech-radar/`.

## Categories

| id | Watch for |
|----|-----------|
| audio_ai | Sound gen, ASR, separation, tagging |
| coding_ai | C++, CMake, codegen agents, build tools |
| dsp_research | Synthesis, filters, physical modeling |
| local_models | Quantization, edge/CPU/GPU inference |
| agent_frameworks | MCP, orchestration, reliable automation |
| commercial_opportunity | Shippable plugin or SaaS angle |

## Steps

1. Read `intake.json` for `latest_issue` or `inbox/latest-body.md`.
2. Optionally open the issue URL in browser tools if body is missing.
3. For each **relevant** headline only, draft one YAML block (see `REVIEW-*.md`).
4. Cap total kept items at **six** per issue — prefer depth over coverage.
5. Write `report-YYYY-MM-DD.md`; dedupe against prior reports in same folder.
6. Never auto-implement adopt items — propose `next_action` for FORGE/DEV/ARCH.

## Product fit map

- **vst_factory** — JUCE plugins, DSP, Instruments CI
- **tripwire_packs** — drum kits, QC, Grok kit-spec
- **command_center** — ops PWA, kanban, gates UI
- **agent_stack** — OpenClaw, cron, hermes-out, Telegram ops
- **none** — watch/skip only

## Opportunity stack (rank 1–12)

Read `hermes-out/research/opportunities.json` or repo `disklordz-agent-stack/lib/opportunities.mjs`.

Each kept item must list `opportunity_ids` (e.g. `juce_vst_factory`, `drum_sample_factory`).

**Adopt bias:** unblock ranks **1–5** (`buildPriority: immediate`) when integration ≤ 3 and licensing is usable.

| Rank | id | Priority |
|------|-----|----------|
| 1 | drum_sample_factory | immediate |
| 2 | juce_vst_factory | immediate |
| 3 | audio_qc_pipeline | immediate |
| 4 | sample_tagging_search | immediate |
| 5 | marketing_launch_automation | immediate |
| 6–10 | (see OPPORTUNITIES.md) | next |
| 11–12 | support_docs_assistant, market_intelligence_monitoring | later |

## Licensing discipline

State license or ToS tier explicitly. If unknown, say `unknown` and set `verdict: watch`.