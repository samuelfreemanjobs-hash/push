"""
AGENT SANDBOX — only file the autoresearch agent may edit.
"""

from __future__ import annotations

import numpy as np


def synthesize_kick(num_samples: int, sample_rate: int, velocity: float = 1.0) -> np.ndarray:
    """Render mono kick/808. Shape must match target.wav length exactly."""
    t = np.arange(num_samples, dtype=np.float64) / float(sample_rate)

    f_start = 160.0
    f_end = 48.0
    pitch_decay = 0.065
    freq = f_end + (f_start - f_end) * np.exp(-t / pitch_decay)

    phase = 2.0 * np.pi * np.cumsum(freq) / float(sample_rate)
    osc = np.sin(phase)

    amp_decay = 0.45
    amp_env = np.exp(-t / amp_decay)

    attack_samples = int(0.002 * sample_rate)
    if attack_samples > 0:
        amp_env[:attack_samples] *= np.linspace(0.0, 1.0, attack_samples, dtype=np.float64)

    drive = 1.5
    signal = np.tanh(osc * amp_env * drive)

    peak = float(np.max(np.abs(signal)))
    if peak > 0.0:
        signal = (signal / peak) * 0.95 * float(velocity)

    return signal.astype(np.float32)


def render_batch(
    velocities: tuple[int, ...] = (32, 64, 100, 127),
    root_hz: float = 49.0,
    sample_rate: float = 44100.0,
    seed: int = 261115,
) -> list[np.ndarray]:
    """Legacy helper — evaluate.py uses synthesize_kick when target.wav is present."""
    del root_hz, seed
    dur_s = 1.2
    n = int(sample_rate * dur_s)
    return [synthesize_kick(n, int(sample_rate), velocity=v / 127.0) for v in velocities]