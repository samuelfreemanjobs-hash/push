"""Deterministic tools for Deep Agents — thin wrappers over DiskLordz factory/VST modules."""

from __future__ import annotations

import os
import sys
from datetime import datetime, timezone
from pathlib import Path

_FRAMEWORK = Path(__file__).resolve().parent.parent / "ms-agent-framework"
if str(_FRAMEWORK) not in sys.path:
    sys.path.insert(0, str(_FRAMEWORK))

import audio_factory_tools as audio_factory
import vst_engineering_tools as vst_eng

WORKSPACE: Path = Path(".").resolve()
LOG_DIR: Path = WORKSPACE / ".deep-agent-logs"
VST_ROOT: Path = WORKSPACE
FACTORY_ROOT: Path = WORKSPACE


def _offload_large(text: str, prefix: str) -> str:
    """Write bulky tool output to workspace file; return short pointer for context compression."""
    LOG_DIR.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    path = LOG_DIR / f"{prefix}_{stamp}.log"
    if len(text) > 8000:
        path.write_text(text, encoding="utf-8", errors="replace")
        return f"[offloaded {len(text)} chars -> {path.relative_to(WORKSPACE)}]\n" + text[-3500:]
    return text


def configure_roots(vst_root: Path, factory_root: Path, workspace: Path | None = None) -> None:
    global VST_ROOT, FACTORY_ROOT, WORKSPACE
    VST_ROOT = vst_root.resolve()
    FACTORY_ROOT = factory_root.resolve()
    WORKSPACE = (workspace or vst_root).resolve()
    vst_eng.PROJECT_ROOT = VST_ROOT
    audio_factory.FACTORY_ROOT = FACTORY_ROOT


def build_and_validate_vst3() -> str:
    """CMake Release build + pluginval (Windows)."""
    out = vst_eng.run_cmake_build()
    if not out.startswith("MSVC BUILD SUCCEEDED"):
        return _offload_large(out, "msvc_build")
    qa = vst_eng.run_pluginval_tests()
    return _offload_large(qa, "pluginval")


def run_multi_sku_pack_factory(pack_prefix: str = "DKP") -> str:
    from multi_sku_exporter import export_all_commercial_skus

    result = export_all_commercial_skus(pack_title=pack_prefix)
    return _offload_large(str(result), "pack_factory")


def read_dsp_file(relative_path: str) -> str:
    return vst_eng.read_project_file(relative_path)


def write_dsp_file(relative_path: str, content: str) -> str:
    if not _human_approved_processor_edit():
        return (
            "HUMAN_INTERRUPT: PluginProcessor/src write blocked. "
            "Set DLZ_DEEP_AGENT_APPROVED=1 after review or resume graph with approval."
        )
    return vst_eng.write_project_file(relative_path, content)


def _human_approved_processor_edit() -> bool:
    return os.environ.get("DLZ_DEEP_AGENT_APPROVED", "0") == "1"