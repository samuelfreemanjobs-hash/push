#!/usr/bin/env python3
"""TokenEconomizer — trim handoff markdown to line budget."""

from __future__ import annotations

import argparse
import re
from pathlib import Path

DEFAULT_MAX = 120


def _load_yaml_config(config: Path) -> dict:
    try:
        import yaml
    except ImportError:
        return {}
    if not config.is_file():
        return {}
    data = yaml.safe_load(config.read_text(encoding="utf-8"))
    return data if isinstance(data, dict) else {}


def load_max_lines(config: Path | None) -> int:
    if config is None or not config.is_file():
        return DEFAULT_MAX
    data = _load_yaml_config(config)
    return int(data.get("defaults", {}).get("max_handoff_lines", DEFAULT_MAX))


def prune(text: str, max_lines: int, patterns: list[str]) -> str:
    for pat in patterns:
        text = re.sub(pat, "", text, flags=re.M | re.S)
    lines = [ln for ln in text.splitlines() if ln.strip()]
    if len(lines) <= max_lines:
        return "\n".join(lines) + "\n"
    head = lines[: max_lines // 2]
    tail = lines[-(max_lines // 2) :]
    omitted = len(lines) - len(head) - len(tail)
    return "\n".join(head + [f"\n… [{omitted} lines omitted — run with smaller scope]\n"] + tail) + "\n"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--in", dest="in_path", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument(
        "--config",
        type=Path,
        default=Path(__file__).with_name("context_budget.yaml"),
    )
    args = parser.parse_args()
    max_lines = load_max_lines(args.config)
    data = _load_yaml_config(args.config)
    patterns = list(data.get("defaults", {}).get("strip_patterns", []))
    text = args.in_path.read_text(encoding="utf-8")
    args.out.write_text(prune(text, max_lines, patterns), encoding="utf-8")
    print(f"Pruned to ~{max_lines} lines → {args.out}")


if __name__ == "__main__":
    main()