#pragma once
// STEP 3c — Cassette hiss: per-channel pink noise (Paul Kellet's filter), band-limited
// to the hiss region. Added BEFORE the compressor so the FET stage pumps it.

#include "DspUtils.h"

namespace md
{
class TapeHiss
{
public:
    static constexpr int kMaxCh = 2;

    void prepare (double fs)
    {
        hpCoeff = (float) std::exp (-kTwoPi * 600.0 / fs);
        const auto lp = BiquadCoeffs::lowpass (fs, 12000.0, 0.70710678);
        for (int c = 0; c < kMaxCh; ++c) lpf[c].c = lp;
        level.prepare (fs, 0.05, 0.0f);
        duckAtk = (float) std::exp (-1.0 / (0.001 * fs));
        duckRel = (float) std::exp (-1.0 / (0.18 * fs));
        reset();
    }

    void reset()
    {
        for (int c = 0; c < kMaxCh; ++c)
        {
            for (auto& s : st[c]) s = 0.0f;
            hpX[c] = hpY[c] = 0.0f;
            lpf[c].reset();
        }
        duckEnv = 0.0f;
    }

    // 0 = constant hiss, 1 = up to ~18 dB of ducking on loud hits (breathes in the gaps)
    void setDuck (float amount01) { duck = std::clamp (amount01, 0.0f, 1.0f); }

    void setLevelDb (float db) { level.setTarget (db <= -89.9f ? 0.0f : dbToGain (db)); }

    void process (float* const* ch, int numCh, int n) noexcept
    {
        numCh = std::min (numCh, kMaxCh);
        for (int i = 0; i < n; ++i)
        {
            float key = 0.0f;
            for (int c = 0; c < numCh; ++c) key = std::max (key, std::abs (ch[c][i]));
            duckEnv = key + (key > duckEnv ? duckAtk : duckRel) * (duckEnv - key);
            const float duckDb = -18.0f * duck * std::clamp ((gainToDb (duckEnv) + 40.0f) / 30.0f, 0.0f, 1.0f);
            const float g = level.next() * (duck > 0.0f ? dbToGain (duckDb) : 1.0f);
            for (int c = 0; c < numCh; ++c)
            {
                auto& b = st[c];
                const float w = rng[c].next();
                b[0] = 0.99886f * b[0] + w * 0.0555179f;
                b[1] = 0.99332f * b[1] + w * 0.0750759f;
                b[2] = 0.96900f * b[2] + w * 0.1538520f;
                b[3] = 0.86650f * b[3] + w * 0.3104856f;
                b[4] = 0.55000f * b[4] + w * 0.5329522f;
                b[5] = -0.7616f * b[5] - w * 0.0168980f;
                float pink = (b[0] + b[1] + b[2] + b[3] + b[4] + b[5] + b[6] + w * 0.5362f) * 0.11f;
                b[6] = w * 0.115926f;

                // 1-pole high-pass (hiss lives up top)
                const float hp = hpCoeff * (hpY[c] + pink - hpX[c]);
                hpX[c] = pink; hpY[c] = hp;

                ch[c][i] += lpf[c].process (hp) * g * kCal; // calibrated: setting == RMS dBFS
            }
        }
    }

private:
    static constexpr float kCal = 9.45f;
    float st[kMaxCh][7] {};
    float hpX[kMaxCh] {}, hpY[kMaxCh] {}, hpCoeff = 0.9f;
    Biquad lpf[kMaxCh];
    Rng rng[kMaxCh] { Rng (0xA341316Cu), Rng (0xC8013EA4u) };
    Smoother level;
    float duck = 0.0f, duckEnv = 0.0f, duckAtk = 0.0f, duckRel = 0.0f;
};
} // namespace md
