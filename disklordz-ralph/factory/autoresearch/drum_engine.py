"""
AGENT SANDBOX — only file the autoresearch agent may edit.

Synthesis-only BR-style 808. Deterministic given seed.
"""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np


@dataclass
class EngineParams:
    root_hz: float = 49.0
    pitch_start_hz: float = 155.0
    pitch_tau_ms: float = 28.0
    amp_tau_ms: float = 420.0
    drive: float = 1.35
    click_mix: float = 0.12
    click_decay_ms: float = 4.0
    clip_pos: float = 0.92  # asymmetric soft clip
    clip_neg: float = 1.05


DEFAULT = EngineParams()


def _pitch_hz(t: np.ndarray, p: EngineParams) -> np.ndarray:
    tau = max(p.pitch_tau_ms, 1.0) * 1e-3
    start = np.clip(p.pitch_start_hz, p.root_hz, 8000.0)
    return p.root_hz + (start - p.root_hz) * np.exp(-t / tau)


def _soft_asymmetric(x: np.ndarray, pos: float, neg: float) -> np.ndarray:
    pos = float(np.clip(pos, 0.1, 3.0))
    neg = float(np.clip(neg, 0.1, 3.0))
    y = np.empty_like(x)
    mask = x >= 0
    y[mask] = np.tanh(x[mask] * pos)
    y[~mask] = np.tanh(x[~mask] * neg)
    return y


def render_hit(
    *,
    velocity: int = 100,
    root_hz: float | None = None,
    sample_rate: float = 48000.0,
    duration_s: float = 1.2,
    seed: int = 261115,
    params: EngineParams | None = None,
) -> np.ndarray:
    """Mono float32 [-1, 1]. Agent tunes params / synthesis graph here."""
    p = params or DEFAULT
    if root_hz is not None:
        p = EngineParams(**{**p.__dict__, "root_hz": float(root_hz)})

    vel = float(np.clip(velocity, 1, 127) / 127.0)
    rng = np.random.default_rng(seed)
    n = int(sample_rate * duration_s)
    t = np.arange(n, dtype=np.float64) / sample_rate

    phase = np.cumsum(_pitch_hz(t, p) / sample_rate) * (2.0 * np.pi)
    body = np.sin(phase)

    amp_tau = max(p.amp_tau_ms, 2.0) * 1e-3
    env = np.exp(-t / amp_tau) * (0.35 + 0.65 * vel)

    click_n = max(int(p.click_decay_ms * 1e-3 * sample_rate), 1)
    click = rng.standard_normal(click_n).astype(np.float64)
    click *= np.linspace(1.0, 0.0, click_n)
    click_full = np.zeros(n, dtype=np.float64)
    click_full[:click_n] = click

    x = body * env + click_full * p.click_mix * vel
    drive = p.drive * (0.75 + 0.5 * vel)
    x = _soft_asymmetric(x * drive, p.clip_pos, p.clip_neg)

    # One-pole DC blocker (evaluate still measures residual)
    r = 0.995
    y = np.empty_like(x)
    prev_x = 0.0
    prev_y = 0.0
    for i, xi in enumerate(x):
        yi = xi - prev_x + r * prev_y
        prev_x = float(xi)
        prev_y = yi
        y[i] = yi
    x = y

    peak = np.max(np.abs(x)) + 1e-12
    if peak > 0.99:
        x *= 0.99 / peak
    return x.astype(np.float32)


def render_batch(
    velocities: tuple[int, ...] = (32, 64, 100, 127),
    root_hz: float = 49.0,
    sample_rate: float = 48000.0,
    seed: int = 261115,
) -> list[np.ndarray]:
    """Fixed harness entry — evaluate.py calls this."""
    return [
        render_hit(velocity=v, root_hz=root_hz, sample_rate=sample_rate, seed=seed + v)
        for v in velocities
    ]