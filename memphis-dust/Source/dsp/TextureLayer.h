#pragma once
// Master-bus texture layer: cassette hiss + vinyl dust/crackle, mixed the way you'd
// blend a noise loop on a separate track without washing out the mix:
//
//  1. Filtered into a pocket: 12 dB/oct low cut at 350 Hz (keeps it off the kick/808/bass)
//     and a 12 dB/oct "Tone" high cut (4-12 kHz, real hiss and dust are dark).
//  2. Ducked by the program: fast attack (3 ms), ~150 ms release, so it dips 2-4 dB on
//     every kick/snare and breathes back in the gaps.
//  3. Narrowed: mid/side width (default 30 %) anchors it behind the centre of the mix.
//  4. Blended low: Hiss is calibrated in RMS dBFS, Dust in peak dBFS of the loudest pops.
//
// Dust = sparse stereo-independent clicks with a heavy-tailed amplitude distribution
// (mostly tiny ticks, occasional pops) plus rare longer, softer "thumps".

#include "DspUtils.h"

namespace md
{
class TextureLayer
{
public:
    static constexpr int kMaxCh = 2;

    void prepare (double fs)
    {
        sampleRate = fs;
        const auto hp = BiquadCoeffs::highpass (fs, 350.0, 0.70710678);
        for (int c = 0; c < kMaxCh; ++c) { hpf[c].c = hp; }
        lastTone = -1.0f;
        hissGain.prepare (fs, 0.05, 0.0f);
        dustGain.prepare (fs, 0.05, 0.0f);
        width.prepare (fs, 0.05, 0.3f);
        duckAtk = (float) std::exp (-1.0 / (0.003 * fs));
        duckRel = (float) std::exp (-1.0 / (0.15 * fs));
        reset();
    }

    void reset()
    {
        for (int c = 0; c < kMaxCh; ++c)
        {
            for (auto& s : pink[c]) s = 0.0f;
            hpf[c].reset(); lpf[c].reset();
            clickEnv[c] = 0.0f; clickLp[c] = 0.0f;
        }
        duckEnv = 0.0f;
    }

    void setParams (float hissDb, float dustDb, float density01, float toneHz, float width01, float duckDb)
    {
        hissGain.setTarget (hissDb <= -89.9f ? 0.0f : dbToGain (hissDb));
        dustGain.setTarget (dustDb <= -89.9f ? 0.0f : dbToGain (dustDb));
        width.setTarget (std::clamp (width01, 0.0f, 1.0f));
        duckDepthDb = std::max (0.0f, duckDb);
        // 2 .. ~250 crackles per second, perceptually spread
        eventProb = (float) ((2.0 * std::pow (125.0, (double) std::clamp (density01, 0.0f, 1.0f))) / sampleRate);
        if (std::abs (toneHz - lastTone) > 1.0e-3f)
        {
            lastTone = toneHz;
            const auto lp = BiquadCoeffs::lowpass (sampleRate, toneHz, 0.70710678);
            for (int c = 0; c < kMaxCh; ++c) lpf[c].c = lp;
        }
    }

    void process (float* const* ch, int numCh, int n) noexcept
    {
        numCh = std::min (numCh, kMaxCh);
        for (int i = 0; i < n; ++i)
        {
            // --- program-keyed ducking
            float key = 0.0f;
            for (int c = 0; c < numCh; ++c) key = std::max (key, std::abs (ch[c][i]));
            duckEnv = key + (key > duckEnv ? duckAtk : duckRel) * (duckEnv - key);
            const float duck = dbToGain (-duckDepthDb * std::clamp ((gainToDb (duckEnv) + 30.0f) / 24.0f, 0.0f, 1.0f));

            const float hg = hissGain.next(), dg = dustGain.next(), w = width.next();
            float tex[kMaxCh] {};

            for (int c = 0; c < numCh; ++c)
            {
                float v = 0.0f;
                if (hg > 0.0f) v += pinkSample (c) * hg * kHissCal;

                if (dg > 0.0f)
                {
                    if (0.5f + 0.5f * rng[c].next() < eventProb) triggerClick (c);
                    if (clickEnv[c] > 1.0e-5f)
                    {
                        const float burst = clickEnv[c] * rng[c].next();
                        clickLp[c] += clickLpCoeff[c] * (burst - clickLp[c]);  // thumps are low-passed
                        v += clickLp[c] * dg;
                        clickEnv[c] *= clickDecay[c];
                    }
                }
                tex[c] = lpf[c].process (hpf[c].process (v)) * duck;
            }

            if (numCh == 2)
            {
                const float m = 0.5f * (tex[0] + tex[1]);
                const float s = 0.5f * (tex[0] - tex[1]) * w;
                ch[0][i] += m + s;
                ch[1][i] += m - s;
            }
            else
                ch[0][i] += tex[0];
        }
    }

private:
    inline float pinkSample (int c) noexcept
    {
        auto& b = pink[c];
        const float w = rng[c].next();
        b[0] = 0.99886f * b[0] + w * 0.0555179f;
        b[1] = 0.99332f * b[1] + w * 0.0750759f;
        b[2] = 0.96900f * b[2] + w * 0.1538520f;
        b[3] = 0.86650f * b[3] + w * 0.3104856f;
        b[4] = 0.55000f * b[4] + w * 0.5329522f;
        b[5] = -0.7616f * b[5] - w * 0.0168980f;
        const float p = (b[0] + b[1] + b[2] + b[3] + b[4] + b[5] + b[6] + w * 0.5362f) * 0.11f;
        b[6] = w * 0.115926f;
        return p;
    }

    void triggerClick (int c) noexcept
    {
        const float u = 0.5f + 0.5f * rng[c].next();
        const bool thump = u > 0.97f;                     // ~3 % are softer, longer pops
        const float a = 0.5f + 0.5f * rng[c].next();
        clickEnv[c] = std::copysign (a * a * a * a, rng[c].next()) * (thump ? 0.7f : 1.0f) * 1.8f;
        const double len = thump ? 0.0015 : 0.00012 + 0.0004 * (0.5 + 0.5 * rng[c].next());
        clickDecay[c] = (float) std::exp (-1.0 / (len * sampleRate));
        clickLpCoeff[c] = thump ? (float) (1.0 - std::exp (-kTwoPi * 1500.0 / sampleRate)) : 1.0f;
    }

    static constexpr float kHissCal = 8.85f; // pink -> 350 Hz..9 kHz band, calibrated to RMS dBFS

    double sampleRate = 44100.0;
    float pink[kMaxCh][7] {};
    Biquad hpf[kMaxCh], lpf[kMaxCh];
    float lastTone = -1.0f, eventProb = 0.0f, duckDepthDb = 3.0f;
    float clickEnv[kMaxCh] {}, clickDecay[kMaxCh] {}, clickLp[kMaxCh] {}, clickLpCoeff[kMaxCh] { 1.0f, 1.0f };
    float duckEnv = 0.0f, duckAtk = 0.0f, duckRel = 0.0f;
    Smoother hissGain, dustGain, width;
    Rng rng[kMaxCh] { Rng (0x2545F491u), Rng (0x9E3779B1u) };
};
} // namespace md
