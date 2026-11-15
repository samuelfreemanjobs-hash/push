#pragma once
// STEP 3b — Wow & flutter: a modulated delay line (cubic Hermite interpolation).
// Wow   = slow capstan/reel speed drift (~0.5 Hz sine + random drift)
// Flutter = fast motor/pinch-roller wobble (~6.5 Hz + 11 Hz + band-limited noise)
// The delay always runs at a fixed base delay so plugin latency stays constant
// even when the tape section is switched off.

#include "DspUtils.h"
#include <vector>

namespace md
{
class WowFlutter
{
public:
    static constexpr int kMaxCh = 2;
    static constexpr double kBaseDelaySec = 0.003;
    static constexpr double kWowMaxSec     = 0.0012;  // ~0.4% pitch dev at full depth
    static constexpr double kFlutterMaxSec = 0.00004; // ~0.15% at full depth

    void prepare (double fs)
    {
        sampleRate = fs;
        baseDelay = std::round (kBaseDelaySec * fs);
        wowMax = kWowMaxSec * fs;
        flutterMax = kFlutterMaxSec * fs;

        int needed = (int) (baseDelay + wowMax + flutterMax) + 8;
        size = 1;
        while (size < needed) size <<= 1;
        mask = size - 1;
        for (auto& b : buf) b.assign ((size_t) size, 0.0f);

        wowAmt.prepare (fs, 0.15, 0.0f);
        flutAmt.prepare (fs, 0.15, 0.0f);
        driftCoeff = (float) (1.0 - std::exp (-1.0 / (0.35 * fs)));
        flNoiseCoeff = (float) (1.0 - std::exp (-kTwoPi * 18.0 / fs));
        reset();
    }

    void reset()
    {
        for (auto& b : buf) std::fill (b.begin(), b.end(), 0.0f);
        writeIdx = 0;
        wowPhase = flPhase1 = flPhase2 = 0.0;
        drift = driftTarget = flNoise = 0.0f;
        driftCounter = 0;
    }

    int getLatencySamples() const noexcept { return (int) baseDelay; }

    void setParams (float wow01, float flutter01)
    {
        wowAmt.setTarget (wow01);
        flutAmt.setTarget (flutter01);
    }

    void process (float* const* ch, int numCh, int n) noexcept
    {
        numCh = std::min (numCh, kMaxCh);
        const double wowInc = kTwoPi * 0.55 / sampleRate;
        const double fl1Inc = kTwoPi * 6.5 / sampleRate;
        const double fl2Inc = kTwoPi * 11.3 / sampleRate;

        for (int i = 0; i < n; ++i)
        {
            // --- modulation sources
            if (--driftCounter <= 0)
            {
                driftTarget = rng.next();
                driftCounter = (int) (sampleRate * (0.6 + 0.9 * (0.5 + 0.5 * rng.next())));
            }
            drift += (driftTarget - drift) * driftCoeff;
            flNoise += (rng.next() - flNoise) * flNoiseCoeff;

            wowPhase += wowInc;  if (wowPhase > kTwoPi) wowPhase -= kTwoPi;
            flPhase1 += fl1Inc;  if (flPhase1 > kTwoPi) flPhase1 -= kTwoPi;
            flPhase2 += fl2Inc;  if (flPhase2 > kTwoPi) flPhase2 -= kTwoPi;

            const double wowSig = 0.6 * std::sin (wowPhase) + 0.4 * drift;
            const double flSig  = 0.6 * std::sin (flPhase1) + 0.25 * std::sin (flPhase2) + 0.6 * flNoise;

            double d = baseDelay + wowAmt.next() * wowMax * wowSig + flutAmt.next() * flutterMax * flSig;
            d = std::clamp (d, 2.0, (double) size - 4.0);

            // --- write, then read with Hermite interpolation
            const double rp = (double) writeIdx - d;
            const double fl = std::floor (rp);
            const int i0 = (int) fl;
            const float f = (float) (rp - fl);

            for (int c = 0; c < numCh; ++c)
            {
                auto& b = buf[(size_t) c];
                b[(size_t) writeIdx] = ch[c][i];
                const float xm1 = b[(size_t) ((i0 - 1) & mask)];
                const float x0  = b[(size_t) ((i0)     & mask)];
                const float x1  = b[(size_t) ((i0 + 1) & mask)];
                const float x2  = b[(size_t) ((i0 + 2) & mask)];
                const float c1 = 0.5f * (x1 - xm1);
                const float c2 = xm1 - 2.5f * x0 + 2.0f * x1 - 0.5f * x2;
                const float c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
                ch[c][i] = ((c3 * f + c2) * f + c1) * f + x0;
            }
            writeIdx = (writeIdx + 1) & mask;
        }
    }

private:
    double sampleRate = 44100.0, baseDelay = 132.0, wowMax = 0.0, flutterMax = 0.0;
    double wowPhase = 0.0, flPhase1 = 0.0, flPhase2 = 0.0;
    std::vector<float> buf[kMaxCh];
    int size = 1, mask = 0, writeIdx = 0, driftCounter = 0;
    float drift = 0.0f, driftTarget = 0.0f, driftCoeff = 0.0f, flNoise = 0.0f, flNoiseCoeff = 0.0f;
    Smoother wowAmt, flutAmt;
    Rng rng { 0x1234567u };
};
} // namespace md
