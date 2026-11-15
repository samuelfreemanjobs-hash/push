#!/usr/bin/env python3
"""Run evaluate.py and optionally append results.tsv."""

from __future__ import annotations

import argparse
import csv
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

RESULTS = Path(__file__).resolve().parent / "results.tsv"


def run_evaluate() -> tuple[int, str]:
    proc = subprocess.run(
        [sys.executable, "evaluate.py"],
        cwd=RESULTS.parent,
        capture_output=True,
        text=True,
    )
    out = proc.stdout + proc.stderr
    print(out, end="" if out.endswith("\n") else "\n")
    return proc.returncode, out


def parse_losses(out: str) -> dict[str, float | None]:
    def grab(name: str) -> float | None:
        m = re.search(rf"{name}:\s*([0-9.]+)", out)
        return float(m.group(1)) if m else None

    return {
        "total_loss": grab("TOTAL_LOSS"),
        "kick_loss": grab("KICK_LOSS"),
        "snare_loss": grab("SNARE_LOSS"),
        "hat_loss": grab("HAT_LOSS"),
    }


def best_total() -> float:
    if not RESULTS.is_file():
        return float("inf")
    with RESULTS.open(newline="") as f:
        rows = list(csv.DictReader(f, delimiter="\t"))
    vals = []
    for r in rows:
        if r.get("status") == "kept" and r.get("total_loss"):
            try:
                vals.append(float(r["total_loss"]))
            except ValueError:
                pass
    return min(vals) if vals else float("inf")


def append_row(losses: dict[str, float | None], status: str, description: str) -> None:
    write_header = not RESULTS.exists() or RESULTS.stat().st_size < 16
    with RESULTS.open("a", newline="") as f:
        w = csv.DictWriter(
            f,
            fieldnames=[
                "iteration",
                "total_loss",
                "kick_loss",
                "snare_loss",
                "hat_loss",
                "status",
                "description",
            ],
            delimiter="\t",
        )
        if write_header:
            w.writeheader()
        w.writerow(
            {
                "iteration": datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S"),
                "total_loss": f"{losses.get('total_loss', 9999):.6f}",
                "kick_loss": f"{losses.get('kick_loss', 0):.6f}",
                "snare_loss": f"{losses.get('snare_loss', 0):.6f}",
                "hat_loss": f"{losses.get('hat_loss', 0):.6f}",
                "status": status,
                "description": description[:500],
            }
        )


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--log", action="store_true")
    ap.add_argument("--description", default="")
    ap.add_argument("--status", default="kept", choices=("kept", "reverted", "fail"))
    args = ap.parse_args()

    code, out = run_evaluate()
    losses = parse_losses(out)
    if losses["total_loss"] is not None:
        print(f"best_total={best_total():.6f}")

    if args.log:
        append_row(losses, "fail" if code != 0 else args.status, args.description)

    return code


if __name__ == "__main__":
    raise SystemExit(main())