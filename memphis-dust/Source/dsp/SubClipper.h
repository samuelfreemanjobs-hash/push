#pragma once
// STEP 5 — Monophonic sub / 808 clipper.
// Linkwitz-Riley 4th-order crossover splits off the lows (flat magnitude on recombination).
// Lows are summed to mono, driven into a tanh/hard-clip blend at a fixed ceiling, then
// low-passed again (24 dB/oct) at the same frequency so only the "knock" harmonics that
// translate on phone/car speakers survive, without fizz leaking into the mids.

#include "DspUtils.h"

namespace md
{
class SubClipper
{
public:
    static constexpr int kMaxCh = 2;

    void prepare (double fs)
    {
        sampleRate = fs;
        lastFreq = -1.0f;
        drive.prepare (fs, 0.03, 1.0f);
        level.prepare (fs, 0.03, 1.0f);
        hardness.prepare (fs, 0.03, 0.5f);
        reset();
    }

    void reset()
    {
        for (int c = 0; c < kMaxCh; ++c) { lp1[c].reset(); lp2[c].reset(); hp1[c].reset(); hp2[c].reset(); }
        post1.reset(); post2.reset();
    }

    void setParams (float freqHz, float driveDb, float hard01, float levelDb)
    {
        drive.setTarget (dbToGain (driveDb));
        level.setTarget (dbToGain (levelDb));
        hardness.setTarget (hard01);

        if (std::abs (freqHz - lastFreq) > 1.0e-3f)
        {
            lastFreq = freqHz;
            const auto lp = BiquadCoeffs::lowpass (sampleRate, freqHz, 0.70710678);
            const auto hp = BiquadCoeffs::highpass (sampleRate, freqHz, 0.70710678);
            for (int c = 0; c < kMaxCh; ++c) { lp1[c].c = lp; lp2[c].c = lp; hp1[c].c = hp; hp2[c].c = hp; }
            post1.c = BiquadCoeffs::lowpass (sampleRate, freqHz, 0.54119610);
            post2.c = BiquadCoeffs::lowpass (sampleRate, freqHz, 1.30656296);
        }
    }

    void process (float* const* ch, int numCh, int n) noexcept
    {
        numCh = std::min (numCh, kMaxCh);
        constexpr float ceiling = 0.7f;

        for (int i = 0; i < n; ++i)
        {
            const float g = drive.next();
            const float h = hardness.next();
            const float out = level.next() * std::pow (g, -0.3f); // partial loudness comp: drive still pushes the knock forward

            float lowSum = 0.0f;
            float highs[kMaxCh] {};
            for (int c = 0; c < numCh; ++c)
            {
                const float x = ch[c][i];
                lowSum   += lp2[c].process (lp1[c].process (x));
                highs[c]  = hp2[c].process (hp1[c].process (x));
            }
            const float mono = lowSum / (float) numCh;

            const float u = g * mono / ceiling;
            const float soft = std::tanh (u);
            const float hardC = std::clamp (u, -1.0f, 1.0f);
            float clipped = ceiling * ((1.0f - h) * soft + h * hardC);

            const float sub = post2.process (post1.process (clipped)) * out;

            for (int c = 0; c < numCh; ++c)
                ch[c][i] = highs[c] + sub;
        }
    }

private:
    double sampleRate = 44100.0;
    float lastFreq = -1.0f;
    Smoother drive, level, hardness;
    Biquad lp1[kMaxCh], lp2[kMaxCh], hp1[kMaxCh], hp2[kMaxCh];
    Biquad post1, post2;
};
} // namespace md
