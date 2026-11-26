#pragma once
// STEP 3a — Tape saturation. Runs at the oversampled rate.
// Pre-emphasis (HF boost) -> biased tanh (even + odd harmonics, transients square off)
// -> exact inverse de-emphasis. This mimics how real tape saturates highs first,
// rounding and squashing snare/hat transients before the body distorts.

#include "DspUtils.h"

namespace md
{
class TapeSaturator
{
public:
    static constexpr int kMaxCh = 2;

    void prepare (double fsOversampled)
    {
        sampleRate = fsOversampled;
        const auto pre  = BiquadCoeffs::highShelf (sampleRate, 3000.0, +6.0);
        const auto post = BiquadCoeffs::highShelf (sampleRate, 3000.0, -6.0);
        for (int c = 0; c < kMaxCh; ++c) { preEmph[c].c = pre; deEmph[c].c = post; dc[c].prepare (sampleRate); }
        drive.prepare (sampleRate, 0.03, 1.0f);
        bias.prepare (sampleRate, 0.03, 0.0f);
        reset();
    }

    void reset()
    {
        for (int c = 0; c < kMaxCh; ++c) { preEmph[c].reset(); deEmph[c].reset(); dc[c].reset(); }
    }

    // driveDb: 0..24, biasAmt: 0..1
    void setParams (float driveDb, float biasAmt)
    {
        drive.setTarget (dbToGain (driveDb));
        bias.setTarget (biasAmt * 0.6f);
    }

    void process (float* const* ch, int numCh, int n) noexcept
    {
        numCh = std::min (numCh, kMaxCh);
        for (int i = 0; i < n; ++i)
        {
            const float g = drive.next();
            const float b = bias.next();
            const float tb = std::tanh (b);
            const float slope = 1.0f - tb * tb;            // small-signal gain of tanh at the bias point
            const float comp = 1.0f / (slope * std::pow (g, 0.55f)); // keep loudness roughly sane

            for (int c = 0; c < numCh; ++c)
            {
                float x = preEmph[c].process (ch[c][i]);
                float y = (std::tanh (g * x + b) - tb) * comp;
                y = deEmph[c].process (y);
                ch[c][i] = dc[c].process (y);
            }
        }
    }

private:
    double sampleRate = 88200.0;
    Smoother drive, bias;
    Biquad preEmph[kMaxCh], deEmph[kMaxCh];
    DcBlocker dc[kMaxCh];
};
} // namespace md
