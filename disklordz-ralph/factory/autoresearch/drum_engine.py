"""
AGENT SANDBOX — only file the autoresearch agent may edit.
"""

from __future__ import annotations

import numpy as np
from scipy.signal import butter, lfilter


def _normalize(sig: np.ndarray, velocity: float) -> np.ndarray:
    sig = sig - np.mean(sig)
    peak = float(np.max(np.abs(sig)))
    if peak > 0.0:
        sig = (sig / peak) * 0.95 * float(velocity)
    return sig.astype(np.float32)


def synthesize_kick(num_samples: int, sample_rate: int, velocity: float = 1.0) -> np.ndarray:
    t = np.arange(num_samples, dtype=np.float64) / float(sample_rate)
    freq = 48.0 + (160.0 - 48.0) * np.exp(-t / 0.06)
    phase = 2.0 * np.pi * np.cumsum(freq) / float(sample_rate)
    osc = np.sin(phase)
    amp_env = np.exp(-t / 0.40)
    signal = np.tanh(osc * amp_env * 1.5)
    return _normalize(signal, velocity)


def synthesize_snare(num_samples: int, sample_rate: int, velocity: float = 1.0) -> np.ndarray:
    t = np.arange(num_samples, dtype=np.float64) / float(sample_rate)

    f1 = 185.0 + 90.0 * np.exp(-t / 0.02)
    f2 = 330.0 + 110.0 * np.exp(-t / 0.015)
    body = 0.6 * np.sin(2.0 * np.pi * np.cumsum(f1) / float(sample_rate))
    body += 0.4 * np.sin(2.0 * np.pi * np.cumsum(f2) / float(sample_rate))
    body *= np.exp(-t / 0.11)

    rng = np.random.default_rng(42)
    noise = rng.standard_normal(num_samples)
    b, a = butter(2, [900.0 / (sample_rate / 2), 7500.0 / (sample_rate / 2)], btype="bandpass")
    snare_wires = lfilter(b, a, noise) * np.exp(-t / 0.16)

    signal = np.tanh((body * 0.8 + snare_wires * 1.1) * 1.3)
    return _normalize(signal, velocity)


def synthesize_hat(num_samples: int, sample_rate: int, velocity: float = 1.0) -> np.ndarray:
    t = np.arange(num_samples, dtype=np.float64) / float(sample_rate)

    ratios = [205.3, 304.4, 369.6, 522.7, 540.0, 800.0]
    metal = np.zeros(num_samples, dtype=np.float64)
    for r in ratios:
        metal += np.sign(np.sin(2.0 * np.pi * r * t))
    metal /= len(ratios)

    b, a = butter(3, 6500.0 / (sample_rate / 2), btype="highpass")
    filtered = lfilter(b, a, metal)
    amp_env = np.exp(-t / 0.045)

    signal = filtered * amp_env
    return _normalize(signal, velocity)