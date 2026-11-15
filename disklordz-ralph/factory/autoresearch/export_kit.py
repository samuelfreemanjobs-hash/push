#!/usr/bin/env python3
"""
Batch commercial kit export from drum_engine.py (autoresearch sandbox).

Usage:
  python export_kit.py
  python export_kit.py --sfz
  PACK_NAME=Louisiana_Bounce_V1 python export_kit.py
"""

from __future__ import annotations

import argparse
import json
import os
import wave
from datetime import datetime, timezone
from pathlib import Path

import numpy as np

import drum_engine

PACK_NAME = os.environ.get("PACK_NAME", "DiskLordz_Autoresearch_Vol1")
SAMPLE_RATE = int(os.environ.get("KIT_SAMPLE_RATE", "48000"))
MAX_DURATION_SEC = 1.5
TARGET_PEAK_DBFS = -0.3
FILE_PREFIX = "DLZ"

NOTE_SEMITONES_FROM_C1 = {
    "C1": 0,
    "C#1": 1,
    "D1": 2,
    "D#1": 3,
    "E1": 4,
    "F1": 5,
    "F#1": 6,
    "G1": 7,
}

KIT_MATRIX = {
    "Kicks": {
        "fn": "synthesize_kick",
        "midi_note": 36,
        "pitches": ["C1", "D1", "E1", "F1"],
        "velocities": {"Soft": 0.45, "Med": 0.78, "Hard": 1.00},
        "duration": 1.2,
    },
    "Snares": {
        "fn": "synthesize_snare",
        "midi_note": 38,
        "pitches": ["D1", "E1"],
        "velocities": {"Ghost": 0.35, "Rim": 0.75, "Crack": 1.00},
        "duration": 0.65,
    },
    "Hats": {
        "fn": "synthesize_hat",
        "midi_note": 42,
        "pitches": ["F#1"],
        "velocities": {"Tight": 0.50, "Crisp": 0.80, "Open": 1.00},
        "duration": 0.35,
    },
}


def pitch_shift_resample(audio: np.ndarray, semitones: int) -> np.ndarray:
    if semitones == 0:
        return audio
    ratio = 2.0 ** (semitones / 12.0)
    new_len = max(1, int(len(audio) / ratio))
    x_old = np.linspace(0.0, 1.0, len(audio))
    x_new = np.linspace(0.0, 1.0, new_len)
    return np.interp(x_new, x_old, audio).astype(np.float32)


def trim_and_fade(audio: np.ndarray, sr: int, threshold_db: float = -72.0) -> np.ndarray:
    thresh_amp = 10.0 ** (threshold_db / 20.0)
    above = np.where(np.abs(audio) > thresh_amp)[0]
    if len(above) == 0:
        return audio[: int(0.05 * sr)]

    end_idx = min(len(audio), int(above[-1]) + int(0.015 * sr))
    trimmed = audio[:end_idx].copy()

    fade_len = min(int(0.005 * sr), len(trimmed))
    if fade_len > 0:
        trimmed[-fade_len:] *= np.linspace(1.0, 0.0, fade_len, dtype=np.float32)
    return trimmed


def normalize_peak(audio: np.ndarray, target_dbfs: float, velocity_scale: float) -> np.ndarray:
    peak = float(np.max(np.abs(audio)))
    if peak == 0.0:
        return audio
    target_linear = (10.0 ** (target_dbfs / 20.0)) * velocity_scale
    return (audio / peak) * target_linear


def write_wav_24bit(filepath: Path, audio: np.ndarray, sr: int) -> None:
    clipped = np.clip(audio, -1.0, 1.0)
    frames = bytearray()
    for s in clipped:
        v = int(np.clip(s * 8388607.0, -8388608, 8388607))
        frames.extend(int(v).to_bytes(3, byteorder="little", signed=True))

    filepath.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(filepath), "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(3)
        wf.setframerate(sr)
        wf.writeframes(bytes(frames))


def write_sfz(out_root: Path, manifest: dict) -> None:
    lines = [
        "// DiskLordz autoresearch kit — SFZ 1.0",
        f"// Pack: {manifest.get('pack_name')}",
        "",
    ]
    for entry in manifest.get("samples", []):
        rel = entry["file"].replace("\\", "/")
        midi = entry["midi_note"]
        lines.append("<group>")
        lines.append(f"sample={rel}")
        lines.append(f"pitch_keycenter={midi}")
        lines.append(f"lokey={midi}")
        lines.append(f"hikey={midi}")
        lines.append("")
    (out_root / "DiskLordz_Kit.sfz").write_text("\n".join(lines), encoding="utf-8")


def export_commercial_pack(emit_sfz: bool = False) -> Path:
    stamp = datetime.now(timezone.utc).strftime("%Y%m%d_%H%M")
    root = Path(__file__).resolve().parent
    out_root = root / "outputs" / "packs" / f"{PACK_NAME}_{stamp}"
    out_root.mkdir(parents=True, exist_ok=True)

    manifest = {
        "kit_id": PACK_NAME.lower().replace(" ", "-"),
        "pack_name": PACK_NAME,
        "title": PACK_NAME.replace("_", " "),
        "created_at": datetime.now(timezone.utc).isoformat().replace("+00:00", "Z"),
        "format": f"24-bit / {SAMPLE_RATE // 1000}kHz mono WAV",
        "wav_bit_depth": 24,
        "sample_rate": SAMPLE_RATE,
        "generator": "autoresearch/export_kit.py",
        "genre_tags": ["southern_trap", "bounce", "synthesis_only"],
        "samples": [],
    }

    total_files = 0
    for category, spec in KIT_MATRIX.items():
        cat_dir = out_root / category
        cat_dir.mkdir(parents=True, exist_ok=True)
        synth_fn = getattr(drum_engine, spec["fn"])
        num_samples = min(int(spec["duration"] * SAMPLE_RATE), int(MAX_DURATION_SEC * SAMPLE_RATE))

        for pitch_label in spec["pitches"]:
            semitones = NOTE_SEMITONES_FROM_C1.get(pitch_label, 0)
            for vel_label, vel_val in spec["velocities"].items():
                raw = synth_fn(num_samples, SAMPLE_RATE, velocity=vel_val)
                pitched = pitch_shift_resample(raw, semitones)
                cleaned = trim_and_fade(pitched, SAMPLE_RATE)
                mastered = normalize_peak(cleaned, TARGET_PEAK_DBFS, velocity_scale=0.65 + 0.35 * vel_val)

                stem = category[:-1]
                filename = (
                    f"{FILE_PREFIX}_{stem}_{pitch_label}_{vel_label}_v{int(vel_val * 127)}.wav"
                )
                filepath = cat_dir / filename
                write_wav_24bit(filepath, mastered, SAMPLE_RATE)

                manifest["samples"].append(
                    {
                        "file": f"{category}/{filename}",
                        "category": category,
                        "pitch": pitch_label,
                        "pitch_semitones_c1": semitones,
                        "velocity_layer": vel_label,
                        "velocity": round(vel_val, 3),
                        "midi_note": spec["midi_note"],
                        "duration_ms": round(len(mastered) / SAMPLE_RATE * 1000, 1),
                    }
                )
                total_files += 1

    manifest["sample_count"] = total_files
    manifest_path = out_root / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")

    if emit_sfz:
        write_sfz(out_root, manifest)

    print(f"Exported {total_files} commercial 24-bit WAV samples to: {out_root}")
    return out_root


def _publish_openmontage(out_root: Path) -> None:
    from publish_to_openmontage import stage_latest_release

    try:
        stage_latest_release(pack_dir=out_root)
    except (FileNotFoundError, OSError) as exc:
        print(f"OpenMontage publish skipped: {exc}")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--sfz", action="store_true", help="Write DiskLordz_Kit.sfz for SFZ/Ableton import")
    ap.add_argument(
        "--no-publish",
        action="store_true",
        help="Skip publish_to_openmontage (default: publish unless SKIP_OPENMONTAGE_PUBLISH=1)",
    )
    args = ap.parse_args()
    out_root = export_commercial_pack(emit_sfz=args.sfz)
    skip_env = os.environ.get("SKIP_OPENMONTAGE_PUBLISH", "").lower() in ("1", "true", "yes")
    if not args.no_publish and not skip_env:
        _publish_openmontage(out_root)


if __name__ == "__main__":
    main()