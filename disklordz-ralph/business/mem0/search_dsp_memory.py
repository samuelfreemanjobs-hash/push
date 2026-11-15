#!/usr/bin/env python3
"""Search Mem0 for DiskLordz DSP / kit memories (CLI for coordinators)."""

from __future__ import annotations

import argparse
import json

from mem0_client import get_client, scope_ids


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("query", help="Natural-language search (e.g. pitch envelope 808)")
    parser.add_argument("--limit", type=int, default=5)
    args = parser.parse_args()

    client = get_client()
    ids = scope_ids()
    hits = client.search(
        args.query,
        user_id=ids["user_id"],
        agent_id=ids["agent_id"],
        limit=args.limit,
    )

    # SDK may return list or dict with "results" — normalize for scripts.
    if isinstance(hits, dict) and "results" in hits:
        rows = hits["results"]
    else:
        rows = hits

    print(json.dumps(rows, indent=2, default=str))


if __name__ == "__main__":
    main()