#!/usr/bin/env python3
"""Single evaluation entry — updates results.tsv when invoked with --log."""

from __future__ import annotations

import argparse
import csv
import datetime as dt
from pathlib import Path

from evaluate import evaluate

RESULTS = Path(__file__).resolve().parent / "results.tsv"


def best_loss() -> float:
    if not RESULTS.is_file():
        return float("inf")
    with RESULTS.open(newline="") as f:
        rows = list(csv.DictReader(f, delimiter="\t"))
    if not rows:
        return float("inf")
    kept = [float(r["eval_loss"]) for r in rows if r.get("kept") == "1"]
    return min(kept) if kept else float("inf")


def append_row(m: dict, hypothesis: str, kept: int) -> None:
    write_header = not RESULTS.exists()
    with RESULTS.open("a", newline="") as f:
        w = csv.DictWriter(
            f,
            fieldnames=[
                "run_id",
                "timestamp",
                "eval_loss",
                "timbre",
                "crest_factor",
                "attack_ms",
                "thd_ratio",
                "render_ms",
                "safety_pass",
                "kept",
                "hypothesis",
            ],
            delimiter="\t",
        )
        if write_header:
            w.writeheader()
        w.writerow(
            {
                "run_id": dt.datetime.now(dt.timezone.utc).strftime("%Y%m%dT%H%M%S"),
                "timestamp": dt.datetime.now(dt.timezone.utc).isoformat().replace("+00:00", "Z"),
                "eval_loss": f"{m['eval_loss']:.6f}",
                "timbre": f"{m['timbre']:.6f}",
                "crest_factor": f"{m['crest_factor']:.4f}",
                "attack_ms": f"{m['attack_ms']:.4f}",
                "thd_ratio": f"{m['thd_ratio']:.4f}",
                "render_ms": f"{m['render_ms']:.4f}",
                "safety_pass": str(m["safety_pass"]),
                "kept": str(kept),
                "hypothesis": hypothesis[:500],
            }
        )


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--log", action="store_true", help="Append to results.tsv")
    ap.add_argument("--hypothesis", default="", help="Short description for log row")
    ap.add_argument("--kept", type=int, choices=(0, 1), default=0)
    args = ap.parse_args()

    m = evaluate()
    print(f"eval_loss={m['eval_loss']:.6f}")
    print(f"best_loss={best_loss():.6f}")

    if args.log:
        append_row(m, args.hypothesis, args.kept)

    return 0 if m["safety_pass"] else 2


if __name__ == "__main__":
    raise SystemExit(main())