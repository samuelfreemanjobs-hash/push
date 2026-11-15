#!/usr/bin/env python3
"""VST factory automation — Waza, EGM duels, Helix JSON, pod registry."""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

try:
    import yaml
except ImportError:
    yaml = None

ROOT = Path(__file__).resolve().parents[3]
AUTOMATION = Path(__file__).resolve().parent


def run(cmd: list[str], cwd: Path | None = None) -> int:
    print("+", " ".join(cmd), flush=True)
    return subprocess.run(cmd, cwd=cwd or ROOT).returncode


def load_specs() -> list[Path]:
    manifest = AUTOMATION / "specs.manifest.yaml"
    if yaml is None or not manifest.is_file():
        return [ROOT / "disklordz-ralph/labs/gpu-inference/README.md"]
    data = yaml.safe_load(manifest.read_text(encoding="utf-8"))
    paths = data.get("specs", []) if isinstance(data, dict) else []
    return [ROOT / p for p in paths]


def egm_duels() -> int:
    code = 0
    duel = ROOT / "openclaw/elite-frameworks/egm/run_spec_duel.py"
    for spec in load_specs():
        if not spec.is_file():
            print(f"WARN skip missing spec: {spec}", file=sys.stderr)
            continue
        rc = run([sys.executable, str(duel), "--spec", str(spec)])
        if rc != 0:
            code = rc
    return code


def waza() -> int:
    sh = ROOT / "openclaw/elite-frameworks/waza/evals/run_all.sh"
    return subprocess.run(["bash", str(sh)], cwd=ROOT).returncode


def helix_seed() -> int:
    seed = ROOT / "openclaw/elite-frameworks/helix-foundry/seed/vst-helix-seed.json"
    schema = ROOT / "openclaw/elite-frameworks/helix-foundry/schema/knowledge-graph.schema.json"
    try:
        data = json.loads(seed.read_text(encoding="utf-8"))
    except json.JSONDecodeError:
        print("FAIL helix seed JSON", file=sys.stderr)
        return 1
    if "nodes" not in data or "edges" not in data:
        print("FAIL helix seed shape", file=sys.stderr)
        return 1
    pods = ROOT / "openclaw/agents/vst-factory-pods.yaml"
    if not pods.is_file():
        print("FAIL missing vst-factory-pods.yaml", file=sys.stderr)
        return 1
    if schema.is_file():
        print(f"OK helix nodes={len(data['nodes'])} edges={len(data['edges'])}")
    return 0


def factory_pods_count() -> int:
    pods = ROOT / "openclaw/agents/vst-factory-pods.yaml"
    if yaml is None:
        return 0
    text = pods.read_text(encoding="utf-8")
    data = yaml.safe_load(text)
    n = len(data.get("pods", {})) if isinstance(data, dict) else 0
    if n < 7:
        print(f"FAIL expected 7 factory pods, got {n}", file=sys.stderr)
        return 1
    print(f"OK factory pods={n}")
    return 0


def main() -> int:
    steps = [
        ("waza", waza),
        ("egm", egm_duels),
        ("helix", helix_seed),
        ("pods", factory_pods_count),
    ]
    code = 0
    for name, fn in steps:
        print(f"\n=== {name} ===")
        rc = fn()
        if rc != 0:
            code = rc
    return code


if __name__ == "__main__":
    raise SystemExit(main())