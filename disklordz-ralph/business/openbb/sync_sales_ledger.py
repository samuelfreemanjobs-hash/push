#!/usr/bin/env python3
"""
Merge storefront sales into business/data/sales_ledger.csv for OpenBB.

Sources (operator-only — never commit API keys):
  - Stripe:  STRIPE_SECRET_KEY + --sync-stripe
  - Gumroad: GUMROAD_ACCESS_TOKEN + --sync-gumroad
  - Manual:  --import-csv path --source stripe|gumroad|lemon|shopify

Example:
  python sync_sales_ledger.py --sync-stripe --sync-gumroad
  python sync_sales_ledger.py --import-csv ~/Downloads/gumroad-sales.csv --source gumroad
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import urllib.parse
import urllib.request
from datetime import datetime, timezone
from pathlib import Path

BUSINESS = Path(__file__).resolve().parent.parent
LEDGER_PATH = Path(os.environ.get("DLZ_SALES_LEDGER", BUSINESS / "data" / "sales_ledger.csv"))

COLUMNS = [
    "order_id",
    "product_name",
    "amount_usd",
    "net_usd",
    "refunded",
    "created_at",
    "source",
    "currency",
]

# Approximate processor fee for net estimate when provider does not return net (operator can overwrite in CSV).
DEFAULT_FEE_RATE = float(os.environ.get("DLZ_PROCESSOR_FEE_RATE", "0.029"))


def _read_ledger() -> list[dict]:
    if not LEDGER_PATH.is_file():
        return []
    with LEDGER_PATH.open(newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def _write_ledger(rows: list[dict]) -> None:
    LEDGER_PATH.parent.mkdir(parents=True, exist_ok=True)
    with LEDGER_PATH.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=COLUMNS, extrasaction="ignore")
        w.writeheader()
        for row in rows:
            w.writerow({k: row.get(k, "") for k in COLUMNS})


def _dedupe_key(row: dict) -> str:
    return f"{row.get('source', '')}:{row.get('order_id', '')}"


def _merge(existing: list[dict], incoming: list[dict]) -> list[dict]:
    by_key = {_dedupe_key(r): r for r in existing}
    for row in incoming:
        by_key[_dedupe_key(row)] = row
    return sorted(by_key.values(), key=lambda r: r.get("created_at", ""))


def _cents_to_usd(cents: int | float) -> float:
    return round(float(cents) / 100.0, 2)


def sync_stripe(limit: int = 100) -> list[dict]:
    key = os.environ.get("STRIPE_SECRET_KEY", "").strip()
    if not key:
        raise RuntimeError("STRIPE_SECRET_KEY not set")

    try:
        import stripe  # type: ignore
    except ImportError as exc:
        raise RuntimeError("pip install stripe") from exc

    stripe.api_key = key
    rows: list[dict] = []
    charges = stripe.Charge.list(limit=limit)
    for ch in charges.auto_paging_iter():
        if not getattr(ch, "paid", False):
            continue
        amount = _cents_to_usd(ch.amount)
        refunded = bool(ch.refunded or (ch.amount_refunded or 0) > 0)
        net = amount * (1.0 - DEFAULT_FEE_RATE) if not refunded else 0.0
        desc = ch.description or (ch.metadata or {}).get("product_name", "Stripe charge")
        created = datetime.fromtimestamp(ch.created, tz=timezone.utc).isoformat()
        rows.append(
            {
                "order_id": ch.id,
                "product_name": desc,
                "amount_usd": amount,
                "net_usd": round(net, 2),
                "refunded": int(refunded),
                "created_at": created,
                "source": "stripe",
                "currency": (ch.currency or "usd").upper(),
            }
        )
        if len(rows) >= limit:
            break
    return rows


def sync_gumroad(limit: int = 100) -> list[dict]:
    token = os.environ.get("GUMROAD_ACCESS_TOKEN", "").strip()
    if not token:
        raise RuntimeError("GUMROAD_ACCESS_TOKEN not set")

    rows: list[dict] = []
    page = 1
    while len(rows) < limit:
        query = urllib.parse.urlencode(
            {"access_token": token, "page": page, "per_page": min(50, limit - len(rows))}
        )
        url = f"https://api.gumroad.com/v2/sales?{query}"
        req = urllib.request.Request(url, headers={"User-Agent": "DiskLordz-OpenBB/1.0"})
        with urllib.request.urlopen(req, timeout=60) as resp:
            payload = json.loads(resp.read().decode("utf-8"))
        sales = payload.get("sales") or []
        if not sales:
            break
        for sale in sales:
            price_cents = int(sale.get("price", 0) or 0)
            amount = round(price_cents / 100.0, 2)
            refunded = bool(sale.get("refunded", False))
            net = amount * (1.0 - DEFAULT_FEE_RATE) if not refunded else 0.0
            rows.append(
                {
                    "order_id": sale.get("id") or sale.get("sale_id", ""),
                    "product_name": sale.get("product_name") or sale.get("short_product_id", "Gumroad"),
                    "amount_usd": amount,
                    "net_usd": round(net, 2),
                    "refunded": int(refunded),
                    "created_at": sale.get("created_at", ""),
                    "source": "gumroad",
                    "currency": (sale.get("currency") or "usd").upper(),
                }
            )
        if not payload.get("next_page_url"):
            break
        page += 1
    return rows[:limit]


def import_csv(path: Path, source: str) -> list[dict]:
    rows: list[dict] = []
    with path.open(newline="", encoding="utf-8-sig") as f:
        reader = csv.DictReader(f)
        for raw in reader:
            amount = float(raw.get("amount_usd") or raw.get("amount") or raw.get("gross") or 0)
            net = float(raw.get("net_usd") or raw.get("net") or amount * (1.0 - DEFAULT_FEE_RATE))
            refunded_raw = raw.get("refunded") or raw.get("refund") or "0"
            refunded = int(str(refunded_raw).lower() in ("1", "true", "yes", "y"))
            rows.append(
                {
                    "order_id": raw.get("order_id") or raw.get("id") or raw.get("Sale ID", ""),
                    "product_name": raw.get("product_name") or raw.get("product") or raw.get("Product", ""),
                    "amount_usd": round(amount, 2),
                    "net_usd": round(net, 2),
                    "refunded": refunded,
                    "created_at": raw.get("created_at") or raw.get("date") or raw.get("Date", ""),
                    "source": source,
                    "currency": (raw.get("currency") or "USD").upper(),
                }
            )
    return rows


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--sync-stripe", action="store_true")
    ap.add_argument("--sync-gumroad", action="store_true")
    ap.add_argument("--import-csv", type=Path, default=None)
    ap.add_argument("--source", default="manual", help="Label for --import-csv rows")
    ap.add_argument("--limit", type=int, default=200)
    args = ap.parse_args()

    incoming: list[dict] = []
    if args.sync_stripe:
        incoming.extend(sync_stripe(limit=args.limit))
    if args.sync_gumroad:
        incoming.extend(sync_gumroad(limit=args.limit))
    if args.import_csv:
        incoming.extend(import_csv(args.import_csv, args.source))

    if not incoming:
        print("No new rows (pass --sync-stripe, --sync-gumroad, or --import-csv).")
        return

    merged = _merge(_read_ledger(), incoming)
    _write_ledger(merged)
    print(f"Wrote {len(merged)} rows to {LEDGER_PATH}")


if __name__ == "__main__":
    main()