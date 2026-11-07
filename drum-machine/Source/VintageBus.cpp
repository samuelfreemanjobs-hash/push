#include "VintageBus.h"

#include <algorithm>
#include <cmath>

namespace kf
{
namespace
{
    constexpr float butterworthQ = 0.70710678f;
}

//==============================================================================
// VintageCrusher
//==============================================================================
void VintageCrusher::prepare (double sampleRate)
{
    sr = sampleRate;
    setParams (params);
    reset();
}

void VintageCrusher::reset()
{
    for (int ch = 0; ch < 2; ++ch)
    {
        xoverLp1[ch].reset(); xoverLp2[ch].reset();
        xoverHp1[ch].reset(); xoverHp2[ch].reset();
        smooth[ch].reset();
        held[ch] = 0.0f;
    }
    holdPhase = 1.0;
}

void VintageCrusher::setParams (const BusParams& p)
{
    params = p;

    // Crossover (Linkwitz-Riley 4th order = two cascaded Butterworth 2-poles):
    // the low band is passed untouched so kicks and 808 subs keep a clean, solid low end.
    useCrossover = p.lowKeep >= 20.0f;
    if (useCrossover)
        for (int ch = 0; ch < 2; ++ch)
        {
            xoverLp1[ch].setCoefficients (p.lowKeep, butterworthQ, sr);
            xoverLp2[ch].setCoefficients (p.lowKeep, butterworthQ, sr);
            xoverHp1[ch].setCoefficients (p.lowKeep, butterworthQ, sr);
            xoverHp2[ch].setCoefficients (p.lowKeep, butterworthQ, sr);
        }

    // Analog-style pre-gain. Unity-ish for quiet signals, saturating when hot.
    useDrive = p.drive > 0.001f;
    driveGain = 1.0f + 7.0f * p.drive;
    driveMakeup = 1.0f / std::pow (driveGain, 0.6f);

    // Bit depth (continuous, so it can be swept smoothly).
    useBits = p.bits < 15.99f;
    quantSteps = std::pow (2.0f, std::max (1.0f, p.bits - 1.0f));

    // Decimation with the pitch-up / pitch-down trick.
    effectiveRate = (double) p.rate / std::pow (2.0, (double) p.pitch / 12.0);
    useHold = effectiveRate < sr * 0.999;
    holdInc = effectiveRate / sr;

    // 2-pole (12 dB/oct) output smoothing filter.
    useLpf = p.lpf < 19999.0f && p.lpf < (float) sr * 0.49f;
    if (useLpf)
        for (auto& f : smooth)
            f.setCoefficients (p.lpf, butterworthQ, sr);
}

float VintageCrusher::crushChannel (float x) noexcept
{
    if (useDrive)
        x = std::tanh (x * driveGain) * driveMakeup;

    if (useBits)
        x = std::round (x * quantSteps) / quantSteps;

    return x;
}

void VintageCrusher::process (float& left, float& right) noexcept
{
    float in[2] { left, right };
    float low[2] { 0.0f, 0.0f };

    if (useCrossover)
        for (int ch = 0; ch < 2; ++ch)
        {
            low[ch] = xoverLp2[ch].process (xoverLp1[ch].process (in[ch], 0), 0);
            in[ch]  = xoverHp2[ch].process (xoverHp1[ch].process (in[ch], 2), 2);
        }

    float out[2];
    if (useHold)
    {
        // Zero-order hold: only take a new (crushed) sample at the vintage rate.
        holdPhase += holdInc;
        if (holdPhase >= 1.0)
        {
            holdPhase -= std::floor (holdPhase);
            held[0] = crushChannel (in[0]);
            held[1] = crushChannel (in[1]);
        }
        out[0] = held[0];
        out[1] = held[1];
    }
    else
    {
        out[0] = crushChannel (in[0]);
        out[1] = crushChannel (in[1]);
    }

    if (useLpf)
    {
        out[0] = smooth[0].process (out[0], 0);
        out[1] = smooth[1].process (out[1], 0);
    }

    left  = out[0] + low[0];
    right = out[1] + low[1];
}

//==============================================================================
// TransientShaper
//==============================================================================
float TransientShaper::coef (double ms, double sampleRate)
{
    return (float) std::exp (-1.0 / (ms * 0.001 * sampleRate));
}

void TransientShaper::prepare (double sampleRate)
{
    atk1     = coef (0.5, sampleRate);   // fast attack
    attSlowA = coef (25.0, sampleRate);  // slow attack
    relLong  = coef (120.0, sampleRate); // shared release for the attack detector
    susFastR = coef (25.0, sampleRate);  // fast release
    susSlowR = coef (350.0, sampleRate); // slow release
    reset();
}

void TransientShaper::reset()
{
    envAttFast = envAttSlow = envSusFast = envSusSlow = 0.0f;
}

void TransientShaper::process (float& left, float& right) noexcept
{
    const float d = std::max (std::abs (left), std::abs (right));

    auto follow = [d] (float& env, float a, float r)
    {
        const float c = d > env ? a : r;
        env = c * env + (1.0f - c) * d;
    };

    follow (envAttFast, atk1, relLong);
    follow (envAttSlow, attSlowA, relLong);
    follow (envSusFast, atk1, susFastR);
    follow (envSusSlow, atk1, susSlowR);

    if (std::abs (punchAmt) < 1.0e-4f && std::abs (tailAmt) < 1.0e-4f)
        return;

    constexpr float eps = 1.0e-6f;
    const float attackDb  = std::clamp (20.0f * std::log10 ((envAttFast + eps) / (envAttSlow + eps)), 0.0f, 20.0f);
    const float sustainDb = std::clamp (20.0f * std::log10 ((envSusSlow + eps) / (envSusFast + eps)), 0.0f, 30.0f);

    const float gainDb = std::clamp (punchAmt * attackDb * 0.6f + tailAmt * sustainDb * 0.4f, -18.0f, 12.0f);
    const float g = std::pow (10.0f, gainDb * 0.05f);
    left *= g;
    right *= g;
}

//==============================================================================
// VinylNoise
//==============================================================================
void VinylNoise::prepare (double sampleRate)
{
    sr = sampleRate;
    hissCoef = (float) std::exp (-2.0 * 3.14159265 * 5000.0 / sr);
    crackleCoef = (float) std::exp (-1.0 / (0.00025 * sr));
    popCoef = (float) std::exp (-1.0 / (0.004 * sr));
    popInc = (float) (180.0 / sr);
}

float VinylNoise::white() noexcept
{
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return (float) (int32_t) rng * (1.0f / 2147483648.0f);
}

void VinylNoise::process (float& left, float& right) noexcept
{
    if (amount <= 0.0f)
        return;

    // Hiss: low-passed white noise, slightly decorrelated per side.
    hissL = hissCoef * hissL + (1.0f - hissCoef) * white();
    hissR = hissCoef * hissR + (1.0f - hissCoef) * white();
    const float hissLevel = 0.03f * amount;

    // Crackle: sparse random impulses, high-passed by differencing.
    const float crackleRate = 6.0f + 40.0f * amount; // per second
    if ((white() * 0.5f + 0.5f) < crackleRate / (float) sr)
        crackle += (0.15f + 0.85f * std::abs (white())) * (white() > 0.0f ? 1.0f : -1.0f);
    const float c = crackle - cracklePrev;
    cracklePrev = crackle;
    crackle *= crackleCoef;

    // Pop: rare low thump from a dust particle.
    if ((white() * 0.5f + 0.5f) < (0.15f * amount) / (float) sr)
    {
        pop = 0.6f;
        popPhase = 0.0f;
    }
    float p = 0.0f;
    if (pop > 1.0e-4f)
    {
        popPhase += popInc;
        p = pop * std::sin (6.2831853f * popPhase);
        pop *= popCoef;
    }

    const float common = (c * 0.35f + p * 0.25f) * amount;
    left  += hissL * hissLevel + common;
    right += hissR * hissLevel + common;
}
} // namespace kf
