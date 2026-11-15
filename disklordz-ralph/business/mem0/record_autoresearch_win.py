#!/usr/bin/env python3
"""Record a kept autoresearch win into Mem0 (results category)."""

from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path

from mem0_client import get_client, scope_ids

REPO_ROOT = Path(__file__).resolve().parents[3]

TRACKS = {
    "cpp": {
        "results": REPO_ROOT / "disklordz-ralph/vst/autoresearch-cpp/results.tsv",
        "sandbox": "disklordz-ralph/vst/autoresearch-cpp/Source/VoiceDSP.h",
        "label": "C++ VoiceDSP autoresearch",
    },
    "kit": {
        "results": REPO_ROOT / "disklordz-ralph/factory/autoresearch/results.tsv",
        "sandbox": "disklordz-ralph/factory/autoresearch/drum_engine.py",
        "label": "Python kit autoresearch",
    },
}


def last_results_line(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"results file not found: {path}")
    lines = [ln.strip() for ln in path.read_text(encoding="utf-8").splitlines() if ln.strip()]
    if not lines:
        raise SystemExit(f"results file is empty: {path}")
    return lines[-1]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--track", choices=sorted(TRACKS), default="cpp")
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print payload without calling Mem0",
    )
    args = parser.parse_args()
    meta = TRACKS[args.track]
    row = last_results_line(meta["results"])

    git_sha = os.environ.get("DLZ_MEM0_GIT_SHA", "").strip()
    sha_note = f" commit={git_sha}" if git_sha else ""

    user_content = (
        f"DiskLordz {meta['label']} WIN.{sha_note}\n"
        f"Sandbox: {meta['sandbox']}\n"
        f"results.tsv last row: {row}\n"
        f"Constraints: real-time safe, MIN_SPECTRAL_CONVERGENCE=0.08, judges immutable."
    )
    assistant_content = (
        "Stored as project memory (category: results). "
        "Reuse winning formulas; do not edit evaluate_vst.py or evaluate.py."
    )

    messages = [
        {"role": "user", "content": user_content},
        {"role": "assistant", "content": assistant_content},
    ]

    if args.dry_run:
        print("DRY RUN — would send to Mem0:")
        print("scopes:", scope_ids())
        for m in messages:
            print(f"  [{m['role']}] {m['content'][:500]}")
        return

    client = get_client()
    ids = scope_ids()
    # Mem0 extracts facts from the conversation; metadata scopes retention.
    client.add(
        messages,
        user_id=ids["user_id"],
        agent_id=ids["agent_id"],
        metadata={
            "app_id": ids["app_id"],
            "category": "results",
            "track": args.track,
            "sandbox_path": meta["sandbox"],
        },
    )
    print(f"Mem0: recorded {args.track} win for agent_id={ids['agent_id']}")


if __name__ == "__main__":
    main()