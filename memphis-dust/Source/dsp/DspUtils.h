#pragma once
// Small, allocation-free DSP building blocks used by every stage.
// Pure standard C++ so the DSP can be unit-tested without JUCE.

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace md
{
constexpr double kPi    = 3.14159265358979323846;
constexpr double kTwoPi = 2.0 * kPi;

inline float dbToGain (float db) noexcept { return std::pow (10.0f, db * 0.05f); }
inline float gainToDb (float g) noexcept  { return 20.0f * std::log10 (std::max (g, 1.0e-9f)); }

//==============================================================================
// RBJ-cookbook biquad coefficients (normalised so a0 == 1).
struct BiquadCoeffs
{
    double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;

    static double clampFreq (double fs, double f) { return std::clamp (f, 1.0, fs * 0.49); }

    static BiquadCoeffs lowpass (double fs, double f, double q)
    {
        const double w0 = kTwoPi * clampFreq (fs, f) / fs, c = std::cos (w0), alpha = std::sin (w0) / (2.0 * q);
        return normalise ((1.0 - c) * 0.5, 1.0 - c, (1.0 - c) * 0.5, 1.0 + alpha, -2.0 * c, 1.0 - alpha);
    }

    static BiquadCoeffs highpass (double fs, double f, double q)
    {
        const double w0 = kTwoPi * clampFreq (fs, f) / fs, c = std::cos (w0), alpha = std::sin (w0) / (2.0 * q);
        return normalise ((1.0 + c) * 0.5, -(1.0 + c), (1.0 + c) * 0.5, 1.0 + alpha, -2.0 * c, 1.0 - alpha);
    }

    static BiquadCoeffs peak (double fs, double f, double q, double gainDb)
    {
        const double A = std::pow (10.0, gainDb / 40.0);
        const double w0 = kTwoPi * clampFreq (fs, f) / fs, c = std::cos (w0), alpha = std::sin (w0) / (2.0 * q);
        return normalise (1.0 + alpha * A, -2.0 * c, 1.0 - alpha * A, 1.0 + alpha / A, -2.0 * c, 1.0 - alpha / A);
    }

    // Shelf slope S = 1. Note: highShelf(+g) followed by highShelf(-g) is an exact inverse.
    static BiquadCoeffs highShelf (double fs, double f, double gainDb)
    {
        const double A = std::pow (10.0, gainDb / 40.0), sA = std::sqrt (A);
        const double w0 = kTwoPi * clampFreq (fs, f) / fs, c = std::cos (w0);
        const double alpha = std::sin (w0) / 2.0 * std::sqrt (2.0);
        return normalise (A * ((A + 1.0) + (A - 1.0) * c + 2.0 * sA * alpha),
                          -2.0 * A * ((A - 1.0) + (A + 1.0) * c),
                          A * ((A + 1.0) + (A - 1.0) * c - 2.0 * sA * alpha),
                          (A + 1.0) - (A - 1.0) * c + 2.0 * sA * alpha,
                          2.0 * ((A - 1.0) - (A + 1.0) * c),
                          (A + 1.0) - (A - 1.0) * c - 2.0 * sA * alpha);
    }

private:
    static BiquadCoeffs normalise (double b0, double b1, double b2, double a0, double a1, double a2)
    {
        return { b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 };
    }
};

// Transposed direct form II, double-precision state (stable for low cutoffs at high rates).
struct Biquad
{
    BiquadCoeffs c;
    double s1 = 0.0, s2 = 0.0;

    inline float process (float xIn) noexcept
    {
        const double x = xIn;
        const double y = c.b0 * x + s1;
        s1 = c.b1 * x - c.a1 * y + s2;
        s2 = c.b2 * x - c.a2 * y;
        return (float) y;
    }

    void reset() noexcept { s1 = s2 = 0.0; }
};

// 1-pole DC blocker (~5 Hz).
struct DcBlocker
{
    float R = 0.9995f, x1 = 0.0f, y1 = 0.0f;
    void prepare (double fs) { R = (float) std::exp (-kTwoPi * 5.0 / fs); reset(); }
    void reset() { x1 = y1 = 0.0f; }
    inline float process (float x) noexcept
    {
        const float y = x - x1 + R * y1;
        x1 = x; y1 = y;
        return y;
    }
};

// Fast, deterministic white noise in [-1, 1).
struct Rng
{
    uint32_t state = 0x9E3779B9u;
    explicit Rng (uint32_t seed = 0x9E3779B9u) : state (seed ? seed : 1u) {}
    inline float next() noexcept
    {
        state ^= state << 13; state ^= state >> 17; state ^= state << 5;
        return (float) ((int32_t) state) * (1.0f / 2147483648.0f);
    }
};

// One-pole parameter smoother (per-sample, exponential).
struct Smoother
{
    float current = 0.0f, target = 0.0f, coeff = 0.0f;
    void prepare (double fs, double timeSec, float initial)
    {
        coeff = (float) std::exp (-1.0 / (std::max (timeSec, 1.0e-4) * fs));
        current = target = initial;
    }
    void setTarget (float t) noexcept { target = t; }
    inline float next() noexcept { current = target + coeff * (current - target); return current; }
};

// Soft knee into a hard ceiling at +/-1: linear below t, smooth tanh shoulder above.
inline float softClip (float x, float t = 0.7f) noexcept
{
    const float a = std::abs (x);
    if (a <= t) return x;
    const float k = 1.0f - t;
    const float y = t + k * std::tanh ((a - t) / k);
    return std::copysign (y, x);
}

} // namespace md
