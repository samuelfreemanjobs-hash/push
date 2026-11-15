#pragma once
// Master-bus utility: everything below the crossover is summed to true mono
// (Linkwitz-Riley 4th order, flat on recombination). No distortion - just
// centred, solid cassette low end that doesn't waste headroom on stereo subs.

#include "DspUtils.h"

namespace md
{
class MonoBass
{
public:
    static constexpr int kMaxCh = 2;

    void prepare (double fs) { sampleRate = fs; lastFreq = -1.0f; reset(); }
    void reset()
    {
        for (int c = 0; c < kMaxCh; ++c) { lp1[c].reset(); lp2[c].reset(); hp1[c].reset(); hp2[c].reset(); }
    }

    void setFrequency (float hz)
    {
        if (std::abs (hz - lastFreq) < 1.0e-3f) return;
        lastFreq = hz;
        const auto lp = BiquadCoeffs::lowpass (sampleRate, hz, 0.70710678);
        const auto hp = BiquadCoeffs::highpass (sampleRate, hz, 0.70710678);
        for (int c = 0; c < kMaxCh; ++c) { lp1[c].c = lp2[c].c = lp; hp1[c].c = hp2[c].c = hp; }
    }

    void process (float* const* ch, int numCh, int n) noexcept
    {
        if (numCh < 2) return; // already mono
        for (int i = 0; i < n; ++i)
        {
            const float l = ch[0][i], r = ch[1][i];
            const float low = 0.5f * (lp2[0].process (lp1[0].process (l)) + lp2[1].process (lp1[1].process (r)));
            ch[0][i] = hp2[0].process (hp1[0].process (l)) + low;
            ch[1][i] = hp2[1].process (hp1[1].process (r)) + low;
        }
    }

private:
    double sampleRate = 44100.0;
    float lastFreq = -1.0f;
    Biquad lp1[kMaxCh], lp2[kMaxCh], hp1[kMaxCh], hp2[kMaxCh];
};
} // namespace md
