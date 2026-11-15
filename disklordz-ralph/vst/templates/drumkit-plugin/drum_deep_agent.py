#!/usr/bin/env python3
"""Run Deep Agent harness for this DrumKit project."""

from __future__ import annotations

import os
import runpy
import sys
from pathlib import Path

_PROJECT = Path(__file__).resolve().parent
_REPO = _PROJECT.parents[4]
_ENGINE = _REPO / "disklordz-ralph" / "business" / "deep-agents" / "drum_deep_agent.py"

os.environ.setdefault("DLZ_VST_PROJECT_ROOT", str(_PROJECT))
os.environ.setdefault("DLZ_REPO_ROOT", str(_REPO))

if not _ENGINE.is_file():
    raise SystemExit(f"Missing: {_ENGINE}")

sys.argv = [str(_ENGINE), *sys.argv[1:]]
runpy.run_path(str(_ENGINE), run_name="__main__")