#!/usr/bin/env python3
"""
Competitor drum VST / sample pack price monitor (Firecrawl structured scrape).
Writes docs/market-research/ and mirrors to ~/.openclaw/hermes-out/research/.
"""
from __future__ import annotations

import json
import os
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, List, Optional

from pydantic import BaseModel, Field

try:
    from firecrawl import Firecrawl
except ImportError as exc:  # pragma: no cover
    print("Install: pip install -r track_competitors-requirements.txt", file=sys.stderr)
    raise SystemExit(1) from exc


class CompetitorProduct(BaseModel):
    product_name: str = Field(description="Drum VST or trap/phonk sample pack name")
    brand: str = Field(description="Developer or label")
    price_usd: Optional[float] = Field(default=None, description="USD price; null if unknown")
    is_on_sale: bool = Field(default=False, description="Discount active")
    included_formats: List[str] = Field(default_factory=list)
    key_selling_points: List[str] = Field(default_factory=list)


class MarketSnapshot(BaseModel):
    products: List[CompetitorProduct] = Field(default_factory=list)


TARGET_URLS = [
    "https://xlnaudio.com",
    "https://wavealchemy.co.uk",
    "https://wavs.com",
    "https://waproduction.com",
]

SCRAPE_PROMPT = (
    "Extract drum plugins, trap/phonk sample packs, or beatmaker tools visible on this page: "
    "name, brand, USD price if shown, sale banner, formats (VST3, AU, WAV, MIDI), top 3 marketing hooks."
)


def repo_root() -> Path:
    env = os.environ.get("DLZ_REPO_ROOT", "").strip()
    if env:
        return Path(env).expanduser().resolve()
    here = Path(__file__).resolve().parent
    for candidate in [here, *here.parents]:
        if (candidate / "disklordz-ralph").is_dir():
            return candidate
    return Path.cwd()


def hermes_out() -> Path:
    state = os.environ.get("OPENCLAW_STATE_DIR", Path.home() / ".openclaw")
    return Path(state).expanduser() / "hermes-out" / "research"


def extract_products(result: Any) -> list[dict]:
    if result is None:
        return []
    if hasattr(result, "json") and result.json:
        data = result.json
    elif isinstance(result, dict):
        data = result.get("json") or result.get("data") or result
    else:
        data = {}
    if isinstance(data, dict) and "products" in data:
        return data["products"]
    if isinstance(data, dict) and "items" in data:
        return data["items"]
    return []


def run_tracker() -> int:
    api_key = os.environ.get("FIRECRAWL_API_KEY")
    if not api_key:
        print("Missing FIRECRAWL_API_KEY", file=sys.stderr)
        return 2

    app = Firecrawl(api_key=api_key)
    root = repo_root()
    out_dir = root / "docs" / "market-research"
    out_dir.mkdir(parents=True, exist_ok=True)
    history_file = out_dir / "price_history.json"

    previous_data: dict = {}
    if history_file.exists():
        previous_data = json.loads(history_file.read_text(encoding="utf-8"))

    current_run = {
        "timestamp": datetime.now(timezone.utc).isoformat(),
        "items": [],
        "alerts": [],
    }

    schema = MarketSnapshot.model_json_schema()

    for url in TARGET_URLS:
        try:
            result = app.scrape(
                url,
                formats=[
                    {
                        "type": "json",
                        "schema": schema,
                        "prompt": SCRAPE_PROMPT,
                    }
                ],
            )
            for raw in extract_products(result):
                try:
                    item = CompetitorProduct.model_validate(raw).model_dump()
                except Exception:
                    item = {**raw} if isinstance(raw, dict) else {"product_name": str(raw), "brand": "unknown"}
                item["source_url"] = url
                current_run["items"].append(item)

                key = f"{item.get('brand', '')}::{item.get('product_name', '')}"
                prev_price = previous_data.get(key, {}).get("price_usd")
                new_price = item.get("price_usd")

                if key not in previous_data and item.get("product_name"):
                    current_run["alerts"].append(f"NEW: {item.get('product_name')} ({url})")
                if (
                    prev_price is not None
                    and new_price is not None
                    and new_price < prev_price
                ):
                    current_run["alerts"].append(
                        f"PRICE DROP: {item.get('product_name')} ${prev_price} → ${new_price} ({url})"
                    )
                previous_data[key] = item

        except Exception as e:
            current_run["alerts"].append(f"ERROR scraping {url}: {e}")

    history_file.write_text(json.dumps(previous_data, indent=2), encoding="utf-8")
    snapshot_path = out_dir / "latest_snapshot.json"
    snapshot_path.write_text(json.dumps(current_run, indent=2), encoding="utf-8")

    hermes = hermes_out()
    hermes.mkdir(parents=True, exist_ok=True)
    (hermes / "vst-market-latest.json").write_text(
        json.dumps(current_run, indent=2), encoding="utf-8"
    )

    print(f"Scanned {len(current_run['items'])} competitor products.")
    if current_run["alerts"]:
        print("ALERTS:")
        for alert in current_run["alerts"]:
            print(f"- {alert}")
    else:
        print("No price drops detected since last run.")
    return 0


if __name__ == "__main__":
    raise SystemExit(run_tracker())