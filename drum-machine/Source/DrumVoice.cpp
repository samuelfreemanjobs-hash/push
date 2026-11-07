#include "DrumVoice.h"

#include <algorithm>
#include <cmath>

namespace kf
{
namespace
{
    constexpr float twoPi = 6.283185307179586f;
    constexpr float silence = 1.0e-4f;

    // Classic six-square "metallic" ratios, scaled by the voice pitch.
    constexpr float metalFreqs[6] = { 205.3f, 304.4f, 369.6f, 522.7f, 540.0f, 800.0f };

    inline float wrap (float p) noexcept { return p - std::floor (p); }
}

//==============================================================================
void SVF::setCoefficients (float cutoffHz, float q, double sampleRate)
{
    const float fc = std::clamp (cutoffHz, 10.0f, (float) (sampleRate * 0.49));
    g  = std::tan (3.14159265f * fc / (float) sampleRate);
    k  = 1.0f / std::max (q, 0.05f);
    a1 = 1.0f / (1.0f + g * (g + k));
    a2 = g * a1;
    a3 = g * a2;
}

float SVF::process (float v0, int mode)
{
    const float v3 = v0 - ic2;
    const float v1 = a1 * ic1 + a2 * v3;
    const float v2 = ic2 + a2 * ic1 + a3 * v3;
    ic1 = 2.0f * v1 - ic1;
    ic2 = 2.0f * v2 - ic2;

    switch (mode)
    {
        case 0:  return v2;                 // low-pass
        case 1:  return v1;                 // band-pass
        default: return v0 - k * v1 - v2;   // high-pass
    }
}

//==============================================================================
float DrumVoice::decayCoef (float ms, double sampleRate) noexcept
{
    // Time to fall by 60 dB.
    const double samples = std::max (1.0, (double) ms * 0.001 * sampleRate);
    return (float) std::exp (std::log (0.001) / samples);
}

void DrumVoice::prepare (double sampleRate)
{
    sr = sampleRate;
    active = false;
    noiseFilter.reset();
    clickFilter.reset();
    clickFilter.setCoefficients (2500.0f, 0.7f, sr);
    clickCoef = decayCoef (4.0f, sr);
    chokeCoef = decayCoef (25.0f, sr);
    burstCoef = decayCoef (25.0f, sr);
    burstInterval = (int) (0.009 * sr);
    setParams (params);
}

void DrumVoice::setParams (const VoiceParams& p)
{
    params = p;

    toneCoef  = decayCoef (p.toneDecayMs, sr);
    noiseCoef = decayCoef (p.noiseDecayMs, sr);
    pitchCoef = decayCoef (p.pEnvDecayMs, sr);
    fmCoef    = decayCoef (p.fmDecayMs, sr);
    attackSamples = (int) (p.attackMs * 0.001f * (float) sr);

    noiseFilter.setCoefficients (p.cutoff, p.reso, sr);

    const float metalScale = p.pitch / 400.0f;
    for (int i = 0; i < 6; ++i)
        metalInc[i] = std::min (0.45f, metalFreqs[i] * metalScale / (float) sr);

    driveGain = 1.0f + p.drive * 12.0f;
    driveNorm = 1.0f / std::tanh (driveGain);
}

void DrumVoice::trigger (float vel)
{
    velocity = std::clamp (vel, 0.0f, 1.0f);
    active = true;
    choking = false;
    chokeGain = 1.0f;

    toneEnv = 1.0f;
    pitchEnv = 1.0f;
    clickEnv = 1.0f;
    fmEnv = 1.0f;
    sampleCount = 0;

    phase = 0.0f;
    modPhase = 0.0f;

    burstsLeft = std::max (0, params.bursts - 1);
    burstCounter = 0;
    noiseEnv = 1.0f;
}

void DrumVoice::choke()
{
    if (active)
        choking = true;
}

float DrumVoice::nextWhite() noexcept
{
    // xorshift32
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return (float) (int32_t) rng * (1.0f / 2147483648.0f);
}

float DrumVoice::nextMetal() noexcept
{
    float sum = 0.0f;
    for (int i = 0; i < 6; ++i)
    {
        metalPhase[i] = wrap (metalPhase[i] + metalInc[i]);
        sum += metalPhase[i] < 0.5f ? 1.0f : -1.0f;
    }
    return sum * (1.0f / 6.0f);
}

float DrumVoice::osc (float p) const noexcept
{
    switch (params.wave)
    {
        case 0:  return std::sin (twoPi * p);
        case 1:  return 4.0f * std::abs (p - 0.5f) - 1.0f;
        case 2:  return 2.0f * p - 1.0f;
        default: return p < 0.5f ? 1.0f : -1.0f;
    }
}

float DrumVoice::process() noexcept
{
    if (! active)
        return 0.0f;

    const float fs = (float) sr;

    // --- Tone oscillator with pitch envelope and phase-modulation FM ---
    float tone = 0.0f;
    if (params.toneLevel > 0.0f && toneEnv > silence)
    {
        float freq = params.pitch * std::exp2 (params.pEnvAmt * pitchEnv * (1.0f / 12.0f));
        freq = std::min (freq, fs * 0.45f);

        float pm = 0.0f;
        if (params.fmAmt > 0.0f)
        {
            modPhase = wrap (modPhase + std::min (freq * params.fmRatio / fs, 0.45f));
            pm = std::sin (twoPi * modPhase) * params.fmAmt * fmEnv * toneEnv * (1.0f / twoPi);
        }

        phase = wrap (phase + freq / fs);
        tone = osc (wrap (phase + pm)) * toneEnv * params.toneLevel;
    }
    toneEnv *= toneCoef;
    pitchEnv *= pitchCoef;
    fmEnv *= fmCoef;

    // --- Noise / metallic generator ---
    float noise = 0.0f;
    if (params.noiseLevel > 0.0f && noiseEnv > silence)
    {
        const float src = params.noiseType == 0 ? nextWhite() : nextMetal();
        noise = noiseFilter.process (src, params.filterMode) * noiseEnv * params.noiseLevel;
    }

    if (burstsLeft > 0)
    {
        // Short re-triggered bursts give the "smeared" hand-clap attack.
        noiseEnv *= burstCoef;
        if (++burstCounter >= burstInterval)
        {
            burstCounter = 0;
            --burstsLeft;
            noiseEnv = 1.0f;
        }
    }
    else
    {
        noiseEnv *= noiseCoef;
    }

    // --- Transient click ---
    float clk = 0.0f;
    if (params.click > 0.0f && clickEnv > silence)
        clk = clickFilter.process (nextWhite(), 2) * clickEnv * params.click * 1.5f;
    clickEnv *= clickCoef;

    float out = tone + noise + clk;

    // --- Attack ramp ---
    if (sampleCount < attackSamples)
        out *= (float) sampleCount / (float) attackSamples;
    ++sampleCount;

    // --- Drive ---
    if (params.drive > 0.001f)
        out = std::tanh (out * driveGain) * driveNorm;

    out *= params.level * velocity;

    // --- Choke fade ---
    if (choking)
    {
        chokeGain *= chokeCoef;
        out *= chokeGain;
        if (chokeGain < silence)
            active = false;
    }

    if (toneEnv < silence && noiseEnv < silence && clickEnv < silence && burstsLeft == 0)
        active = false;

    return out;
}
} // namespace kf
