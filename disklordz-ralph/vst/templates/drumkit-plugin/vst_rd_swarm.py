#!/usr/bin/env python3
"""Run the DiskLordz AutoGen VST swarm against this DrumKitPlugin folder."""

from __future__ import annotations

import os
import runpy
import sys
from pathlib import Path

_PROJECT = Path(__file__).resolve().parent
_REPO = _PROJECT.parents[3]

os.environ.setdefault("DLZ_VST_PROJECT_ROOT", str(_PROJECT))
os.environ.setdefault("DLZ_REPO_ROOT", str(_REPO))

_FRAMEWORK = _REPO / "disklordz-ralph" / "business" / "ms-agent-framework" / "vst_rd_swarm.py"
if not _FRAMEWORK.is_file():
    raise SystemExit(f"Missing framework script: {_FRAMEWORK}")

sys.argv = [str(_FRAMEWORK), *sys.argv[1:]]
runpy.run_path(str(_FRAMEWORK), run_name="__main__")