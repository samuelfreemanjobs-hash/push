"""Waza — repeatable checks that agent outputs match repo contracts."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[4]


def test_neural_loop_generator_documents_rt_path() -> None:
    header = REPO / "disklordz-ralph/labs/gpu-inference/include/NeuralLoopGeneratorThread.h"
    assert header.is_file(), "Neural TRT thread header missing"
    text = header.read_text(encoding="utf-8")
    assert "Thread" in text
    assert "never" in text.lower() and "processblock" in text.lower()


def test_agents_md_forbids_rt_allocations() -> None:
    agents = REPO / "disklordz-ralph/vst/templates/drumkit-plugin/AGENTS.md"
    text = agents.read_text(encoding="utf-8")
    assert "processBlock" in text
    assert "allocation" in text.lower() or "heap" in text.lower()


def test_kit_mastering_naming_contract() -> None:
    mastering = REPO / "disklordz-ralph/business/trap-phonk-factory/kit_mastering.py"
    text = mastering.read_text(encoding="utf-8")
    assert "BPM_" in text


def test_storefront_demos_present() -> None:
    demos = REPO / "storefront/producer-storefront/public/demos"
    for name in ("808.wav", "cowbell.wav", "trap_bell.wav", "snare.wav"):
        assert (demos / name).is_file(), f"missing demo {name}"


def test_factory_handoff_report() -> None:
    handoff = REPO / "docs/launch/factory-handoff-latest.json"
    if not handoff.is_file():
        return
    data = json.loads(handoff.read_text(encoding="utf-8"))
    assert data.get("pipeline") == "pass"


def test_producer_storefront_buildable() -> None:
    pkg = REPO / "storefront/producer-storefront/package.json"
    assert pkg.is_file()
    data = json.loads(pkg.read_text(encoding="utf-8"))
    assert "build" in data.get("scripts", {})


def test_vst_factory_pods_registry() -> None:
    pods = REPO / "openclaw/agents/vst-factory-pods.yaml"
    text = pods.read_text(encoding="utf-8")
    assert "vst-architect" in text and "vst-qa" in text
    assert text.count("tier: factory") >= 7


def test_elite_frameworks_manifest_lists_fourteen() -> None:
    manifest = REPO / "openclaw/elite-frameworks/manifest.yaml"
    text = manifest.read_text(encoding="utf-8")
    keys = re.findall(r"^  \w+:\s*$", text, re.M)
    assert len(keys) >= 14, f"expected 14 frameworks, got {len(keys)}: {keys}"


def test_helix_seed_valid_json() -> None:
    seed = REPO / "openclaw/elite-frameworks/helix-foundry/seed/vst-helix-seed.json"
    data = json.loads(seed.read_text(encoding="utf-8"))
    assert "nodes" in data and "edges" in data
    assert len(data["nodes"]) >= 3


def main() -> int:
    tests = [
        test_neural_loop_generator_documents_rt_path,
        test_agents_md_forbids_rt_allocations,
        test_kit_mastering_naming_contract,
        test_storefront_demos_present,
        test_factory_handoff_report,
        test_producer_storefront_buildable,
        test_vst_factory_pods_registry,
        test_elite_frameworks_manifest_lists_fourteen,
        test_helix_seed_valid_json,
    ]
    for fn in tests:
        fn()
        print(f"ok {fn.__name__}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)