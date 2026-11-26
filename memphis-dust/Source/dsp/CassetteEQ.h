#pragma once
// STEP 2 — Cassette EQ curve.
// Low cut (12 dB/oct) -> "cardboard box" bell in the low mids -> presence bell @ 3.5 kHz
// (dip it to soften digital harshness, as on a Portastudio) -> steep high cut
// (24 dB/oct Butterworth) that kills the sterile digital air above ~10-12 kHz.

#include "DspUtils.h"

namespace md
{
class CassetteEQ
{
public:
    static constexpr int kMaxCh = 2;

    void prepare (double fs) { sampleRate = fs; dirty = true; reset(); }

    void reset()
    {
        for (int c = 0; c < kMaxCh; ++c) { hp[c].reset(); box[c].reset(); pres[c].reset(); lp1[c].reset(); lp2[c].reset(); }
    }

    void setParams (float lowCutHz, float boxHz, float boxGainDb, float boxQ, float highCutHz, float presenceDb = 0.0f)
    {
        auto same = [] (float a, float b) { return std::abs (a - b) < 1.0e-5f; };
        if (! dirty && same (lowCutHz, lc) && same (boxHz, bf) && same (boxGainDb, bg) && same (boxQ, bq) && same (highCutHz, hc) && same (presenceDb, pr))
            return;

        dirty = false;
        lc = lowCutHz; bf = boxHz; bg = boxGainDb; bq = boxQ; hc = highCutHz; pr = presenceDb;

        const auto hpC  = BiquadCoeffs::highpass (sampleRate, lc, 0.70710678);
        const auto boxC = BiquadCoeffs::peak (sampleRate, bf, bq, bg);
        const auto prC  = BiquadCoeffs::peak (sampleRate, 3500.0, 1.4, pr);
        const auto l1   = BiquadCoeffs::lowpass (sampleRate, hc, 0.54119610);
        const auto l2   = BiquadCoeffs::lowpass (sampleRate, hc, 1.30656296);

        for (int c = 0; c < kMaxCh; ++c) { hp[c].c = hpC; box[c].c = boxC; pres[c].c = prC; lp1[c].c = l1; lp2[c].c = l2; }
    }

    void process (float* const* ch, int numCh, int n) noexcept
    {
        for (int c = 0; c < std::min (numCh, kMaxCh); ++c)
        {
            float* d = ch[c];
            for (int i = 0; i < n; ++i)
                d[i] = lp2[c].process (lp1[c].process (pres[c].process (box[c].process (hp[c].process (d[i])))));
        }
    }

private:
    double sampleRate = 44100.0;
    bool dirty = true;
    float lc = 0, bf = 0, bg = 0, bq = 0, hc = 0, pr = 0;
    Biquad hp[kMaxCh], box[kMaxCh], pres[kMaxCh], lp1[kMaxCh], lp2[kMaxCh];
};
} // namespace md
