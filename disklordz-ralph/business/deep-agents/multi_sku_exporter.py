"""Commercial SKU export stub — WAV pack + presets; MPC .xpm when verified."""

from __future__ import annotations

import json
from pathlib import Path

import audio_factory_tools as af


def export_all_commercial_skus(pack_title: str = "DKP") -> dict:
    master = json.loads(af.process_and_master_sample_pack(pack_title))
    map_msg = af.generate_velocity_layer_map()
    xpm = Path(af.FACTORY_ROOT) / "dist_presets" / f"{pack_title}_mpc.xpm"
    xpm.write_text(
        "; Akai MPC program placeholder — implement verified XPM writer in M2\n",
        encoding="utf-8",
    )
    return {
        "pack_title": pack_title,
        "mastering": master,
        "velocity_map": map_msg,
        "mpc_xpm": str(xpm.relative_to(af.FACTORY_ROOT)),
        "status": "STAGED_NOT_SHIPPED",
    }