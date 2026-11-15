#!/usr/bin/env python3
"""Synthesis-only demo one-shots for beat pad (24-bit PCM). No copyrighted samples."""

from __future__ import annotations

import math
import struct
import wave
from pathlib import Path

SR = 44100
OUT = Path(__file__).resolve().parent.parent / "public" / "demos"


def write_wav(path: Path, samples: list[float]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    peak = max(max(abs(s) for s in samples), 1e-9)
    norm = [int(max(-1.0, min(1.0, s / peak)) * 0x7FFFFF) for s in samples]
    with wave.open(str(path), "w") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(3)
        wf.setframerate(SR)
        frames = b"".join(struct.pack("<i", s << 8) for s in norm)
        wf.writeframes(frames)


def sine(freq: float, dur: float, amp: float = 1.0) -> list[float]:
    n = int(SR * dur)
    return [amp * math.sin(2 * math.pi * freq * t / SR) for t in range(n)]


def decay(samples: list[float], tau: float) -> list[float]:
    return [s * math.exp(-i / (SR * tau)) for i, s in enumerate(samples)]


def mix(*tracks: list[float]) -> list[float]:
    m = max(len(t) for t in tracks)
    out = [0.0] * m
    for t in tracks:
        for i, s in enumerate(t):
            out[i] += s
    return out


def soft_clip(samples: list[float], gain: float = 1.8) -> list[float]:
    return [math.tanh(s * gain) for s in samples]


def kick_808() -> list[float]:
    body = decay(sine(52.0, 0.55, 1.0), 0.18)
    click = decay(sine(180.0, 0.04, 0.35), 0.01)
    return soft_clip(mix(body, click))


def cowbell() -> list[float]:
    return soft_clip(mix(sine(587.0, 0.22, 0.55), sine(880.0, 0.22, 0.45)))


def trap_bell() -> list[float]:
    return decay(soft_clip(sine(740.0, 0.35, 0.7)), 0.12)


def phonk_snare() -> list[float]:
    noise = [(hash(i) % 1000) / 500.0 - 1.0 for i in range(int(SR * 0.2))]
    tone = decay(sine(200.0, 0.2, 0.4), 0.05)
    return soft_clip(mix(noise, tone))


def main() -> None:
    write_wav(OUT / "808.wav", kick_808())
    write_wav(OUT / "cowbell.wav", cowbell())
    write_wav(OUT / "trap_bell.wav", trap_bell())
    write_wav(OUT / "snare.wav", phonk_snare())
    print(f"Wrote 4 demos under {OUT}")


if __name__ == "__main__":
    main()