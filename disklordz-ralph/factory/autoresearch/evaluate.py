"""
IMMUTABLE JUDGE — autoresearch agents must NOT edit this file.

Run: python evaluate.py
"""

from __future__ import annotations

import os
import time
from pathlib import Path

import numpy as np
from scipy.io import wavfile
from scipy.signal import hilbert, stft

import drum_engine

ROOT = Path(__file__).resolve().parent
TARGETS = ROOT / "targets"
OUTPUTS = ROOT / "outputs"
BEST_FILE = OUTPUTS / ".best_loss"
BEST_KIT = OUTPUTS / "best_kit.wav"
LEGACY_KICK = ROOT / "target.wav"

SAMPLE_RATE = 44100
VOICES = ("kick", "snare", "hat")
MAX_WALL_SECONDS = 5.0
# Commercial: penalize spectral convergence below this (too close to reference = cloning risk).
MIN_SC_COMPONENT = float(os.environ.get("MIN_SPECTRAL_CONVERGENCE", "0"))
KIT_VELOCITY_TEST = os.environ.get("KIT_VELOCITY_TEST", "").strip() in ("1", "true", "yes")
VELOCITIES = (30 / 127.0, 80 / 127.0, 1.0)


def load_wav(path: Path) -> np.ndarray:
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

        data = resample(data, int(len(data) * SAMPLE_RATE / sr)).astype(np.float32)
    return data


def resolve_target(voice: str) -> np.ndarray | None:
    p = TARGETS / f"{voice}.wav"
    if p.is_file():
        return load_wav(p)
    if voice == "kick" and LEGACY_KICK.is_file():
        return load_wav(LEGACY_KICK)
    return None


def multi_res_stft_loss(y_true: np.ndarray, y_pred: np.ndarray) -> float:
    fft_sizes = [256, 512, 1024, 2048]
    loss = 0.0
    used = 0
    for n_fft in fft_sizes:
        if len(y_true) < n_fft:
            continue
        _, _, z_true = stft(y_true, fs=SAMPLE_RATE, nperseg=n_fft, noverlap=n_fft // 4)
        _, _, z_pred = stft(y_pred, fs=SAMPLE_RATE, nperseg=n_fft, noverlap=n_fft // 4)
        frames = min(z_true.shape[1], z_pred.shape[1])
        mag_true = np.abs(z_true[:, :frames])
        mag_pred = np.abs(z_pred[:, :frames])
        sc_loss = float(np.linalg.norm(mag_true - mag_pred) / (np.linalg.norm(mag_true) + 1e-7))
        log_loss = float(np.mean(np.abs(np.log(mag_true + 1e-7) - np.log(mag_pred + 1e-7))))
        if MIN_SC_COMPONENT > 0 and sc_loss < MIN_SC_COMPONENT:
            sc_loss += (MIN_SC_COMPONENT - sc_loss) * 10.0
        loss += sc_loss + log_loss
        used += 1
    return loss / max(used, 1)


def envelope_loss(y_true: np.ndarray, y_pred: np.ndarray) -> float:
    env_true = np.abs(hilbert(y_true))
    env_pred = np.abs(hilbert(y_pred))
    return float(np.mean(np.abs(env_true - env_pred)))


def voice_loss(target: np.ndarray, generated: np.ndarray) -> float:
    return multi_res_stft_loss(target, generated) + (5.0 * envelope_loss(target, generated))


def check_safety(audio: np.ndarray, expected_shape: tuple[int, ...]) -> str | None:
    if not isinstance(audio, np.ndarray) or audio.shape != expected_shape:
        return "Invalid shape"
    if np.isnan(audio).any() or np.isinf(audio).any():
        return "NaN or Inf detected"
    if float(np.max(np.abs(audio))) > 1.0:
        return "Clipped above 1.0"
    if float(np.abs(np.mean(audio))) > 0.005:
        return "Excessive DC offset"
    return None


def render_demo_beat(rendered_voices: dict[str, np.ndarray]) -> None:
    OUTPUTS.mkdir(parents=True, exist_ok=True)
    step_samples = int(0.25 * SAMPLE_RATE)
    pattern = ["kick", "hat", "snare", "hat", "kick", "kick", "snare", "hat"]
    total_len = step_samples * len(pattern) + SAMPLE_RATE
    mix = np.zeros(total_len, dtype=np.float32)

    for i, voice in enumerate(pattern):
        start = i * step_samples
        hit = rendered_voices[voice]
        end = min(start + len(hit), total_len)
        mix[start:end] += hit[: end - start] * 0.8

    peak = float(np.max(np.abs(mix)))
    if peak > 0.95:
        mix = (mix / peak) * 0.95
    wavfile.write(BEST_KIT, SAMPLE_RATE, np.clip(mix, -1.0, 1.0))


def evaluate() -> int:
    start_time = time.time()
    voice_losses: dict[str, float] = {}
    rendered_voices: dict[str, np.ndarray] = {}

    missing = [v for v in VOICES if resolve_target(v) is None]
    if missing:
        print(f"ERROR: Missing targets: {', '.join(missing)}")
        print("Place kick.wav, snare.wav, hat.wav in targets/ (see targets/README.md).")
        print("TOTAL_LOSS: 9999.0")
        return 2

    for voice in VOICES:
        target = resolve_target(voice)
        assert target is not None
        synth_fn = getattr(drum_engine, f"synthesize_{voice}")
        generated = synth_fn(len(target), SAMPLE_RATE, velocity=1.0)

        err = check_safety(generated, target.shape)
        if err:
            print(f"ERROR [{voice}]: {err}")
            print("TOTAL_LOSS: 9999.0")
            return 2

        loss = voice_loss(target, generated)

        if KIT_VELOCITY_TEST:
            vel_losses = []
            for vel in VELOCITIES:
                g = synth_fn(len(target), SAMPLE_RATE, velocity=vel)
                e = check_safety(g, target.shape)
                if e:
                    print(f"ERROR [{voice} @ vel {vel:.2f}]: {e}")
                    print("TOTAL_LOSS: 9999.0")
                    return 2
                vel_losses.append(voice_loss(target, g))
            loss = 0.35 * loss + 0.65 * float(np.mean(vel_losses))

        voice_losses[voice] = loss
        rendered_voices[voice] = generated

    elapsed = time.time() - start_time
    if elapsed > MAX_WALL_SECONDS:
        print("ERROR: Evaluate exceeded wall time.")
        print("TOTAL_LOSS: 9999.0")
        return 2

    total_loss = float(sum(voice_losses.values()))

    prev_best = float("inf")
    if BEST_FILE.is_file():
        try:
            prev_best = float(BEST_FILE.read_text().strip())
        except ValueError:
            prev_best = float("inf")

    if total_loss < prev_best:
        OUTPUTS.mkdir(parents=True, exist_ok=True)
        BEST_FILE.write_text(f"{total_loss:.6f}")
        render_demo_beat(rendered_voices)

    print(f"RENDER_TIME: {elapsed:.4f}s")
    for voice in VOICES:
        print(f"{voice.upper()}_LOSS: {voice_losses[voice]:.6f}")
    print(f"TOTAL_LOSS: {total_loss:.6f}")
    if KIT_VELOCITY_TEST:
        print("KIT_VELOCITY_TEST: on (velocities ~30, 80, 127)")
    return 0


if __name__ == "__main__":
    raise SystemExit(evaluate())