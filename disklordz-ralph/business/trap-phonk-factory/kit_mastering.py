#!/usr/bin/env python3
"""
Bot 2: soft-clip mastering + librosa root-note tagging + ZIP lead magnet.
"""

from __future__ import annotations

import argparse
import json
import re
import zipfile
from pathlib import Path

import numpy as np
import soundfile as sf

try:
    import librosa
except ImportError as exc:
    raise SystemExit("pip install librosa") from exc

try:
    from pedalboard import Pedalboard, HighpassFilter, Compressor
    from pedalboard.io import AudioFile
except ImportError as exc:
    raise SystemExit("pip install pedalboard") from exc


def tanh_soft_clip(samples: np.ndarray, gain_db: float = 4.0) -> np.ndarray:
    g = 10.0 ** (gain_db / 20.0)
    return np.tanh(samples * g).astype(np.float32)


def estimate_root_note(samples: np.ndarray, sr: int) -> str:
    if len(samples) < 512:
        return "C"
    f0 = librosa.yin(samples, fmin=40, fmax=400, sr=sr)
    f0 = f0[np.isfinite(f0) & (f0 > 0)]
    if len(f0) == 0:
        return "C"
    midi = int(np.round(librosa.hz_to_midi(np.median(f0))))
    return librosa.midi_to_note(midi, unicode=False)


def master_one_shot(path: Path, out_dir: Path, style: str, bpm: int) -> dict:
    data, sr = sf.read(path, always_2d=True)
    mono = np.mean(data, axis=1)

    board = Pedalboard([HighpassFilter(cutoff_frequency_hz=130.0), Compressor(ratio=2.0)])
    stereo = data.astype(np.float32)
    processed = board(stereo, sr)

    role = "808" if np.median(np.abs(mono)) > 0.3 and len(mono) / sr > 0.25 else "Loop"
    if role == "808":
        processed = tanh_soft_clip(processed, gain_db=4.0)

    peak = np.max(np.abs(processed)) + 1e-9
    processed = (processed * (0.89125 / peak)).astype(np.float32)

    root = estimate_root_note(mono, sr)
    safe_root = re.sub(r"[^A-G#0-9]", "", root.replace("b", "b"))
    out_name = f"{role}_{style}_{bpm}BPM_{safe_root}.wav"
    out_path = out_dir / out_name
    sf.write(out_path, processed, sr, subtype="PCM_24")

    return {"file": out_name, "role": role, "root": root, "source": path.name}


def package_lead_magnet(mastered_dir: Path, zip_path: Path, max_files: int = 15) -> int:
    wavs = sorted(mastered_dir.glob("*.wav"))[:max_files]
    with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_DEFLATED) as zf:
        for w in wavs:
            zf.write(w, w.name)
    return len(wavs)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--in-dir", type=Path, required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--style", default="Memphis")
    parser.add_argument("--bpm", type=int, default=145)
    parser.add_argument("--zip", type=Path, default=None)
    args = parser.parse_args()

    args.out_dir.mkdir(parents=True, exist_ok=True)
    manifest = []
    for wav in sorted(args.in_dir.glob("*.wav")):
        manifest.append(master_one_shot(wav, args.out_dir, args.style, args.bpm))

    (args.out_dir / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")

    if args.zip:
        n = package_lead_magnet(args.out_dir, args.zip)
        print(f"Lead magnet: {n} files -> {args.zip}")
    print(json.dumps({"mastered": len(manifest)}, indent=2))


if __name__ == "__main__":
    main()