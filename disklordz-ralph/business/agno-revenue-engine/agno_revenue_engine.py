#!/usr/bin/env python3
"""
DiskLordz Agno AgentOS — custom sonic kit configurator + scheduled merch (opt-in).

Human confirmation on all storefront writes. Synthesis/VST positioning — not artist samples.
"""

from __future__ import annotations

import json
import os
import subprocess
import sys
from pathlib import Path
from typing import List

from pydantic import BaseModel, Field

ROOT = Path(__file__).resolve().parent
REPO = Path(os.environ.get("DLZ_REPO_ROOT", ROOT.parents[3])).resolve()
FACTORY = Path(os.environ.get("DLZ_AUDIO_FACTORY_ROOT", REPO / "disklordz-ralph/factory/audio-factory")).resolve()
VST_ROOT = Path(os.environ.get("DLZ_VST_PROJECT_ROOT", REPO / "disklordz-ralph/vst/templates/drumkit-plugin")).resolve()

DSP_HARNESS = FACTORY / "build/dsp_harness_artefacts/Release/dsp_harness.exe"
TEST_LOOP = FACTORY / "raw_audio/test_loop.wav"
PREVIEW_DIR = ROOT / "dist_release/Previews"
DB_PATH = ROOT / "tmp/agno_revenue.db"

try:
    from agno.agent import Agent
    from agno.db.sqlite import SqliteDb
    from agno.models.openai import OpenAIChat
    from agno.os import AgentOS
    from agno.tools import tool
    from agno.workflow import Step, Workflow
except ImportError as exc:
    raise SystemExit(
        "pip install -r requirements.txt (see docs/dev/agno-revenue-engine-opt-in.md)"
    ) from exc


# -----------------------------------------------------------------------------
# Structured configurator (frontend / API)
# -----------------------------------------------------------------------------


class VoiceSlotSpec(BaseModel):
    role: str = Field(..., description="Kick, 808, Snare, Hat, etc.")
    dna_profile: str = Field(..., description="DiskLordz DNA e.g. Good Money, Luxury Soul")
    saturation_drive: float = Field(0.5, ge=0.0, le=1.0)
    midi_note: int = Field(..., ge=0, le=127)


class CustomKitConfigResponse(BaseModel):
    kit_name: str
    voices: List[VoiceSlotSpec]
    recommended_vst_preset: str
    preview_audio_url: str
    total_price_usd: float
    estimated_lead_days: int = Field(0, description="Digital goods: 0")


# -----------------------------------------------------------------------------
# Knowledge retrieval (JSON stand-in for LanceDb hybrid search — extend later)
# -----------------------------------------------------------------------------


@tool
def search_kit_compatibility_rules(query: str) -> str:
    """Return trigger/MIDI/VST compatibility snippets from local knowledge JSON."""
    kb = ROOT / "knowledge" / "compatibility.json"
    if not kb.is_file():
        return json.dumps({"error": "knowledge/compatibility.json missing", "query": query})

    data = json.loads(kb.read_text(encoding="utf-8"))
    hits = [r for r in data.get("rules", []) if query.lower() in json.dumps(r).lower()][:8]
    return json.dumps({"query": query, "hits": hits})


# -----------------------------------------------------------------------------
# Tools (approval gates on writes)
# -----------------------------------------------------------------------------


@tool
def render_custom_kit_audio_preview(kit_slug: str, saturation_drive: float = 0.65) -> str:
    """Headless dsp_harness preview for configurator (Windows build required)."""
    PREVIEW_DIR.mkdir(parents=True, exist_ok=True)
    out_wav = PREVIEW_DIR / f"{kit_slug}_preview.wav"

    harness = DSP_HARNESS
    loop = TEST_LOOP
    if not harness.is_file():
        return f"PREVIEW_SKIPPED: build dsp_harness at {FACTORY} (Windows MSVC)"
    if not loop.is_file():
        return f"PREVIEW_SKIPPED: add {loop.name} under audio-factory/raw_audio/"

    try:
        subprocess.run(
            [str(harness), str(loop), str(out_wav), str(saturation_drive)],
            capture_output=True,
            timeout=120,
            check=False,
        )
    except OSError as exc:
        return f"PREVIEW_ERROR: {exc}"

    if out_wav.is_file():
        return f"/api/previews/{out_wav.name}"
    return "PREVIEW_FAILED"


@tool(requires_confirmation=True)
def publish_shopify_flash_promo(sku: str, discount_percent: int, email_subject: str) -> str:
    """Queue Shopify discount + email — AgentOS Approve/Reject card only."""
    return (
        f"PENDING_APPROVAL: {discount_percent}% off {sku} | "
        f"Campaign: {email_subject!r} — implement GraphQL after approval"
    )


@tool(requires_confirmation=True)
def publish_vst_release(version: str, release_notes: str) -> str:
    """Gate production VST publish — never auto-ship starter-cpp."""
    return f"PENDING_APPROVAL: VST release {version} — notes length {len(release_notes)}"


@tool(requires_confirmation=True)
def issue_refund(order_id: str, amount_usd: float, reason: str) -> str:
    return f"PENDING_APPROVAL: refund ${amount_usd:.2f} on {order_id} — {reason}"


# -----------------------------------------------------------------------------
# Agents & workflows
# -----------------------------------------------------------------------------


def run_qlib_forecast_step(step_input) -> str:
    from drum_qlib_forecaster import train_and_forecast

    csv_path = ROOT / "data/shopify_daily_sku_metrics.csv"
    if not csv_path.is_file():
        return '{"status":"skip","reason":"no shopify_daily_sku_metrics.csv"}'
    return train_and_forecast(csv_path)


def run_nightly_factory_step(step_input) -> str:
    """Idempotent hook — operator runs factory_swarm on Windows; this logs intent only."""
    manifest = FACTORY / "dist_sample_pack/manifest.json"
    if manifest.is_file():
        return f'{{"status":"ready","manifest":"{manifest.as_posix()}"}}'
    return '{"status":"awaiting","hint":"Drop WAVs in audio-factory/raw_audio or run factory_swarm.py"}'


db = SqliteDb(db_file=str(DB_PATH))
db_path_parent = DB_PATH.parent
db_path_parent.mkdir(parents=True, exist_ok=True)

kit_architect = Agent(
    id="custom-kit-architect",
    name="Custom Kit & Sound Architect",
    model=OpenAIChat(id=os.environ.get("AGNO_MODEL", "gpt-4o")),
    tools=[search_kit_compatibility_rules, render_custom_kit_audio_preview],
    output_schema=CustomKitConfigResponse,
    db=db,
    instructions=[
        "DiskLordz synthesis-only kits: map natural language to DNA profiles and MIDI notes.",
        "Query search_kit_compatibility_rules for Roland/GM mapping hints.",
        "Call render_custom_kit_audio_preview (drive 0.75 dry vintage, 0.25 clean modern).",
        "Return strict CustomKitConfigResponse JSON.",
        "Never claim copyrighted samples or artist endorsements.",
    ],
)

merch_strategist = Agent(
    id="merch-strategist",
    name="Revenue & Merchandising Strategist",
    model=OpenAIChat(id=os.environ.get("AGNO_MODEL", "gpt-4o")),
    tools=[publish_shopify_flash_promo, publish_vst_release, issue_refund],
    db=db,
    instructions=[
        "Review forecast JSON. If velocity < 8 units/day, propose flash promo via publish_shopify_flash_promo.",
        "All publish tools pause for owner approval — do not assume approval.",
    ],
)

weekly_revenue_workflow = Workflow(
    id="weekly-revenue-optimizer",
    name="Weekly Qlib Demand & Promo Optimizer",
    db=db,
    steps=[
        Step(name="QlibForecast", executor=run_qlib_forecast_step),
        Step(name="MerchandisingDecision", agent=merch_strategist),
    ],
)

nightly_factory_workflow = Workflow(
    id="nightly-factory-prep",
    name="Nightly Sample Pack Prep Check",
    db=db,
    steps=[Step(name="FactoryManifestCheck", executor=run_nightly_factory_step)],
)

agent_os = AgentOS(
    agents=[kit_architect, merch_strategist],
    workflows=[weekly_revenue_workflow, nightly_factory_workflow],
    scheduler=True,
)

app = agent_os.get_app()

if __name__ == "__main__":
    port = int(os.environ.get("AGNO_PORT", "7777"))
    agent_os.serve(app="agno_revenue_engine:app", port=port, reload=os.environ.get("AGNO_RELOAD", "0") == "1")