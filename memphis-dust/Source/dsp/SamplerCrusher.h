#pragma once
// STEP 1 — 12-bit sampler grit.
// Zero-order-hold sample-rate reduction (fractional, so any target rate works) followed by
// mid-tread quantisation with hard clipping at full scale, like a budget sampler's ADC.
// No reconstruction filter on purpose: the imaging/aliasing IS the sound.
// Optional anti-alias pre-filter gives the smoother S900/S950 flavour instead of SP-style fizz.

#include "DspUtils.h"

namespace md
{
class SamplerCrusher
{
public:
    static constexpr int kMaxCh = 2;

    void prepare (double fs)
    {
        sampleRate = fs;
        lastRate = -1.0f;
        reset();
    }

    void reset()
    {
        for (int c = 0; c < kMaxCh; ++c)
        {
            phase[c] = 1.0;  // sample immediately on first call
            held[c] = 0.0f;
            aa1[c].reset(); aa2[c].reset();
        }
    }

    void setParams (float targetRateHz, float bits, bool antiAlias)
    {
        increment = std::min ((double) targetRateHz / sampleRate, 1.0);
        levels = std::pow (2.0f, std::clamp (bits, 1.0f, 24.0f) - 1.0f);
        useAA = antiAlias;

        if (std::abs (targetRateHz - lastRate) > 1.0e-3f)
        {
            lastRate = targetRateHz;
            const double fc = std::min (0.45 * targetRateHz, 0.45 * sampleRate);
            // 4th-order Butterworth
            const auto c1 = BiquadCoeffs::lowpass (sampleRate, fc, 0.54119610);
            const auto c2 = BiquadCoeffs::lowpass (sampleRate, fc, 1.30656296);
            for (int c = 0; c < kMaxCh; ++c) { aa1[c].c = c1; aa2[c].c = c2; }
        }
    }

    void process (float* const* ch, int numCh, int n) noexcept
    {
        const float top = (levels - 1.0f) / levels;   // two's-complement asymmetry
        for (int c = 0; c < std::min (numCh, kMaxCh); ++c)
        {
            float* d = ch[c];
            for (int i = 0; i < n; ++i)
            {
                float x = d[i];
                if (useAA) x = aa2[c].process (aa1[c].process (x));

                phase[c] += increment;
                if (phase[c] >= 1.0)
                {
                    phase[c] -= 1.0;
                    const float clipped = std::clamp (x, -1.0f, top);
                    held[c] = std::round (clipped * levels) / levels;
                }
                d[i] = held[c];
            }
        }
    }

private:
    double sampleRate = 44100.0, increment = 1.0;
    float levels = 2048.0f, lastRate = -1.0f;
    bool useAA = false;
    double phase[kMaxCh] {};
    float held[kMaxCh] {};
    Biquad aa1[kMaxCh], aa2[kMaxCh];
};
} // namespace md
