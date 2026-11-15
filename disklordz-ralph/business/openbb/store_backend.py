#!/usr/bin/env python3
"""FastAPI widget backend for OpenBB Workspace — DiskLordz storefront metrics."""

from __future__ import annotations

import os
from pathlib import Path

import pandas as pd
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

BUSINESS = Path(__file__).resolve().parent.parent
DEFAULT_LEDGER = BUSINESS / "data" / "sales_ledger.csv"
LEDGER_PATH = Path(os.environ.get("DLZ_SALES_LEDGER", DEFAULT_LEDGER))

app = FastAPI(title="DiskLordz Store Metrics")

app.add_middleware(
    CORSMiddleware,
    allow_origins=[
        "https://openbb.co",
        "http://localhost:1420",
        "http://127.0.0.1:1420",
    ],
    allow_methods=["*"],
    allow_headers=["*"],
)


@app.get("/widgets.json")
def get_widgets() -> dict:
    return {
        "kit_sales_summary": {
            "name": "Drum Kit & VST Sales Summary",
            "description": "Revenue, units, and refund rate by product (DiskLordz)",
            "category": "Storefront Ops",
            "type": "table",
            "endpoint": "/sales_data",
        },
        "sku_pricing_reference": {
            "name": "SKU tier reference (Lite / Core / Suite)",
            "description": "Static tier map from LICENSING.md — not live prices",
            "category": "Storefront Ops",
            "type": "table",
            "endpoint": "/tier_reference",
        },
    }


def _load_ledger() -> pd.DataFrame:
    if not LEDGER_PATH.is_file():
        return pd.DataFrame(
            columns=[
                "order_id",
                "product_name",
                "amount_usd",
                "net_usd",
                "refunded",
                "created_at",
                "source",
                "currency",
            ]
        )
    return pd.read_csv(LEDGER_PATH)


@app.get("/sales_data")
def get_sales_data() -> list[dict]:
    df = _load_ledger()
    if df.empty:
        return []
    summary = (
        df.groupby("product_name", dropna=False)
        .agg(
            units_sold=("order_id", "count"),
            gross_revenue=("amount_usd", "sum"),
            net_revenue=("net_usd", "sum"),
            refund_rate=("refunded", "mean"),
        )
        .reset_index()
    )
    return summary.to_dict(orient="records")


@app.get("/tier_reference")
def tier_reference() -> list[dict]:
    return [
        {"tier": "Lite", "price_usd": 0, "sku_hint": "DLZ-VST-LITE"},
        {"tier": "Core", "price_usd": 44, "sku_hint": "DLZ-VST-CORE"},
        {"tier": "Suite", "price_usd": 89, "sku_hint": "DLZ-VST-SUITE"},
        {"tier": "Expansion", "price_usd": 19, "sku_hint": "DLZ-EXP-*"},
        {"tier": "Drum kit WAV", "price_usd": None, "sku_hint": "DLZ-DRUM-*"},
    ]


@app.get("/health")
def health() -> dict:
    return {"ok": True, "ledger": str(LEDGER_PATH), "rows": len(_load_ledger())}