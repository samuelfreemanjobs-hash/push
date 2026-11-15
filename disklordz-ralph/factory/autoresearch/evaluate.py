"""
IMMUTABLE JUDGE — autoresearch agents must NOT edit this file.

Run: python evaluate.py
"""

from __future__ import annotations

import json
import time
from pathlib import Path

import numpy as np
from scipy.io import wavfile
from scipy.signal import hilbert, stft

import drum_engine

ROOT = Path(__file__).resolve().parent
TARGET_PRIMARY = ROOT / "target.wav"
TARGET_FALLBACK = ROOT / "targets" / "reference_808.wav"
BEST_WAV = ROOT / "best_attempt.wav"
BEST_STATE = ROOT / ".autoresearch_best.json"

SAMPLE_RATE = 44100
MAX_WALL_SECONDS = 5.0


def load_target(path: Path) -> tuple[np.ndarray, int]:
    sr, data = wavfile.read(path)
    if data.dtype == np.int16:
        data = data.astype(np.float32) / 32768.0
    elif data.dtype == np.int32:
        data = data.astype(np.float32) / 2147483648.0
    else:
        data = data.astype(np.float32)
    if data.ndim > 1:
        data = np.mean(data, axis=1)
    if sr != SAMPLE_RATE:
        from scipy.signal import resample

        n = int(len(data) * SAMPLE_RATE / sr)
        data = resample(data, n).astype(np.float32)
    return data, SAMPLE_RATE


def multi_res_stft_loss(y_true: np.ndarray, y_pred: np.ndarray) -> float:
    fft_sizes = [256, 512, 1024, 2048]
    loss = 0.0
    for n_fft in fft_sizes:
        _, _, z_true = stft(y_true, fs=SAMPLE_RATE, nperseg=n_fft, noverlap=n_fft // 4)
        _, _, z_pred = stft(y_pred, fs=SAMPLE_RATE, nperseg=n_fft, noverlap=n_fft // 4)
        frames = min(z_true.shape[1], z_pred.shape[1])
        mag_true = np.abs(z_true[:, :frames])
        mag_pred = np.abs(z_pred[:, :frames])
        sc_loss = float(np.linalg.norm(mag_true - mag_pred) / (np.linalg.norm(mag_true) + 1e-7))
        log_loss = float(np.mean(np.abs(np.log(mag_true + 1e-7) - np.log(mag_pred + 1e-7))))
        loss += sc_loss + log_loss
    return loss / len(fft_sizes)


def envelope_loss(y_true: np.ndarray, y_pred: np.ndarray) -> float:
    env_true = np.abs(hilbert(y_true))
    env_pred = np.abs(hilbert(y_pred))
    return float(np.mean(np.abs(env_true - env_pred)))


def profile_fallback_loss(generated: np.ndarray) -> float:
    """When no target.wav — coarse BR-808 band profile (not comparable to reference mode)."""
    from scipy import signal

    f, pxx = signal.welch(generated, fs=SAMPLE_RATE, nperseg=4096)
    total = float(np.sum(pxx)) + 1e-18
    targets = ((30, 80, 0.42), (80, 200, 0.28), (200, 800, 0.12))
    loss = 0.0
    for lo, hi, frac in targets:
        mask = (f >= lo) & (f < hi)
        loss += abs(float(np.sum(pxx[mask])) / total - frac)
    return loss


def save_best_if_improved(generated: np.ndarray, total_loss: float) -> None:
    prev = float("inf")
    if BEST_STATE.is_file():
        try:
            prev = float(json.loads(BEST_STATE.read_text()).get("TOTAL_LOSS", float("inf")))
        except (json.JSONDecodeError, TypeError, ValueError):
            prev = float("inf")
    if total_loss < prev:
        wavfile.write(BEST_WAV, SAMPLE_RATE, np.clip(generated, -1.0, 1.0))
        BEST_STATE.write_text(json.dumps({"TOTAL_LOSS": total_loss}, indent=2))


def resolve_target() -> np.ndarray | None:
    for path in (TARGET_PRIMARY, TARGET_FALLBACK):
        if path.is_file():
            y, _ = load_target(path)
            return y
    return None


def evaluate() -> None:
    start_time = time.time()
    target = resolve_target()

    if target is not None:
        num_samples = len(target)
        generated = drum_engine.synthesize_kick(num_samples, SAMPLE_RATE, velocity=1.0)
    else:
        num_samples = int(SAMPLE_RATE * 1.2)
        generated = drum_engine.synthesize_kick(num_samples, SAMPLE_RATE, velocity=1.0)
        target = generated  # envelope term → 0; profile still meaningful

    elapsed = time.time() - start_time

    if not isinstance(generated, np.ndarray):
        print("ERROR: Invalid output type.")
        print("TOTAL_LOSS: 9999.0")
        return
    if target is not None and generated.shape != target.shape:
        print("ERROR: Invalid output shape.")
        print("TOTAL_LOSS: 9999.0")
        return
    if np.isnan(generated).any() or np.isinf(generated).any():
        print("ERROR: NaN or Inf detected.")
        print("TOTAL_LOSS: 9999.0")
        return
    if float(np.max(np.abs(generated))) > 1.0:
        print("ERROR: Audio clipped above 1.0.")
        print("TOTAL_LOSS: 9999.0")
        return
    if float(np.abs(np.mean(generated))) > 0.005:
        print("ERROR: Excessive DC offset.")
        print("TOTAL_LOSS: 9999.0")
        return
    if elapsed > MAX_WALL_SECONDS:
        print("ERROR: Evaluate exceeded wall time.")
        print("TOTAL_LOSS: 9999.0")
        return

    if TARGET_PRIMARY.is_file() or TARGET_FALLBACK.is_file():
        spec_loss = multi_res_stft_loss(target, generated)
        env_loss = envelope_loss(target, generated)
        total_loss = spec_loss + (5.0 * env_loss)
    else:
        spec_loss = profile_fallback_loss(generated)
        env_loss = 0.0
        total_loss = spec_loss

    save_best_if_improved(generated, total_loss)

    print(f"RENDER_TIME: {elapsed:.4f}s")
    print(f"SPECTRAL_LOSS: {spec_loss:.6f}")
    print(f"ENVELOPE_LOSS: {env_loss:.6f}")
    print(f"TOTAL_LOSS: {total_loss:.6f}")
    if not (TARGET_PRIMARY.is_file() or TARGET_FALLBACK.is_file()):
        print("NOTE: No target.wav — drop `target.wav` or `targets/reference_808.wav` for reference mode.")


if __name__ == "__main__":
    evaluate()