# Agno revenue & configurator engine (opt-in)

[Agno AgentOS](https://docs.agno.com/) for:

- **custom-kit-architect** — Pydantic `CustomKitConfigResponse` + `dsp_harness` preview
- **merch-strategist** — promos/refunds with `requires_confirmation=True`
- **Workflows** — weekly Qlib stub + nightly factory manifest check

Not on ship path (`STACK-FROZEN`). Shopify/Stripe live writes require AgentOS **Approve**.

## Install

```powershell
cd disklordz-ralph\business\agno-revenue-engine
python -m venv .venv
.\.venv\Scripts\pip install -r requirements.txt
$env:OPENAI_API_KEY = "..."
$env:DLZ_REPO_ROOT = "C:\Dev\push"
```

## Run control plane

```powershell
.\..\..\..\scripts\run-agno-revenue-engine.ps1
# open static/configurator.html (set AGNO_API_BASE if remote)
```

## Scheduler (AgentOS UI)

| Workflow | Suggested cron | Notes |
|----------|----------------|-------|
| `weekly-revenue-optimizer` | `0 7 * * 1` | Needs `data/shopify_daily_sku_metrics.csv` |
| `nightly-factory-prep` | `0 0 * * *` | Checks `audio-factory` manifest; full mastering still Windows `factory_swarm` |

## LanceDb

`knowledge/compatibility.json` is the M0 hybrid KB. Swap `search_kit_compatibility_rules` for LanceDb when indexed.

## Docs

`docs/dev/agno-revenue-engine-opt-in.md`