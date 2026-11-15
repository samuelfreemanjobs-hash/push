#!/usr/bin/env python3
"""
Stage the latest commercial pack into OpenMontage assets/latest_release/.

Run after export_kit.py:
  python publish_to_openmontage.py
  python publish_to_openmontage.py --pack-dir outputs/packs/<folder>
"""

from __future__ import annotations

import json
import shutil
from datetime import datetime, timezone
from pathlib import Path

from handoff_openmontage import (
    BEST_KIT,
    HERO_VELOCITY_SUFFIX,
    REPO_ROOT,
    _latest_pack_dir,
    _marketing_root,
    _pick_hero_wavs,
)

LATEST_RELEASE = "latest_release"

HERO_OUT_NAMES = {
    "Kicks": "hero_kick.wav",
    "Snares": "hero_snare.wav",
    "Hats": "hero_hat.wav",
}


def _hero_by_category(pack_dir: Path, manifest: dict) -> dict[str, Path]:
    picked = _pick_hero_wavs(pack_dir, manifest)
    by_cat: dict[str, Path] = {}
    for wav in picked:
        for cat in HERO_VELOCITY_SUFFIX:
            if wav.parent.name == cat:
                by_cat[cat] = wav
                break
    for cat in HERO_VELOCITY_SUFFIX:
        if cat in by_cat:
            continue
        vel = HERO_VELOCITY_SUFFIX[cat]
        for entry in manifest.get("samples", []):
            if entry.get("category") != cat or entry.get("velocity_layer") != vel:
                continue
            rel = entry.get("file", "").replace("\\", "/")
            src = pack_dir / rel
            if src.is_file():
                by_cat[cat] = src
            break
    return by_cat


def stage_latest_release(
    pack_dir: Path | None = None,
    marketing_root: Path | None = None,
) -> Path:
    pack = (pack_dir or _latest_pack_dir()).resolve()
    root = _marketing_root(marketing_root)
    dest = root / "assets" / LATEST_RELEASE
    if dest.exists():
        for child in dest.iterdir():
            if child.is_file():
                child.unlink()
            elif child.is_dir():
                shutil.rmtree(child)
    dest.mkdir(parents=True, exist_ok=True)

    manifest_path = pack / "manifest.json"
    if not manifest_path.is_file():
        raise FileNotFoundError(f"Missing manifest: {manifest_path}")

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    shutil.copy2(manifest_path, dest / "manifest.json")

    sfz = pack / "DiskLordz_Kit.sfz"
    if sfz.is_file():
        shutil.copy2(sfz, dest / "DiskLordz_Kit.sfz")

    if BEST_KIT.is_file():
        shutil.copy2(BEST_KIT, dest / "demo_beat.wav")
    else:
        print("Note: outputs/best_kit.wav missing — run evaluate.py or add demo manually.")

    heroes = _hero_by_category(pack, manifest)
    audio_assets: dict[str, str] = {
        "main_track": f"assets/{LATEST_RELEASE}/demo_beat.wav",
    }
    solo_keys = {
        "Kicks": "solo_kick",
        "Snares": "solo_snare",
        "Hats": "solo_hat",
    }
    for cat, out_name in HERO_OUT_NAMES.items():
        src = heroes.get(cat)
        if src and src.is_file():
            shutil.copy2(src, dest / out_name)
            audio_assets[solo_keys[cat]] = f"assets/{LATEST_RELEASE}/{out_name}"

    pack_name = manifest.get("pack_name") or manifest.get("title") or pack.name
    total = manifest.get("sample_count", len(manifest.get("samples", [])))
    brief = {
        "release_timestamp": datetime.now(timezone.utc).isoformat().replace("+00:00", "Z"),
        "brand": "DiskLordz",
        "pack_name": pack_name,
        "format": manifest.get("format", "24-bit / 48kHz mono WAV"),
        "total_samples": total,
        "velocity_layers": sorted(
            {s.get("velocity_layer") for s in manifest.get("samples", []) if s.get("velocity_layer")}
        ),
        "source_pack_dir": str(pack),
        "audio_assets": audio_assets,
        "openmontage_templates": {
            "drum_kit_short_9x16": "docs/marketing/openmontage-prompt-templates.md#template-a",
            "vst_trailer_16x9": "docs/marketing/openmontage-prompt-templates.md#template-b",
        },
    }
    (dest / "release_brief.json").write_text(json.dumps(brief, indent=2), encoding="utf-8")

    prompt = f"""# OpenMontage — latest release ({pack_name})

Read `release_brief.json` and `manifest.json` in this folder.

## Template A (default): 25s 9:16 drum kit Short
See repo: `{REPO_ROOT / "docs/marketing/openmontage-prompt-templates.md"}` section **Template A**.

Assets (relative to marketing-studio root):
- `assets/{LATEST_RELEASE}/demo_beat.wav`
- `assets/{LATEST_RELEASE}/hero_kick.wav`
- `assets/{LATEST_RELEASE}/hero_snare.wav`
- `assets/{LATEST_RELEASE}/hero_hat.wav`

**Pause after storyboard** before final render.

Brand: gunmetal `#121216`, amber accents, synthesis-only, no purple AI gradients.
"""
    (dest / "AGENT_PROMPT.md").write_text(prompt, encoding="utf-8")

    print(f"Staged release '{pack_name}' into OpenMontage: {dest}")
    return dest


def main() -> None:
    import argparse

    ap = argparse.ArgumentParser()
    ap.add_argument("--pack-dir", type=Path, default=None)
    ap.add_argument("--marketing-root", type=Path, default=None)
    args = ap.parse_args()
    stage_latest_release(args.pack_dir, args.marketing_root)


if __name__ == "__main__":
    main()