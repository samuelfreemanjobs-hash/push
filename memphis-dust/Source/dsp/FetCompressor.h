#pragma once
// STEP 4 — FET-style bus compressor (1176-flavoured).
// * Stereo-linked peak detector, log-domain gain computer with soft knee.
// * Microsecond-range attack lets the detector follow the waveform -> the classic FET grit.
// * Program-dependent release: the deeper the gain reduction, the slower it recovers,
//   which is what makes the bus "breathe" and pump the hiss.
// * "ALL" mode emulates all-buttons-in: near-limiting ratio, overshoot and extra colour.
// * Gain-reduction-dependent FET colour (odd + a touch of even harmonics).

#include "DspUtils.h"
#include <atomic>

namespace md
{
class FetCompressor
{
public:
    static constexpr int kMaxCh = 2;

    void prepare (double fs) { sampleRate = fs; makeup.prepare (fs, 0.03, 1.0f); mix.prepare (fs, 0.03, 1.0f); reset(); }
    void reset() { envDb = 0.0f; }

    // ratioIndex: 0=4, 1=8, 2=12, 3=20, 4=ALL
    void setParams (float thresholdDb, int ratioIndex, float attackMs, float releaseMs, float makeupDb, float mix01)
    {
        threshold = thresholdDb;
        static constexpr float ratios[] = { 4.0f, 8.0f, 12.0f, 20.0f, 40.0f };
        ratioIndex = std::clamp (ratioIndex, 0, 4);
        allButtons = ratioIndex == 4;
        slope = 1.0f / ratios[ratioIndex] - 1.0f;
        knee = allButtons ? 2.0f : 6.0f;
        atkCoeff = (float) std::exp (-1.0 / (std::max (attackMs, 0.005f) * 0.001 * sampleRate));
        relMs = std::max (releaseMs, 5.0f);
        makeup.setTarget (dbToGain (makeupDb));
        mix.setTarget (mix01);
    }

    float getAndResetPeakGr() noexcept { return peakGr.exchange (0.0f); } // returns <= 0 dB

    void process (float* const* ch, int numCh, int n) noexcept
    {
        numCh = std::min (numCh, kMaxCh);
        float blockMinGr = 0.0f;
        const float overshoot = allButtons ? 1.25f : 1.0f;
        const float colourMax = allButtons ? 0.7f : 0.35f;

        for (int i = 0; i < n; ++i)
        {
            float peak = 0.0f;
            for (int c = 0; c < numCh; ++c) peak = std::max (peak, std::abs (ch[c][i]));

            const float target = computeGr (gainToDb (peak)) * overshoot;

            if (target < envDb)
                envDb = target + atkCoeff * (envDb - target);
            else
            {
                // program-dependent release: up to 3x slower when slammed
                const float depth = std::clamp (-envDb / 20.0f, 0.0f, 1.0f);
                const float relSec = relMs * 0.001f * (1.0f + 2.0f * depth);
                const float rc = std::exp (-1.0f / (relSec * (float) sampleRate));
                envDb = target + rc * (envDb - target);
            }
            blockMinGr = std::min (blockMinGr, envDb);

            const float g = dbToGain (envDb) * makeup.next();
            const float m = mix.next();
            const float colour = std::clamp (-envDb / 24.0f, 0.0f, 1.0f) * colourMax;

            for (int c = 0; c < numCh; ++c)
            {
                const float dry = ch[c][i];
                float y = dry * g;
                const float sat = std::tanh (y * 1.4f + 0.08f) - 0.0798298f; // tanh(0.08)
                y += colour * (sat / 1.4f - y);
                ch[c][i] = dry + m * (y - dry);
            }
        }

        // keep the most negative value until the UI reads it
        float prev = peakGr.load();
        while (blockMinGr < prev && ! peakGr.compare_exchange_weak (prev, blockMinGr)) {}
    }

private:
    inline float computeGr (float xDb) const noexcept
    {
        const float over = xDb - threshold;
        if (2.0f * over < -knee) return 0.0f;
        if (2.0f * std::abs (over) <= knee)
        {
            const float t = over + knee * 0.5f;
            return slope * t * t / (2.0f * knee);
        }
        return slope * over;
    }

    double sampleRate = 44100.0;
    float threshold = -18.0f, slope = -0.875f, knee = 6.0f, atkCoeff = 0.0f, relMs = 60.0f;
    float envDb = 0.0f;
    bool allButtons = false;
    Smoother makeup, mix;
    std::atomic<float> peakGr { 0.0f };
};
} // namespace md
