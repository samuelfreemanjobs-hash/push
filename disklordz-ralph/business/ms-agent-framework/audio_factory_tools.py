"""Shared Python tools for the unified Audio Factory swarm (DSP, WAV pack, presets)."""

from __future__ import annotations

import json
import os
import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path

import numpy as np
import soundfile as sf
from scipy.fft import rfft, rfftfreq

FACTORY_ROOT: Path = Path(".").resolve()

DSP_HEADER_REL = "dsp_harness/SaturationDSP.h"
ALLOWED_DSP_HEADER = frozenset({DSP_HEADER_REL})


def _resolve_under_factory(relative_path: str) -> Path | None:
    rel = relative_path.replace("\\", "/").lstrip("/")
    target = (FACTORY_ROOT / rel).resolve()
    try:
        target.relative_to(FACTORY_ROOT.resolve())
    except ValueError:
        return None
    return target


def _find_harness_exe() -> Path | None:
    candidates = [
        FACTORY_ROOT / "build" / "Release" / "dsp_harness.exe",
        FACTORY_ROOT / "build" / "dsp_harness" / "Release" / "dsp_harness.exe",
        FACTORY_ROOT / "build" / "dsp_harness",
    ]
    for c in candidates:
        if c.is_file():
            return c
    return None


def _configure_cmake_if_needed() -> str | None:
    build = FACTORY_ROOT / "build"
    if build.is_dir() and any(build.glob("**/dsp_harness*")):
        return None
    if not (FACTORY_ROOT / "CMakeLists.txt").is_file():
        return "Error: CMakeLists.txt missing in audio-factory."
    if os.name != "nt":
        return "Error: dsp_harness build requires Windows MSVC."

    cfg = subprocess.run(
        ["cmake", "-S", ".", "-B", "build", "-G", "Visual Studio 17 2022", "-A", "x64"],
        cwd=FACTORY_ROOT,
        capture_output=True,
        text=True,
        timeout=300,
    )
    if cfg.returncode != 0:
        return f"CMAKE CONFIGURE FAILED:\n{cfg.stderr[-2000:]}\n{cfg.stdout[-2000:]}"
    return None


# --- Module 1: DSP ---


def update_and_compile_dsp(cpp_header_path: str, new_code: str) -> str:
    """Write SaturationDSP.h and build dsp_harness (Windows MSVC)."""
    if cpp_header_path.replace("\\", "/") not in ALLOWED_DSP_HEADER:
        return f"Error: Only {DSP_HEADER_REL} may be edited."
    if len(new_code) > 120_000:
        return "Error: Header too large."

    target = _resolve_under_factory(cpp_header_path)
    if target is None:
        return "Error: Path blocked."

    target.write_text(new_code, encoding="utf-8")

    err = _configure_cmake_if_needed()
    if err:
        return err

    bld = subprocess.run(
        ["cmake", "--build", "build", "--config", "Release", "--target", "dsp_harness"],
        cwd=FACTORY_ROOT,
        capture_output=True,
        text=True,
        timeout=600,
    )
    if bld.returncode != 0:
        return f"MSVC COMPILE ERROR:\n{bld.stderr[-2500:]}\n{bld.stdout[-2500:]}"
    return "DSP_COMPILE_OK"


def benchmark_dsp_audio(input_wav: str, output_wav: str, drive: float) -> str:
    """Run dsp_harness on a loop WAV and return crest factor + spectral centroid JSON."""
    harness = _find_harness_exe()
    in_path = _resolve_under_factory(input_wav)
    out_path = (FACTORY_ROOT / output_wav).resolve()
    if harness is None:
        return "Error: dsp_harness not built. Call update_and_compile_dsp or cmake --build."
    if in_path is None or not in_path.is_file():
        return f"Error: Input not found: {input_wav}"

    out_path.parent.mkdir(parents=True, exist_ok=True)
    try:
        res = subprocess.run(
            [str(harness), str(in_path), str(out_path), str(drive)],
            capture_output=True,
            text=True,
            timeout=120,
        )
        if res.returncode != 0:
            return f"DSP_RUNTIME_ERROR: {res.stderr[-1500:]}\n{res.stdout[-500:]}"
    except OSError as exc:
        return f"DSP_RUNTIME_ERROR: {exc}"

    if not out_path.is_file():
        return "Error: Harness did not write output WAV."

    audio, sr = sf.read(out_path, always_2d=True)
    mono = np.mean(audio, axis=1)
    peak = float(np.max(np.abs(mono)) + 1e-9)
    rms = float(np.sqrt(np.mean(mono**2)) + 1e-9)
    crest_db = float(20.0 * np.log10(peak / rms))

    spectrum = np.abs(rfft(mono))
    freqs = rfftfreq(len(mono), 1.0 / sr)
    centroid_hz = float(np.sum(freqs * spectrum) / (np.sum(spectrum) + 1e-9))

    return json.dumps(
        {
            "peak_dbfs": round(20.0 * np.log10(peak), 2),
            "crest_factor_db": round(crest_db, 2),
            "spectral_centroid_hz": round(centroid_hz, 1),
            "drive": drive,
            "status": "DSP_BENCHMARK_COMPLETE",
        }
    )


# --- Module 2: Sample pack ---


def process_and_master_sample_pack(pack_prefix: str = "DKP") -> str:
    """Trim silence, peak-normalize to -1 dBFS, role-tag, export 24-bit WAVs + manifest."""
    raw_dir = FACTORY_ROOT / "raw_audio"
    out_dir = FACTORY_ROOT / "dist_sample_pack"
    out_dir.mkdir(parents=True, exist_ok=True)

    manifest: list[dict] = []
    wav_files = sorted(raw_dir.glob("*.wav"))
    if not wav_files:
        return "No .wav files in raw_audio/ (drop masters or copy from factory/autoresearch/outputs)."

    for idx, wav in enumerate(wav_files, start=1):
        data, sr = sf.read(wav, always_2d=True)
        mono = np.mean(data, axis=1)
        threshold = 10 ** (-60.0 / 20.0)
        non_silent = np.where(np.abs(mono) > threshold)[0]
        if len(non_silent) == 0:
            continue
        trimmed = data[non_silent[0] : non_silent[-1] + 1]

        max_peak = np.max(np.abs(trimmed))
        if max_peak > 0:
            trimmed = trimmed * (0.89125 / max_peak)

        spec = np.abs(rfft(np.mean(trimmed, axis=1)))
        freqs = rfftfreq(len(trimmed), 1.0 / sr)
        centroid = np.sum(freqs * spec) / (np.sum(spec) + 1e-9)
        duration_sec = len(trimmed) / sr

        if centroid < 350 and duration_sec > 0.45:
            role = "808"
        elif centroid < 600:
            role = "Kick"
        elif centroid < 3200:
            role = "Snare"
        else:
            role = "Hat"

        clean_name = f"{pack_prefix}_{role}_{idx:02d}.wav"
        out_role = out_dir / role
        out_role.mkdir(exist_ok=True)
        sf.write(out_role / clean_name, trimmed, sr, subtype="PCM_24")
        manifest.append(
            {
                "file": f"{role}/{clean_name}",
                "role": role,
                "duration_s": round(duration_sec, 3),
                "source": wav.name,
            }
        )

    (out_dir / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    readme = (
        "# Sample pack (auto-factory)\n\n"
        f"- Files: {len(manifest)}\n"
        "- Peak: -1.0 dBFS\n"
        "- Format: 24-bit PCM\n\n"
        "See manifest.json for role tags.\n"
    )
    (out_dir / "README.md").write_text(readme, encoding="utf-8")
    return json.dumps({"exported_count": len(manifest), "samples": manifest})


# --- Module 3: Presets + velocity map ---


def write_and_validate_vst_preset(preset_name: str, xml_content: str) -> str:
    """Validate APVTS-style <Parameters> XML and save under dist_presets/."""
    preset_dir = FACTORY_ROOT / "dist_presets"
    preset_dir.mkdir(parents=True, exist_ok=True)
    safe = "".join(c if c.isalnum() or c in "._-" else "_" for c in preset_name)
    try:
        root_elem = ET.fromstring(xml_content.strip())
    except ET.ParseError as exc:
        return f"XML_PARSE_ERROR: {exc}"

    if root_elem.tag != "Parameters":
        return "XML_VALIDATION_ERROR: Root tag must be <Parameters> (APVTS state)."

    out_file = preset_dir / f"{safe}.xml"
    out_file.write_text(xml_content.strip(), encoding="utf-8")
    return f"PRESET_SAVED: {out_file.name}"


def generate_velocity_layer_map() -> str:
    """Build MIDI velocity-layer JSON from dist_sample_pack/manifest.json."""
    manifest_path = FACTORY_ROOT / "dist_sample_pack" / "manifest.json"
    if not manifest_path.is_file():
        return "Error: Run process_and_master_sample_pack first."

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    role_midi = {"808": 36, "Kick": 36, "Snare": 38, "Hat": 42}
    layers: dict[str, list] = {r: [] for r in role_midi}

    for entry in manifest:
        role = entry.get("role", "Kick")
        if role not in layers:
            layers[role] = []
        layers[role].append(
            {
                "path": entry["file"],
                "midi_note": role_midi.get(role, 36),
                "velocity_lo": 1,
                "velocity_hi": 127,
                "round_robin_group": role,
            }
        )

    out_dir = FACTORY_ROOT / "dist_presets"
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / "velocity_layers.json"
    out_path.write_text(json.dumps(layers, indent=2), encoding="utf-8")
    return f"VELOCITY_MAP_SAVED: {len(manifest)} samples → {out_path.name}"


def read_factory_manifest() -> str:
    """Return dist_sample_pack/manifest.json for preset agents."""
    path = FACTORY_ROOT / "dist_sample_pack" / "manifest.json"
    if not path.is_file():
        return "Error: manifest.json missing."
    return path.read_text(encoding="utf-8")