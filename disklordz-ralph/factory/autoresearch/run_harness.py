#!/usr/bin/env python3
"""Parse evaluate.py output; optional log to results.tsv."""

from __future__ import annotations

import argparse
import csv
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

RESULTS = Path(__file__).resolve().parent / "results.tsv"


def run_evaluate() -> tuple[float | None, str]:
    proc = subprocess.run(
        [sys.executable, "evaluate.py"],
        cwd=RESULTS.parent,
        capture_output=True,
        text=True,
    )
    out = proc.stdout + proc.stderr
    m = re.search(r"TOTAL_LOSS:\s*([0-9.]+)", out)
    loss = float(m.group(1)) if m else None
    print(out, end="" if out.endswith("\n") else "\n")
    return loss, out


def best_loss() -> float:
    if not RESULTS.is_file():
        return float("inf")
    with RESULTS.open(newline="") as f:
        rows = list(csv.DictReader(f, delimiter="\t"))
    kept = []
    for r in rows:
        if r.get("status") == "kept" and r.get("loss"):
            try:
                kept.append(float(r["loss"]))
            except ValueError:
                pass
    return min(kept) if kept else float("inf")


def append_row(loss: float, status: str, description: str) -> None:
    write_header = not RESULTS.exists() or RESULTS.stat().st_size < 8
    with RESULTS.open("a", newline="") as f:
        w = csv.DictWriter(f, fieldnames=["iteration", "loss", "status", "description"], delimiter="\t")
        if write_header:
            w.writeheader()
        w.writerow(
            {
                "iteration": datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S"),
                "loss": f"{loss:.6f}",
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

    loss, out = run_evaluate()
    if loss is None or "ERROR:" in out:
        if args.log:
            append_row(9999.0, "fail", args.description or "evaluate error")
        return 2

    print(f"best_loss={best_loss():.6f}")
    if args.log:
        append_row(loss, args.status, args.description)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())