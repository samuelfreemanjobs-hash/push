#!/usr/bin/env python3
"""Launch unified Audio Factory swarm from this directory."""

from __future__ import annotations

import os
import runpy
import sys
from pathlib import Path

_FACTORY = Path(__file__).resolve().parent
_REPO = _FACTORY.parents[2]

os.environ.setdefault("DLZ_AUDIO_FACTORY_ROOT", str(_FACTORY))
os.environ.setdefault("DLZ_REPO_ROOT", str(_REPO))

_FRAMEWORK = _REPO / "disklordz-ralph" / "business" / "ms-agent-framework" / "factory_swarm.py"
if not _FRAMEWORK.is_file():
    raise SystemExit(f"Missing: {_FRAMEWORK}")

sys.argv = [str(_FRAMEWORK), *sys.argv[1:]]
runpy.run_path(str(_FRAMEWORK), run_name="__main__")