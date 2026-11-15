#!/usr/bin/env python3
"""Stage latest autoresearch pack (+ best_kit.wav) for OpenMontage sibling repo."""

from __future__ import annotations

import json
import os
import shutil
from datetime import datetime, timezone
from pathlib import Path

AUTORESEARCH = Path(__file__).resolve().parent
REPO_ROOT = AUTORESEARCH.parents[2]  # repo root (push)
PACKS = AUTORESEARCH / "outputs" / "packs"
BEST_KIT = AUTORESEARCH / "outputs" / "best_kit.wav"

HERO_VELOCITY_SUFFIX = {
    "Kicks": "Hard",
    "Snares": "Crack",
    "Hats": "Open",
}


def _latest_pack_dir() -> Path:
    if not PACKS.is_dir():
        raise FileNotFoundError(f"No packs directory: {PACKS}")
    candidates = [p for p in PACKS.iterdir() if p.is_dir()]
    if not candidates:
        raise FileNotFoundError("No exported pack folders under outputs/packs/")
    return max(candidates, key=lambda p: p.stat().st_mtime)


def _marketing_root(explicit: Path | None) -> Path:
    if explicit is not None:
        return explicit.resolve()
    env = os.environ.get("OPENMONTAGE_ROOT", "").strip()
    if env:
        return Path(env).expanduser().resolve()
    return (REPO_ROOT / ".." / "marketing-studio").resolve()


def _pick_hero_wavs(pack_dir: Path, manifest: dict) -> list[Path]:
    heroes: list[Path] = []
    for category, vel in HERO_VELOCITY_SUFFIX.items():
        cat_path = pack_dir / category
        if not cat_path.is_dir():
            continue
        matches = sorted(cat_path.glob(f"*_{vel}_v*.wav"))
        if matches:
            heroes.append(matches[0])
            continue
        for entry in manifest.get("samples", []):
            if entry.get("category") == category and entry.get("velocity_layer") == vel:
                p = pack_dir / entry["file"].split("/", 1)[-1]
                if p.is_file():
                    heroes.append(p)
                break
    return heroes


def stage_handoff(pack_dir: Path | None = None, marketing_root: Path | None = None) -> Path:
    pack = (pack_dir or _latest_pack_dir()).resolve()
    root = _marketing_root(marketing_root)
    dest = root / "assets" / "disklordz" / pack.name
    dest.mkdir(parents=True, exist_ok=True)

    manifest_path = pack / "manifest.json"
    if not manifest_path.is_file():
        raise FileNotFoundError(f"Missing manifest: {manifest_path}")

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    shutil.copy2(manifest_path, dest / "manifest.json")

    sfz = pack / "DiskLordz_Kit.sfz"
    if sfz.is_file():
        shutil.copy2(sfz, dest / "DiskLordz_Kit.sfz")

    clips_dir = dest / "clips"
    clips_dir.mkdir(exist_ok=True)
    for wav in _pick_hero_wavs(pack, manifest):
        shutil.copy2(wav, clips_dir / wav.name)

    if BEST_KIT.is_file():
        shutil.copy2(BEST_KIT, dest / "best_kit.wav")
        audio_relpath = "assets/disklordz/" + pack.name + "/best_kit.wav"
    elif clips_dir.glob("*.wav"):
        first = next(iter(sorted(clips_dir.glob("*.wav"))))
        audio_relpath = f"assets/disklordz/{pack.name}/clips/{first.name}"
    else:
        audio_relpath = "(add best_kit.wav or clips/*.wav)"

    title = manifest.get("title") or manifest.get("pack_name", pack.name)
    brief = {
        "brand": "DiskLordz",
        "product_type": "drum_kit",
        "title": title,
        "length_seconds": 30,
        "aspect_ratios": ["9:16", "16:9"],
        "tone": "confident, hardware booth, southern trap",
        "key_messages": [
            "24-bit / 48 kHz synthesis-only drums",
            "Royalty-free for beats and releases",
            "No copyrighted samples",
        ],
        "audio_soundtrack_relpath": audio_relpath,
        "manifest_sample_count": manifest.get("sample_count"),
        "staged_at": datetime.now(timezone.utc).isoformat().replace("+00:00", "Z"),
        "source_pack_dir": str(pack),
    }
    (dest / "brief.json").write_text(json.dumps(brief, indent=2), encoding="utf-8")

    prompt = f"""# OpenMontage agent prompt (auto-staged)

Pack folder: `{pack.name}`
Read `brief.json` and `manifest.json` in this directory.

## Primary task
Create a **30-second 9:16 vertical** promo for **{title}**.

Soundtrack: `{audio_relpath}` (in marketing-studio repo root)

Follow DiskLordz brand: gunmetal `#121216`, amber accents, no purple AI gradients.

**Pause for human approval** after storyboard and after 720p draft.

Full template library: `{REPO_ROOT / "docs/marketing/openmontage-prompt-templates.md"}`

Setup: `{REPO_ROOT / "docs/dev/openmontage.md"}`
"""
    (dest / "AGENT_PROMPT.md").write_text(prompt, encoding="utf-8")

    staging_meta = {
        "pack_name": pack.name,
        "dest": str(dest),
        "marketing_root": str(root),
        "clips": [p.name for p in clips_dir.glob("*.wav")],
    }
    (dest / "staging_meta.json").write_text(json.dumps(staging_meta, indent=2), encoding="utf-8")

    print(f"Staged OpenMontage handoff: {dest}")
    print(f"Open marketing-studio and read: assets/disklordz/{pack.name}/AGENT_PROMPT.md")
    return dest


def main() -> None:
    import argparse

    ap = argparse.ArgumentParser()
    ap.add_argument("--pack-dir", type=Path, default=None, help="Specific outputs/packs/<folder>")
    ap.add_argument(
        "--marketing-root",
        type=Path,
        default=None,
        help="OpenMontage clone path (default: OPENMONTAGE_ROOT or ../marketing-studio)",
    )
    args = ap.parse_args()
    stage_handoff(args.pack_dir, args.marketing_root)


if __name__ == "__main__":
    main()