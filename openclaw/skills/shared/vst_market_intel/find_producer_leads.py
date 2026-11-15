#!/usr/bin/env python3
"""
Harvest public promo emails from NOLA bounce / BR / phonk type-beat producers (Firecrawl Agent).
Appends deduped rows to docs/outreach/producer_leads.csv — drafts and sends stay human-in-the-loop.
"""
from __future__ import annotations

import csv
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


class ProducerLead(BaseModel):
    producer_name: str = Field(description="Producer or channel name")
    niche: str = Field(description="Primary beat style, e.g. Baton Rouge, NOLA Bounce, Phonk")
    platform_url: str = Field(description="YouTube channel, BeatStars, or Instagram URL")
    public_email: Optional[str] = Field(
        default=None, description="Public business/promo email from bio or description"
    )
    recent_beat_title: Optional[str] = Field(
        default=None, description="Recent beat upload title for personalization"
    )


class LeadBatch(BaseModel):
    leads: List[ProducerLead] = Field(default_factory=list)


HARVEST_PROMPT = (
    "Find up to 15 active YouTube or BeatStars music producers uploading "
    "'NBA YoungBoy type beat', 'Baton Rouge type beat', 'New Orleans bounce beat', "
    "or 'Memphis phonk type beat' in 2025 or 2026. "
    "For each, extract producer name, primary niche, channel or store URL, "
    "one recent beat title, and any publicly listed business or promo email "
    "from their bio or video descriptions (beat inquiries only — no guessing private emails)."
)

CSV_FIELDS = [
    "producer_name",
    "niche",
    "platform_url",
    "public_email",
    "recent_beat_title",
    "status",
    "harvested_at",
]


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


def normalize_email(value: str | None) -> str:
    if not value:
        return ""
    return value.lower().strip()


def extract_lead_rows(result: Any) -> list[dict]:
    data: Any = None
    if result is None:
        return []
    if hasattr(result, "data"):
        data = result.data
    elif isinstance(result, dict):
        data = result.get("data") or result

    if isinstance(data, LeadBatch):
        return [lead.model_dump() for lead in data.leads]
    if isinstance(data, dict) and "leads" in data:
        rows = []
        for raw in data["leads"]:
            try:
                rows.append(ProducerLead.model_validate(raw).model_dump())
            except Exception:
                if isinstance(raw, dict):
                    rows.append(raw)
        return rows
    return []


def run_harvest() -> int:
    api_key = os.environ.get("FIRECRAWL_API_KEY")
    if not api_key:
        print("Missing FIRECRAWL_API_KEY", file=sys.stderr)
        return 2

    app = Firecrawl(api_key=api_key)
    root = repo_root()
    out_dir = root / "docs" / "outreach"
    drafts_dir = out_dir / "drafts"
    out_dir.mkdir(parents=True, exist_ok=True)
    drafts_dir.mkdir(parents=True, exist_ok=True)
    csv_path = out_dir / "producer_leads.csv"

    seen_emails: set[str] = set()
    if csv_path.exists():
        with csv_path.open("r", encoding="utf-8") as handle:
            for row in csv.DictReader(handle):
                email = normalize_email(row.get("public_email"))
                if email and "@" in email:
                    seen_emails.add(email)

    max_credits = os.environ.get("FIRECRAWL_AGENT_MAX_CREDITS")
    agent_kwargs: dict[str, Any] = {
        "prompt": HARVEST_PROMPT,
        "schema": LeadBatch,
    }
    if max_credits:
        try:
            agent_kwargs["maxCredits"] = int(max_credits)
        except ValueError:
            pass

    try:
        response = app.agent(**agent_kwargs)
    except Exception as exc:
        print(f"Firecrawl agent failed: {exc}", file=sys.stderr)
        return 1

    harvested_at = datetime.now(timezone.utc).isoformat()
    new_leads: list[dict[str, str]] = []

    for lead in extract_lead_rows(response):
        email = normalize_email(lead.get("public_email"))
        if not email or "@" not in email or email in seen_emails:
            continue
        seen_emails.add(email)
        row = {
            "producer_name": str(lead.get("producer_name", "")).strip() or "unknown",
            "niche": str(lead.get("niche", "")).strip() or "type beat",
            "platform_url": str(lead.get("platform_url", "")).strip(),
            "public_email": email,
            "recent_beat_title": str(lead.get("recent_beat_title", "") or "").strip(),
            "status": "pending_review",
            "harvested_at": harvested_at,
        }
        new_leads.append(row)

    file_exists = csv_path.exists()
    with csv_path.open("a", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=CSV_FIELDS)
        if not file_exists:
            writer.writeheader()
        writer.writerows(new_leads)

    summary = {
        "timestamp": harvested_at,
        "added": len(new_leads),
        "csv_path": str(csv_path.relative_to(root)) if csv_path.is_relative_to(root) else str(csv_path),
        "pending_review": len(new_leads),
        "leads_preview": new_leads[:5],
    }
    summary_path = out_dir / "latest_harvest.json"
    summary_path.write_text(json.dumps(summary, indent=2), encoding="utf-8")

    hermes = hermes_out()
    hermes.mkdir(parents=True, exist_ok=True)
    (hermes / "producer-leads-latest.json").write_text(
        json.dumps(summary, indent=2), encoding="utf-8"
    )

    print(f"Added {len(new_leads)} new verified producer leads to {csv_path}.")
    return 0


if __name__ == "__main__":
    raise SystemExit(run_harvest())