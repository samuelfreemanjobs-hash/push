#pragma once
// Worn-tape dropouts: random, short losses of head contact. Each event dips the level
// and smears the highs (oxide shedding / crinkled tape), with a raised-cosine envelope.
// Runs "on tape", i.e. inside the dbx loop, so the decoder exaggerates them like a real deck.

#include "DspUtils.h"

namespace md
{
class TapeDropouts
{
public:
    static constexpr int kMaxCh = 2;

    void prepare (double fs) { sampleRate = fs; reset(); }
    void reset()
    {
        remaining = length = 0;
        for (auto& s : lpState) s = 0.0f;
    }

    void setAmount (float amount01) { amount = std::clamp (amount01, 0.0f, 1.0f); }

    void process (float* const* ch, int numCh, int n) noexcept
    {
        numCh = std::min (numCh, kMaxCh);
        // up to ~2.5 events / second at 100%
        const float pEvent = amount * 2.5f / (float) sampleRate;

        for (int i = 0; i < n; ++i)
        {
            if (remaining <= 0)
            {
                if (amount > 0.0f && 0.5f + 0.5f * rng.next() < pEvent)
                {
                    const float r = 0.5f + 0.5f * rng.next();
                    length = remaining = (int) (sampleRate * (0.015 + 0.13 * r));
                    depthDb = -(2.0f + 14.0f * amount * (0.4f + 0.6f * (0.5f + 0.5f * rng.next())));
                }
                else
                {
                    for (int c = 0; c < numCh; ++c) lpState[c] = ch[c][i];  // keep LP primed
                    continue;
                }
            }

            const float phase = 1.0f - (float) remaining / (float) length;
            const float env = 0.5f - 0.5f * std::cos ((float) kTwoPi * phase);   // 0 -> 1 -> 0
            --remaining;

            const float g = dbToGain (depthDb * env);
            const float cutoff = 18000.0f * std::pow (0.1f, env);                // 18 kHz -> 1.8 kHz at the bottom
            const float a = 1.0f - std::exp (-(float) kTwoPi * cutoff / (float) sampleRate);
            for (int c = 0; c < numCh; ++c)
            {
                lpState[c] += a * (ch[c][i] - lpState[c]);
                ch[c][i] = lpState[c] * g;
            }
        }
    }

private:
    double sampleRate = 44100.0;
    float amount = 0.0f, depthDb = 0.0f;
    int remaining = 0, length = 0;
    float lpState[kMaxCh] {};
    Rng rng { 0x5EEDu };
};
} // namespace md
