#!/usr/bin/env python3
"""
Elephant-Goldfish Model (EGM) — static spec duel without calling an LLM.

Elephant: missing sections / depth gaps (expand before code).
Goldfish: bloat, missing acceptance tests, RT-safety omissions (challenge scope).
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path

ELEPHANT_SECTIONS = (
    "goal",
    "scope",
    "non-goals",
    "architecture",
    "rt safety",
    "acceptance",
    "test plan",
    "rollout",
)

GOLDFISH_MAX_WORDS = 2500
GOLDFISH_MAX_SECTIONS_WITHOUT_TESTS = 6


@dataclass
class Finding:
    actor: str
    severity: str
    message: str


def word_count(text: str) -> int:
    return len(re.findall(r"\w+", text))


def section_hits(text: str) -> set[str]:
    lower = text.lower()
    found: set[str] = set()
    for sec in ELEPHANT_SECTIONS:
        if sec.replace(" ", "") in lower.replace(" ", "") or sec in lower:
            found.add(sec)
        for line in text.splitlines():
            if line.strip().startswith("#") and sec.split()[0] in line.lower():
                found.add(sec)
    return found


def elephant_findings(text: str) -> list[Finding]:
    findings: list[Finding] = []
    hits = section_hits(text)
    missing = [s for s in ELEPHANT_SECTIONS if s not in hits]
    for sec in missing:
        findings.append(
            Finding("elephant", "warn", f"Missing or weak section: `{sec}` — expand before C++/DSP commits.")
        )
    if "processblock" not in text.lower() and "vst" in text.lower():
        findings.append(
            Finding(
                "elephant",
                "error",
                "VST-related spec lacks `processBlock` / RT-safety language — add Ponytail/REA constraints.",
            )
        )
    if "pluginval" not in text.lower() and "vst" in text.lower():
        findings.append(
            Finding("elephant", "warn", "No pluginval gate mentioned — link `scripts/vst-build-pluginval-gate.ps1`.")
        )
    return findings


def goldfish_findings(text: str) -> list[Finding]:
    findings: list[Finding] = []
    wc = word_count(text)
    if wc > GOLDFISH_MAX_WORDS:
        findings.append(
            Finding(
                "goldfish",
                "warn",
                f"Spec is {wc} words (>{GOLDFISH_MAX_WORDS}) — split into PRD + ADR or defer non-ship scope.",
            )
        )
    hits = section_hits(text)
    has_tests = "test plan" in hits or "acceptance" in hits
    if len(hits) >= GOLDFISH_MAX_SECTIONS_WITHOUT_TESTS and not has_tests:
        findings.append(
            Finding(
                "goldfish",
                "error",
                "Many sections but no acceptance/test plan — agents will over-build; add Waza eval hooks.",
            )
        )
    vague = len(re.findall(r"\b(etc|TODO|TBD|maybe|eventually)\b", text, re.I))
    if vague > 8:
        findings.append(
            Finding("goldfish", "warn", f"High vagueness count ({vague}) — lock decisions or move to non-goals.")
        )
    return findings


def run_duel(spec_path: Path) -> int:
    text = spec_path.read_text(encoding="utf-8")
    findings = elephant_findings(text) + goldfish_findings(text)
    errors = sum(1 for f in findings if f.severity == "error")

    print(f"# EGM spec duel: {spec_path}\n")
    for actor in ("elephant", "goldfish"):
        actor_rows = [f for f in findings if f.actor == actor]
        if not actor_rows:
            print(f"## {actor.title()}\n- OK — no findings.\n")
            continue
        print(f"## {actor.title()}\n")
        for f in actor_rows:
            print(f"- [{f.severity.upper()}] {f.message}")
        print()

    if errors:
        print(f"**Verdict:** BLOCK ({errors} error(s)) — fix before implementation.")
        return 1
    if findings:
        print("**Verdict:** WARN — proceed with tightened scope.")
        return 0
    print("**Verdict:** PASS — spec is duel-clean.")
    return 0


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--spec", type=Path, required=True, help="Markdown spec or README to challenge")
    args = parser.parse_args()
    if not args.spec.is_file():
        print(f"Spec not found: {args.spec}", file=sys.stderr)
        raise SystemExit(2)
    raise SystemExit(run_duel(args.spec))


if __name__ == "__main__":
    main()