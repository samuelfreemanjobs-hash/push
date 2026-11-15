# OpenBB — finance & market ops (opt-in)

**Policy:** Does not block Lite VST3 ship (`STACK-FROZEN.md`). Operator machine only; no live payment keys in the repo or cloud agent.

[OpenBB Platform](https://docs.openbb.co/) + [OpenBB Workspace / Copilot](https://openbb.co/) give you macro data, equity research, and **custom FastAPI widgets** for your own storefront metrics.

---

## Five-pillar company map (finance slice)

| Department | Engine | Responsibility |
|------------|--------|----------------|
| R&D & sound | Ruflo + Claude Code | `VoiceDSP.h`, `drum_engine.py`, pluginval, `export_kit.py` |
| Knowledge | RAGFlow (MCP) | Schematics, EULAs, manuals |
| Video & social | OpenMontage | Shorts / VST trailers from `assets/latest_release/` |
| Finance & market | **OpenBB** | Unit economics, FX/macro context, treasury modeling |
| Security | Strix (opt-in) | License API, downloads, parsers — `docs/dev/strix-opt-in.md` |

OpenBB does **not** process audio. It reads **`sales_ledger.csv`** and public market data you enable in Workspace.

---

## Step A — Install (operator)

```powershell
cd C:\Dev\push\disklordz-ralph\business\openbb
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -r requirements.txt
pip install openbb
```

Install [OpenBB Workspace](https://openbb.co/) on Windows when you have beta access.

---

## Step B — Build `sales_ledger.csv`

```powershell
# Copy example until live sync works:
Copy-Item ..\data\sales_ledger.example.csv ..\data\sales_ledger.csv

# Live read-only sync (keys in env, never commit):
$env:STRIPE_SECRET_KEY = "sk_live_..."   # read-only or restricted key
$env:GUMROAD_ACCESS_TOKEN = "..."
python sync_sales_ledger.py --sync-stripe --sync-gumroad

# Or import Gumroad / Lemon CSV exports:
python sync_sales_ledger.py --import-csv C:\Downloads\gumroad.csv --source gumroad
```

Adjust net estimates with `DLZ_PROCESSOR_FEE_RATE` (default `0.029`) or edit `net_usd` after export.

---

## Step C — FastAPI widget for Workspace

```powershell
uvicorn store_backend:app --host 127.0.0.1 --port 5050
```

In OpenBB Workspace, add custom backend **`http://127.0.0.1:5050`** and load widget **`/widgets.json`**.

Endpoints:

| Path | Purpose |
|------|---------|
| `/sales_data` | Aggregated units, gross/net, refund rate by `product_name` |
| `/tier_reference` | Static Lite/Core/Suite map (see `vst/LICENSING.md`) |
| `/health` | Ledger path + row count |

---

## OpenBB Copilot prompts (finance)

```text
Using my custom backend sales_data table and OpenBB equity data, summarize:
1) Which DiskLordz SKU has best net revenue per unit this month
2) Refund rate outliers vs 5% target
3) How Avid / Spotify / Apple creator-economy commentary from recent earnings might affect Q4 plugin promotions
Do not change pricing — recommend only.
```

```text
Pull EUR/USD and GBP/USD. Given Core at $44 USD and ~2.9% processor fee,
suggest EU/UK list prices rounded to .99 with VAT called out separately for Lemon Squeezy.
```

**Human gate:** any live price or checkout change is operator-approved (see `AGENTS.md`).

---

## Paths

| Item | Path |
|------|------|
| Ledger | `disklordz-ralph/business/data/sales_ledger.csv` (gitignored) |
| Sync script | `disklordz-ralph/business/openbb/sync_sales_ledger.py` |
| API | `disklordz-ralph/business/openbb/store_backend.py` |
| Tiers | `disklordz-ralph/vst/LICENSING.md` |

---

## Related

- Hybrid stack: `docs/dev/hybrid-agent-stack.md`
- Friday KPI stub: `disklordz-agent-stack/scripts/biz-metrics.mjs` (can point at same ledger later)