#!/usr/bin/env python3
"""
Nightly trap/phonk factory runner (Hermes roster template).

Uses in-repo tools until DeepSeek Harness + model inference are operator-installed.
"""

from __future__ import annotations

import json
import os
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parent
_repo_guess = ROOT.parents[3]
if not (_repo_guess / "openclaw").is_dir() and (ROOT.parents[4] / "openclaw").is_dir():
    _repo_guess = ROOT.parents[4]
REPO = Path(os.environ.get("DLZ_REPO_ROOT", _repo_guess))
FACTORY = REPO / "disklordz-ralph/factory/audio-factory"
OUT = ROOT / "outputs" / datetime.now(timezone.utc).strftime("%Y%m%d")


def run_memphis_architect_stub(genre: str, bpm: int, key: str) -> Path:
    """Replace with ace_step_xl_infer when licensed GPU batch is online."""
    raw = FACTORY / "raw_audio"
    OUT.mkdir(parents=True, exist_ok=True)
    staged = OUT / "raw_stems"
    staged.mkdir(exist_ok=True)
    meta = {"genre": genre, "bpm": bpm, "key": key, "status": "STUB_COPY_FROM_raw_audio"}
    for wav in sorted(raw.glob("*.wav"))[:10]:
        dest = staged / wav.name
        if not dest.exists():
            dest.write_bytes(wav.read_bytes())
    (staged / "_architect.json").write_text(json.dumps(meta, indent=2), encoding="utf-8")
    return staged


def run_kit_mastering(input_dir: Path) -> Path:
    mastered = OUT / "mastered"
    zip_path = OUT / "lead_magnet.zip"
    cmd = [
        sys.executable,
        str(ROOT / "kit_mastering.py"),
        "--in-dir",
        str(input_dir),
        "--out-dir",
        str(mastered),
        "--style",
        "DriftPhonk",
        "--bpm",
        "145",
        "--zip",
        str(zip_path),
    ]
    subprocess.run(cmd, check=True)
    return mastered


def run_promo_stub(kit_path: Path) -> dict:
    """PromoReelDirector — FFmpeg script placeholder."""
    meta = {
        "kit_path": str(kit_path),
        "video": "PENDING_ffmpeg",
        "cta": "Unlock Full Producer Bundle",
        "spec": "1080x1920 15s",
    }
    (OUT / "promo_meta.json").write_text(json.dumps(meta, indent=2), encoding="utf-8")
    return meta


def nightly_pack_drop() -> None:
    print(f"[{datetime.now(timezone.utc).isoformat()}] trap-phonk-factory")
    stems = run_memphis_architect_stub("Drift Phonk", 145, "Fmin")
    packaged = run_kit_mastering(stems)
    promo = run_promo_stub(packaged)
    print(json.dumps({"packaged": str(packaged), "promo": promo}, indent=2))


if __name__ == "__main__":
    nightly_pack_drop()