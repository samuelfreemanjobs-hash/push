"""Local file + build tools for AutoGen VST R&D swarm (Windows operator)."""

from __future__ import annotations

import os
import subprocess
from pathlib import Path

# Set by vst_rd_swarm before agents run
PROJECT_ROOT: Path = Path(".").resolve()

ALLOWED_WRITE = frozenset({"CMakeLists.txt"})
ALLOWED_WRITE_PREFIXES = ("src/", "ui/")


def _resolve_under_root(relative_path: str) -> Path | None:
    rel = relative_path.replace("\\", "/").lstrip("/")
    target = (PROJECT_ROOT / rel).resolve()
    try:
        target.relative_to(PROJECT_ROOT.resolve())
    except ValueError:
        return None
    return target


def _write_allowed(relative_path: str) -> bool:
    rel = relative_path.replace("\\", "/").lstrip("/")
    if rel in ALLOWED_WRITE:
        return True
    return any(rel.startswith(p) for p in ALLOWED_WRITE_PREFIXES)


def read_project_file(relative_path: str) -> str:
    """Read a C++, CMake, or UI file under the JUCE project root."""
    target = _resolve_under_root(relative_path)
    if target is None or not target.is_file():
        return f"Error: File '{relative_path}' not found or blocked."
    if target.stat().st_size > 512_000:
        return f"Error: File too large ({target.stat().st_size} bytes)."
    return target.read_text(encoding="utf-8", errors="replace")


def write_project_file(relative_path: str, content: str) -> str:
    """Write or update an allowed project file (src/, ui/, CMakeLists.txt only)."""
    if not _write_allowed(relative_path):
        return f"Error: Writes blocked for '{relative_path}' (allowed: src/, ui/, CMakeLists.txt)."
    if len(content) > 400_000:
        return "Error: Content too large."
    target = _resolve_under_root(relative_path)
    if target is None:
        return "Error: Path traversal blocked."
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(content, encoding="utf-8")
    return f"Wrote {len(content)} bytes to {relative_path}"


def _find_vst3_bundle() -> Path | None:
    build = PROJECT_ROOT / "build"
    if not build.is_dir():
        return None
    matches = sorted(build.glob("**/*.vst3"), key=lambda p: len(p.parts))
    preferred = os.environ.get("DLZ_VST3_PRODUCT_NAME", "").strip()
    if preferred:
        for p in matches:
            if p.name == preferred or preferred in str(p):
                return p
    return matches[0] if matches else None


def _pluginval_exe() -> Path | None:
    for key in ("DLZ_PLUGINVAL", "PLUGINVAL_PATH"):
        raw = os.environ.get(key, "").strip()
        if raw and Path(raw).is_file():
            return Path(raw)
    repo = os.environ.get("DLZ_REPO_ROOT", "").strip()
    if repo:
        candidate = Path(repo) / "disklordz-ralph/vst/tools/pluginval/pluginval.exe"
        if candidate.is_file():
            return candidate
    return None


def run_cmake_build() -> str:
    """Configure (if needed) and compile Release on Windows MSVC."""
    if os.name != "nt":
        return "Error: run_cmake_build requires Windows (MSVC)."

    extra = []
    if os.environ.get("DLZ_USE_MONOREPO_JUCE", "1") == "1":
        extra = ["-DDLZ_USE_MONOREPO_JUCE=ON"]

    build_dir = PROJECT_ROOT / "build"
    try:
        if not build_dir.exists():
            cfg = subprocess.run(
                [
                    "cmake",
                    "-S",
                    ".",
                    "-B",
                    "build",
                    "-G",
                    "Visual Studio 17 2022",
                    "-A",
                    "x64",
                    *extra,
                ],
                cwd=PROJECT_ROOT,
                capture_output=True,
                text=True,
                timeout=300,
            )
            if cfg.returncode != 0:
                return f"CMAKE CONFIGURE FAILED:\n{cfg.stderr[-2500:]}\n{cfg.stdout[-2500:]}"

        bld = subprocess.run(
            ["cmake", "--build", "build", "--config", "Release", "--parallel"],
            cwd=PROJECT_ROOT,
            capture_output=True,
            text=True,
            timeout=600,
        )
        if bld.returncode != 0:
            return f"MSVC BUILD FAILED:\n{bld.stdout[-4000:]}\n{bld.stderr[-2000:]}"
        return "MSVC BUILD SUCCEEDED."
    except subprocess.TimeoutExpired:
        return "Error: build timed out."
    except OSError as exc:
        return f"Build exception: {exc}"


def run_pluginval_tests() -> str:
    """Run pluginval on the built VST3 bundle."""
    exe = _pluginval_exe()
    if exe is None:
        return "Error: pluginval.exe not found. Set DLZ_PLUGINVAL or install under vst/tools/pluginval/."
    bundle = _find_vst3_bundle()
    if bundle is None:
        return "Error: No .vst3 under build/. Run run_cmake_build first."

    strict = os.environ.get("PLUGINVAL_STRICTNESS", "8")
    try:
        res = subprocess.run(
            [str(exe), f"--strictness-level", strict, "--validate", str(bundle)],
            cwd=PROJECT_ROOT,
            capture_output=True,
            text=True,
            timeout=300,
        )
        if res.returncode != 0:
            return f"PLUGINVAL FAILED (strictness {strict}):\n{res.stdout[-3500:]}\n{res.stderr[-1500:]}"
        return f"PLUGINVAL PASSED (strictness {strict}) on {bundle.name}."
    except subprocess.TimeoutExpired:
        return "Error: pluginval timed out."
    except OSError as exc:
        return f"pluginval exception: {exc}"