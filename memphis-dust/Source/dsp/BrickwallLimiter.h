#pragma once
// Lookahead brickwall limiter (the last thing on an underground tape master).
//
//   target gain g[n] = min(1, ceiling / peak[n])         (stereo-linked)
//   held[n]   = min(g[n-L+1 .. n])                       (sliding minimum, L = lookahead)
//   rel[n]    = instant attack / exponential release of held
//   smooth[n] = mean(rel[n-L+1 .. n])                    (box filter -> click-free attack)
//   y[n]      = x[n-L+1] * smooth[n]
//
// Every sample of the box window "sees" the peak, so the output never exceeds the
// ceiling, and the gain is fully ramped down by the time the peak comes out.
// The delay line always runs, so the reported latency is constant even when bypassed.

#include "DspUtils.h"
#include <atomic>
#include <vector>

namespace md
{
class BrickwallLimiter
{
public:
    static constexpr int kMaxCh = 2;
    static constexpr double kLookaheadSec = 0.0015;

    void prepare (double fs)
    {
        sampleRate = fs;
        L = std::max (2, (int) std::round (kLookaheadSec * fs));
        for (auto& d : delay) d.assign ((size_t) L, 0.0f);
        gainHist.assign ((size_t) L, 1.0f);
        relHist.assign ((size_t) L, 1.0f);
        relCoeff = (float) std::exp (-1.0 / (0.08 * fs));
        drive.prepare (fs, 0.03, 1.0f);
        reset();
    }

    void reset()
    {
        for (auto& d : delay) std::fill (d.begin(), d.end(), 0.0f);
        std::fill (gainHist.begin(), gainHist.end(), 1.0f);
        std::fill (relHist.begin(), relHist.end(), 1.0f);
        boxSum = (double) L;
        relState = 1.0f;
        pos = 0;
    }

    int getLatencySamples() const noexcept { return L - 1; }

    void setParams (bool on, float driveDb, float ceilingDb)
    {
        enabled = on;
        drive.setTarget (on ? dbToGain (driveDb) : 1.0f);
        ceiling = dbToGain (ceilingDb);
    }

    float getAndResetPeakGr() noexcept { return peakGr.exchange (0.0f); }

    void process (float* const* ch, int numCh, int n) noexcept
    {
        numCh = std::min (numCh, kMaxCh);
        float minGain = 1.0f;

        for (int i = 0; i < n; ++i)
        {
            const float gIn = drive.next();
            float peak = 0.0f;
            for (int c = 0; c < numCh; ++c)
            {
                const float x = ch[c][i] * gIn;
                delay[c][(size_t) pos] = x;
                peak = std::max (peak, std::abs (x));
            }

            // Gain path always runs (targets are 1 when bypassed) so on/off transitions ramp instead of clicking.
            gainHist[(size_t) pos] = (enabled && peak > ceiling) ? ceiling / peak : 1.0f;
            float held = 1.0f;
            for (float g : gainHist) held = std::min (held, g);

            relState = held < relState ? held : held + relCoeff * (relState - held);

            boxSum += (double) relState - (double) relHist[(size_t) pos];
            relHist[(size_t) pos] = relState;
            const float smooth = (float) std::min (1.0, boxSum / (double) L);
            minGain = std::min (minGain, smooth);

            const int readPos = (pos + 1) % L; // oldest sample in the ring = x[n - (L-1)]
            for (int c = 0; c < numCh; ++c)
            {
                float y = delay[c][(size_t) readPos] * smooth;
                if (enabled) y = std::clamp (y, -ceiling, ceiling); // guard against float rounding
                ch[c][i] = y;
            }
            pos = (pos + 1) % L;
        }

        const float grDb = gainToDb (minGain);
        float prev = peakGr.load();
        while (grDb < prev && ! peakGr.compare_exchange_weak (prev, grDb)) {}
    }

private:
    double sampleRate = 44100.0;
    int L = 64, pos = 0;
    bool enabled = false;
    float ceiling = 1.0f, relCoeff = 0.999f, relState = 1.0f;
    double boxSum = 0.0;
    Smoother drive;
    std::vector<float> delay[kMaxCh], gainHist, relHist;
    std::atomic<float> peakGr { 0.0f };
};
} // namespace md
