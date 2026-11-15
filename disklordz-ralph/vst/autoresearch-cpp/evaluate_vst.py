#!/usr/bin/env python3
"""
Immutable C++ VST judge — do not let autoresearch agents edit this file.

Requires: Windows MSVC build, optional pluginval on PATH or DLZ_PLUGINVAL.
"""
from __future__ import annotations

import os
import re
import subprocess
import sys
import time
from pathlib import Path

import numpy as np
from scipy.io import wavfile
from scipy.signal import hilbert, stft

ROOT = Path(__file__).resolve().parent
STARTER = ROOT.parent / "starter-cpp"
BUILD_DIR = Path(os.environ.get("DLZ_BUILD_DIR", STARTER / "build"))
CONFIG = os.environ.get("DLZ_BUILD_CONFIG", "Release")
STRICTNESS = os.environ.get("PLUGINVAL_STRICTNESS", "5")

VST3 = BUILD_DIR / "DetroitPhonkTrapMaker_artefacts" / CONFIG / "VST3" / "Detroit Phonk Trap Maker.vst3"
def headless_candidates() -> list[Path]:
    base = BUILD_DIR / "autoresearch-cpp"
    names = [
        base / "HeadlessRender_artefacts" / CONFIG / "HeadlessRender",
        base / CONFIG / "HeadlessRender",
        base / CONFIG / "DiskLordzHeadlessRender",
        base / "DiskLordzHeadlessRender",
    ]
    if sys.platform == "win32":
        return [p.with_suffix(".exe") for p in names]
    return names


HEADLESS = next((p for p in headless_candidates() if p.is_file()), headless_candidates()[0])

TARGET = ROOT / "targets" / "kick.wav"
if not TARGET.is_file():
    TARGET = ROOT.parent.parent / "factory" / "autoresearch" / "targets" / "kick.wav"

OUT_DIR = ROOT / "outputs"
RENDERED = OUT_DIR / "rendered_kick.wav"
MAX_RENDER_MS = float(os.environ.get("DLZ_MAX_RENDER_MS", "15"))
ORIGINALITY_FLOOR = float(os.environ.get("MIN_SPECTRAL_CONVERGENCE", "0.08"))


def run_cmd(cmd: list[str], *, cwd: Path, timeout: float = 120.0) -> tuple[bool, str]:
    try:
        res = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        return False, "timeout"
    log = (res.stdout or "") + (res.stderr or "")
    return res.returncode == 0, log


def multi_res_stft_loss(y_true: np.ndarray, y_pred: np.ndarray, sr: int) -> float:
    fft_sizes = [256, 512, 1024, 2048]
    loss = 0.0
    used = 0
    for n_fft in fft_sizes:
        if len(y_true) < n_fft:
            continue
        _, _, z_t = stft(y_true, fs=sr, nperseg=n_fft, noverlap=n_fft // 4)
        _, _, z_p = stft(y_pred, fs=sr, nperseg=n_fft, noverlap=n_fft // 4)
        frames = min(z_t.shape[1], z_p.shape[1])
        mag_t = np.abs(z_t[:, :frames])
        mag_p = np.abs(z_p[:, :frames])
        sc = float(np.linalg.norm(mag_t - mag_p) / (np.linalg.norm(mag_t) + 1e-7))
        log_l1 = float(np.mean(np.abs(np.log(mag_t + 1e-7) - np.log(mag_p + 1e-7))))
        loss += sc + log_l1
        used += 1
    return loss / max(used, 1)


def find_pluginval() -> str | None:
    env = os.environ.get("DLZ_PLUGINVAL")
    if env and Path(env).is_file():
        return env
    repo = STARTER.parent / "tools" / "pluginval" / "pluginval.exe"
    if repo.is_file():
        return str(repo)
    from shutil import which

    return which("pluginval")


def evaluate() -> int:
    # 1) Build headless + VST3 (operator configures -DDLZ_AUTORESEARCH=ON once)
    ok, log = run_cmd(
        ["cmake", "--build", str(BUILD_DIR), "--config", CONFIG, "--target", "HeadlessRender", "-j"],
        cwd=STARTER,
    )
    if not ok:
        print("ERROR: C++ compilation failed")
        print(log[-3000:])
        print("TOTAL_LOSS: 9999.0")
        return 2

    ok, _ = run_cmd(
        ["cmake", "--build", str(BUILD_DIR), "--config", CONFIG, "--target", "DetroitPhonkTrapMaker_VST3", "-j"],
        cwd=STARTER,
    )
    if not ok:
        print("ERROR: VST3 build failed")
        print("TOTAL_LOSS: 9999.0")
        return 2

    plugval = find_pluginval()
    if plugval and VST3.exists():
        ok, log = run_cmd(
            [plugval, f"--strictness-level={STRICTNESS}", "--validate", str(VST3)],
            cwd=STARTER,
            timeout=90.0,
        )
        if not ok:
            print("ERROR: pluginval check failed")
            print(log[-2000:])
            print("TOTAL_LOSS: 9999.0")
            return 2
    elif not VST3.exists():
        print("WARN: VST3 bundle missing; skipping pluginval")

    if not HEADLESS.exists():
        print(f"ERROR: Headless binary not found: {HEADLESS}")
        print("TOTAL_LOSS: 9999.0")
        return 2

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    out_rel = os.path.relpath(RENDERED, ROOT)

    t0 = time.perf_counter()
    ok, log = run_cmd([str(HEADLESS), str(out_rel)], cwd=ROOT, timeout=30.0)
    render_ms = (time.perf_counter() - t0) * 1000.0
    if not ok or render_ms > MAX_RENDER_MS:
        print(f"ERROR: Render failed or CPU budget ({render_ms:.2f}ms > {MAX_RENDER_MS}ms)")
        print(log)
        print("TOTAL_LOSS: 9999.0")
        return 2

    if not TARGET.is_file():
        print(f"ERROR: Missing target {TARGET}")
        print("TOTAL_LOSS: 9999.0")
        return 2

    sr_t, target = wavfile.read(TARGET)
    sr_p, pred = wavfile.read(RENDERED)
    if target.dtype != np.float32:
        target = target.astype(np.float32) / (32768.0 if np.issubdtype(target.dtype, np.integer) else 1.0)
    if pred.dtype != np.float32:
        pred = pred.astype(np.float32) / (32768.0 if np.issubdtype(pred.dtype, np.integer) else 1.0)
    if target.ndim > 1:
        target = target.mean(axis=1)
    if pred.ndim > 1:
        pred = pred.mean(axis=1)

    n = min(len(target), len(pred))
    target, pred = target[:n], pred[:n]

    if np.isnan(pred).any() or float(np.max(np.abs(pred))) > 1.0:
        print("ERROR: Audio NaN or clipping")
        print("TOTAL_LOSS: 9999.0")
        return 2

    spec_loss = multi_res_stft_loss(target, pred, sr_t)
    env_loss = float(np.mean(np.abs(np.abs(hilbert(target)) - np.abs(hilbert(pred)))))
    originality_penalty = max(0.0, (ORIGINALITY_FLOOR - spec_loss) * 50.0)
    total_loss = spec_loss + (5.0 * env_loss) + originality_penalty

    prev_best = OUT_DIR / ".best_loss"
    if total_loss < float(prev_best.read_text().strip()) if prev_best.is_file() else float("inf"):
        prev_best.write_text(f"{total_loss:.6f}")

    print(f"CPU_RENDER_MS: {render_ms:.2f}ms")
    print(f"SPECTRAL_LOSS: {spec_loss:.6f}")
    print(f"ENVELOPE_LOSS: {env_loss:.6f}")
    print(f"ORIGINALITY_PENALTY: {originality_penalty:.6f}")
    print(f"TOTAL_LOSS: {total_loss:.6f}")
    return 0


if __name__ == "__main__":
    sys.exit(evaluate())