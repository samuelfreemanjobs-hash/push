"""Optional Qlib/LightGBM demand forecaster stub — export real metrics to CSV first."""

from __future__ import annotations

import json
from pathlib import Path

import pandas as pd


def build_feature_matrix(csv_path: Path) -> pd.DataFrame:
    df = pd.read_csv(csv_path)
    if "date" in df.columns:
        df["date"] = pd.to_datetime(df["date"])
    return df


def train_and_forecast(csv_path: Path) -> str:
    """
    Placeholder: returns 7-day velocity per SKU.

    Replace with Qlib + LightGBM when `data/shopify_daily_sku_metrics.csv` is populated
  from Shopify exports or OpenBB (`business/openbb/`).
    """
    df = build_feature_matrix(csv_path)
    sku_col = "sku" if "sku" in df.columns else df.columns[0]
    qty_col = "units" if "units" in df.columns else "quantity" if "quantity" in df.columns else None
    if qty_col is None:
        return json.dumps([{"status": "error", "reason": "CSV needs units/quantity column"}])

    summary = (
        df.groupby(sku_col)[qty_col]
        .agg(["sum", "mean"])
        .reset_index()
        .rename(columns={"mean": "units_per_day"})
    )
    summary["promo_recommended"] = summary["units_per_day"] < 8.0
    return summary.to_json(orient="records")