#!/usr/bin/env python3
"""Document Parsing Protocol — markdown/text → structured chunks for RAG / Helix."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path


def chunk_markdown(text: str, source: str) -> list[dict]:
    sections: list[dict] = []
    current_title = "preamble"
    buffer: list[str] = []

    def flush() -> None:
        body = "\n".join(buffer).strip()
        if not body:
            return
        chunk_id = hashlib.sha256(f"{source}:{current_title}:{body[:80]}".encode()).hexdigest()[:16]
        sections.append(
            {
                "id": chunk_id,
                "source": source,
                "title": current_title,
                "text": body,
                "tokens_est": len(re.findall(r"\w+", body)),
            }
        )
        buffer.clear()

    for line in text.splitlines():
        if line.startswith("#"):
            flush()
            current_title = line.lstrip("#").strip()
        else:
            buffer.append(line)
    flush()
    return sections


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--in", dest="in_path", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    text = args.in_path.read_text(encoding="utf-8")
    chunks = chunk_markdown(text, str(args.in_path))
    args.out.write_text(json.dumps({"chunks": chunks}, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {len(chunks)} chunks → {args.out}")


if __name__ == "__main__":
    main()