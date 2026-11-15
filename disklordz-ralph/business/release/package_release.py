#!/usr/bin/env python3
"""
Bundle commercial release artifacts after pluginval, evaluate_vst, and kit export.

Run from repo root:
  python disklordz-ralph/business/release/package_release.py
  python disklordz-ralph/business/release/package_release.py --version 1.0.0 --include-drumsynth-vst
"""

from __future__ import annotations

import argparse
import glob
import hashlib
import os
import shutil
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
FACTORY = ROOT / "disklordz-ralph" / "factory" / "autoresearch"
STARTER_BUILD = Path(os.environ.get("DLZ_BUILD_DIR", ROOT / "disklordz-ralph" / "vst" / "starter-cpp" / "build"))

ROYALTY_TEXT = """DiskLordz — Royalty-Free Sample License (synthesized one-shots)

You may use these WAV files in commercial music production, beat sales, streaming, and sync
projects without additional royalties to DiskLordz.

You may NOT redistribute, resell, or repackage individual samples as a competing sample library,
preset bank, or “construction kit” resell product.

All sounds are original synthesis — not third-party recordings.
"""

LICENSE_EULA = """END USER LICENSE AGREEMENT — DiskLordz VST3 (summary)

1. One licensee per purchased serial / machine activation unless Team tier.
2. No reverse engineering except where law permits.
3. Provided "AS IS" — see full EULA in storefront checkout.
4. Sample packs bundled with Core tier follow the Royalty-Free Sample License above.
"""


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def latest_pack_dir() -> Path | None:
    packs_root = FACTORY / "outputs" / "packs"
    if not packs_root.is_dir():
        return None
    candidates = sorted(packs_root.iterdir(), key=lambda p: p.stat().st_mtime)
    if not candidates:
        return None
    return candidates[-1]


def find_vst3_bundle(name_contains: str) -> Path | None:
    patterns = [
        STARTER_BUILD / "**" / "VST3" / f"{name_contains}.vst3",
        STARTER_BUILD / "**" / f"{name_contains}.vst3",
    ]
    for pattern in patterns:
        for path in glob.glob(str(pattern), recursive=True):
            bundle = Path(path)
            if bundle.is_dir():
                return bundle
    return None


def zip_vst3_bundle(bundle: Path, dist_dir: Path, zip_stem: str) -> Path | None:
    if not bundle.is_dir():
        return None
    zip_base = dist_dir / zip_stem
    # Archive parent so the .vst3 folder is top-level inside the zip
    parent = bundle.parent
    arc = shutil.make_archive(str(zip_base), "zip", root_dir=parent, base_dir=bundle.name)
    return Path(arc)


def package_kit(pack_dir: Path, dist_dir: Path, version: str, brand: str) -> Path | None:
    (pack_dir / "ROYALTY_FREE_LICENSE.txt").write_text(ROYALTY_TEXT, encoding="utf-8")
    (pack_dir / "LICENSE_EULA.txt").write_text(ROYALTY_TEXT + "\n" + LICENSE_EULA, encoding="utf-8")

    safe_name = pack_dir.name.replace(" ", "_")
    zip_base = dist_dir / f"{brand}_DrumKit_v{version}_{safe_name}"
    arc = shutil.make_archive(str(zip_base), "zip", root_dir=pack_dir)
    return Path(arc)


def build_release_bundle(
    version: str,
    *,
    brand: str,
    include_ship_vst: bool,
    include_drumsynth_vst: bool,
    manual_path: Path | None,
) -> Path:
    stamp = datetime.now(timezone.utc).strftime("%Y%m%d")
    dist_dir = ROOT / "disklordz-ralph" / "business" / "release" / "dist" / f"Release_v{version}_{stamp}"
    dist_dir.mkdir(parents=True, exist_ok=True)
    checksums: list[str] = []

    pack = latest_pack_dir()
    if pack is not None:
        kit_zip = package_kit(pack, dist_dir, version, brand)
        if kit_zip and kit_zip.is_file():
            checksums.append(f"{sha256_file(kit_zip)}  {kit_zip.name}")
            print(f"Kit ZIP: {kit_zip}")
    else:
        print("No kit under factory/autoresearch/outputs/packs — run export_kit.py first")

    if include_ship_vst:
        ship = find_vst3_bundle("Detroit Phonk Trap Maker")
        if ship:
            z = zip_vst3_bundle(ship, dist_dir, f"{brand}_DetroitPhonkTrapMaker_VST3_Win64_v{version}")
            if z:
                checksums.append(f"{sha256_file(z)}  {z.name}")
                print(f"Ship VST3 ZIP: {z}")
        else:
            print("Ship VST3 not found — build DetroitPhonkTrapMaker_VST3")

    if include_drumsynth_vst:
        demo = find_vst3_bundle("DrumSynth Autoresearch")
        if demo:
            z = zip_vst3_bundle(demo, dist_dir, f"{brand}_DrumSynthAutoresearch_VST3_Win64_v{version}")
            if z:
                checksums.append(f"{sha256_file(z)}  {z.name}")
                print(f"DrumSynth VST3 ZIP: {z}")
        else:
            print("DrumSynth VST3 not found — build DrumSynthAutoresearch_VST3")

    if manual_path and manual_path.is_file():
        dest = dist_dir / manual_path.name
        shutil.copy2(manual_path, dest)
        checksums.append(f"{sha256_file(dest)}  {dest.name}")

    if checksums:
        sums_path = dist_dir / "SHA256SUMS.txt"
        sums_path.write_text("\n".join(checksums) + "\n", encoding="utf-8")
        print(f"SHA256SUMS: {sums_path}")

    readme = dist_dir / "RELEASE_README.txt"
    readme.write_text(
        f"DiskLordz release bundle v{version}\n"
        f"Built UTC: {datetime.now(timezone.utc).isoformat()}\n\n"
        "Upload zips to private R2/S3; serve via presigned URLs from AudioStorePortal.\n"
        "Sign Windows installers with signtool before public download.\n",
        encoding="utf-8",
    )

    print(f"Commercial release artifacts: {dist_dir}")
    return dist_dir


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--version", default=os.environ.get("DLZ_RELEASE_VERSION", "1.0.0"))
    parser.add_argument("--brand", default=os.environ.get("DLZ_BRAND_PREFIX", "DiskLordz"))
    parser.add_argument("--include-ship-vst", action="store_true")
    parser.add_argument("--no-ship-vst", action="store_true")
    parser.add_argument("--include-drumsynth-vst", action="store_true")
    parser.add_argument("--manual", type=Path, help="Optional PDF or MD user manual to copy")
    args = parser.parse_args()

    include_ship = (args.include_ship_vst or not args.include_drumsynth_vst) and not args.no_ship_vst
    build_release_bundle(
        args.version,
        brand=args.brand,
        include_ship_vst=include_ship,
        include_drumsynth_vst=args.include_drumsynth_vst,
        manual_path=args.manual,
    )


if __name__ == "__main__":
    main()