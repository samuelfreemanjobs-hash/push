"""
IMMUTABLE JUDGE — autoresearch agents must NOT edit this file.

Run via run_harness.py only.
"""

from __future__ import annotations

import json
import sys
import time
from pathlib import Path

import numpy as np
from scipy import signal
from scipy.io import wavfile

import drum_engine

ROOT = Path(__file__).resolve().parent
TARGET_WAV = ROOT / "targets" / "reference_808.wav"

# Band-energy profile when no reference WAV (BR sub-heavy 808)
PROFILE_808 = (
    (30, 80, 0.42),
    (80, 200, 0.28),
    (200, 800, 0.12),
    (800, 4000, 0.10),
    (4000, 12000, 0.08),
)


def _stft_mag(y: np.ndarray, sr: float, n_fft: int) -> np.ndarray:
    _, _, z = signal.stft(y, fs=sr, nperseg=n_fft, noverlap=n_fft // 2)
    return np.abs(z)


def stft_loss(y: np.ndarray, ref: np.ndarray, sr: float) -> float:
    loss = 0.0
    for n_fft in (512, 2048, 4096):
        my = _stft_mag(y, sr, n_fft)
        mr = _stft_mag(ref, sr, n_fft)
        frames = min(my.shape[1], mr.shape[1])
        my, mr = my[:, :frames], mr[:, :frames]
        loss += float(np.mean(np.abs(my - mr)))
    return loss / 3.0


def profile_loss(y: np.ndarray, sr: float) -> float:
    f, pxx = signal.welch(y, fs=sr, nperseg=4096)
    total = float(np.sum(pxx)) + 1e-18
    loss = 0.0
    for lo, hi, target_frac in PROFILE_808:
        mask = (f >= lo) & (f < hi)
        frac = float(np.sum(pxx[mask])) / total
        loss += abs(frac - target_frac)
    return loss


def crest_factor(y: np.ndarray) -> float:
    rms = float(np.sqrt(np.mean(y * y)) + 1e-18)
    return float(np.max(np.abs(y)) / rms)


def attack_slope_ms(y: np.ndarray, sr: float) -> float:
    win = int(0.02 * sr)
    if win < 8:
        return 0.0
    seg = np.abs(y[:win])
    peak_i = int(np.argmax(seg))
    if peak_i < 2:
        return 0.0
    return float(peak_i / sr * 1000.0)


def thd_ratio(y: np.ndarray, sr: float, fund_hz: float = 49.0) -> float:
    n = min(len(y), 65536)
    y = y[:n] - np.mean(y[:n])
    spec = np.abs(np.fft.rfft(y))
    freqs = np.fft.rfftfreq(n, 1.0 / sr)
    bin_w = freqs[1] - freqs[0] if len(freqs) > 1 else 1.0
    fund_i = int(np.clip(np.round(fund_hz / bin_w), 1, len(spec) - 2))
    fund = float(spec[fund_i] ** 2)
    harm = 0.0
    for h in range(2, 8):
        i = int(np.clip(fund_i * h, 0, len(spec) - 1))
        harm += float(spec[i] ** 2)
    if fund < 1e-18:
        return 1.0
    return harm / fund


def dc_offset(y: np.ndarray, sr: float) -> float:
    """Measure DC on tail (after attack) so click transients do not dominate."""
    start = int(0.08 * sr)
    if start >= len(y):
        start = len(y) // 4
    return float(abs(np.mean(y[start:])))


def peak_linear(y: np.ndarray) -> float:
    return float(np.max(np.abs(y)))


def load_reference(sr: float, n: int) -> np.ndarray | None:
    if not TARGET_WAV.is_file():
        return None
    rs, data = wavfile.read(TARGET_WAV)
    if data.ndim > 1:
        data = data.mean(axis=1)
    y = data.astype(np.float64)
    if rs != sr:
        # simple resample via scipy if needed
        num = int(len(y) * sr / rs)
        y = signal.resample(y, num)
    if len(y) < n:
        y = np.pad(y, (0, n - len(y)))
    return y[:n].astype(np.float32)


def evaluate() -> dict:
    sr = 48000.0
    root_hz = 49.0
    seed = 261115

    t0 = time.perf_counter()
    buffers = drum_engine.render_batch(root_hz=root_hz, sample_rate=sr, seed=seed)
    elapsed_ms = (time.perf_counter() - t0) * 1000.0

    y = np.concatenate(buffers)
    n = len(y)
    ref = load_reference(sr, n)

    safety = {
        "nanfree": bool(np.isfinite(y).all()),
        "peak_ok": peak_linear(y) <= 0.999,
        "dc_ok": dc_offset(y, sr) < 2e-3,
        "time_ok": elapsed_ms < 60.0,
    }
    safety_pass = int(all(safety.values()))

    timbre = stft_loss(y, ref, sr) if ref is not None else profile_loss(y, sr)
    crest = crest_factor(y)
    crest_target = 4.5
    crest_pen = abs(crest - crest_target) / crest_target
    atk = attack_slope_ms(buffers[-1], sr)
    atk_target = 6.0
    atk_pen = abs(atk - atk_target) / max(atk_target, 1.0)
    thd = thd_ratio(y, sr, fund_hz=root_hz)
    thd_pen = max(0.0, thd - 0.35) * 2.0

    clip_pen = max(0.0, peak_linear(y) - 0.999) * 100.0
    dc_pen = dc_offset(y, sr) * 1e4
    time_pen = max(0.0, elapsed_ms - 60.0) * 0.5

    if not safety["nanfree"]:
        loss = 1e6
    else:
        loss = (
            timbre * 1.0
            + crest_pen * 0.25
            + atk_pen * 0.15
            + thd_pen * 0.35
            + clip_pen
            + dc_pen
            + time_pen
        )

    return {
        "eval_loss": float(loss),
        "safety_pass": safety_pass,
        "timbre": float(timbre),
        "crest_factor": float(crest),
        "attack_ms": float(atk),
        "thd_ratio": float(thd),
        "dc_offset": float(dc_offset(y, sr)),
        "peak": float(peak_linear(y)),
        "render_ms": float(elapsed_ms),
        "used_reference_wav": int(ref is not None),
        **{f"safety_{k}": int(v) for k, v in safety.items()},
    }


def main() -> int:
    m = evaluate()
    print(f"eval_loss={m['eval_loss']:.6f}")
    print(json.dumps(m), file=sys.stderr)
    return 0 if m["safety_pass"] else 2


if __name__ == "__main__":
    raise SystemExit(main())