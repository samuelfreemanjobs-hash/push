#pragma once
// dbx Type II noise reduction (Tascam Portastudio 424 / 414 style), as a real compander.
//
//   encode():  pre-emphasis -> 2:1 wide-band RMS compressor   (before the tape)
//   ... tape saturation, wow/flutter, dropouts and hiss happen here ...
//   decode():  1:2 wide-band RMS expander -> de-emphasis       (after the tape)
//
// Both detectors use the same HF-weighted sidechain (as dbx II does), so with Breath = 0
// encode->decode is transparent in steady state. Everything the tape adds in between
// (hiss, saturation, dropouts) gets expanded, which gives the classic noise-floor
// "breathing" and HF swish on cymbal tails.
//
// Breath = deliberate decoder mistracking: steeper expansion slope + faster decoder release
//          -> transients jump out, tails and hiss duck hard, the whole bus pumps.
// Tilt   = dynamic de-emphasis mismatch: highs brighten on loud hits and darken as they decay.

#include "DspUtils.h"

namespace md
{
class DbxNR
{
public:
    static constexpr int kMaxCh = 2;
    static constexpr float kRefDb = -20.0f;

    void prepare (double fs)
    {
        sampleRate = fs;
        const auto pre  = BiquadCoeffs::highShelf (fs, 2000.0, +8.0);
        const auto post = BiquadCoeffs::highShelf (fs, 2000.0, -8.0);
        const auto scShelf = BiquadCoeffs::highShelf (fs, 3000.0, +12.0);
        const auto scHp = BiquadCoeffs::highpass (fs, 120.0, 0.70710678);
        for (int c = 0; c < kMaxCh; ++c)
        {
            preEmph[c].c = pre; deEmph[c].c = post;
            encSc1[c].c = decSc1[c].c = scHp;
            encSc2[c].c = decSc2[c].c = scShelf;
        }
        atkCoeff = coeff (0.002);
        tiltHpCoeff = (float) std::exp (-kTwoPi * 4000.0 / fs);
        breath.prepare (fs, 0.05, 0.0f);
        tilt.prepare (fs, 0.05, 0.0f);
        reset();
    }

    void reset()
    {
        for (int c = 0; c < kMaxCh; ++c)
        {
            preEmph[c].reset(); deEmph[c].reset();
            encSc1[c].reset(); encSc2[c].reset(); decSc1[c].reset(); decSc2[c].reset();
            encPow[c] = decPow[c] = 1.0e-4f;   // start at the reference level, no startup thump
            tiltX[c] = tiltY[c] = 0.0f;
            lastDecGainDb[c] = 0.0f;
        }
    }

    void setParams (float breath01, float releaseMs, float tilt01)
    {
        breath.setTarget (breath01);
        tilt.setTarget (tilt01);
        encRelease = coeff (releaseMs * 0.001);
        relSec = releaseMs * 0.001f;
    }

    void encode (float* const* ch, int numCh, int n) noexcept
    {
        numCh = std::min (numCh, kMaxCh);
        for (int c = 0; c < numCh; ++c)
        {
            float* d = ch[c];
            for (int i = 0; i < n; ++i)
            {
                const float x = preEmph[c].process (d[i]);
                const float level = detect (encPow[c], encSc2[c].process (encSc1[c].process (x)), encRelease);
                const float gDb = std::clamp (-0.5f * (level - kRefDb), -20.0f, 20.0f);
                d[i] = x * dbToGain (gDb);
            }
        }
    }

    void decode (float* const* ch, int numCh, int n) noexcept
    {
        numCh = std::min (numCh, kMaxCh);
        for (int i = 0; i < n; ++i)
        {
            const float b = breath.next();
            const float t = tilt.next();
            const float slope = 1.0f + 0.8f * b;                                     // 1:2 -> ~1:2.8
            const float decRel = coeff (relSec * (1.0f - 0.65f * b));                // decoder recovers faster
            for (int c = 0; c < numCh; ++c)
            {
                const float x = ch[c][i];
                const float level = detect (decPow[c], decSc2[c].process (decSc1[c].process (x)), decRel);
                const float gDb = std::clamp (slope * (level - kRefDb), -60.0f, 24.0f);
                lastDecGainDb[c] = gDb;
                float y = deEmph[c].process (x * dbToGain (gDb));

                if (t > 0.0f)
                {
                    // dynamic high shelf (~4 kHz+): brighter while the expander is open, darker as it closes
                    const float hp = tiltHpCoeff * (tiltY[c] + y - tiltX[c]);
                    tiltX[c] = y; tiltY[c] = hp;
                    const float tiltDb = t * std::clamp (gDb * 0.3f, -6.0f, 3.0f);
                    y += hp * (dbToGain (tiltDb) - 1.0f);
                }
                ch[c][i] = y;
            }
        }
    }

private:
    float coeff (double sec) const { return (float) std::exp (-1.0 / (std::max (sec, 1.0e-4) * sampleRate)); }

    inline float detect (float& pow, float x, float relCoeff) const noexcept
    {
        const float sq = x * x;
        const float c = sq > pow ? atkCoeff : relCoeff;
        pow = sq + c * (pow - sq);
        return 10.0f * std::log10 (pow + 1.0e-12f) + 3.0103f; // RMS, sine-peak referenced
    }

    double sampleRate = 44100.0;
    float atkCoeff = 0.0f, encRelease = 0.0f, relSec = 0.12f, tiltHpCoeff = 0.5f;
    Smoother breath, tilt;
    Biquad preEmph[kMaxCh], deEmph[kMaxCh], encSc1[kMaxCh], encSc2[kMaxCh], decSc1[kMaxCh], decSc2[kMaxCh];
    float encPow[kMaxCh] {}, decPow[kMaxCh] {}, tiltX[kMaxCh] {}, tiltY[kMaxCh] {}, lastDecGainDb[kMaxCh] {};
};
} // namespace md
