// Numerical sanity tests for every DSP stage. No JUCE required.
#include "dsp/SamplerCrusher.h"
#include "dsp/CassetteEQ.h"
#include "dsp/TapeSaturator.h"
#include "dsp/WowFlutter.h"
#include "dsp/TapeHiss.h"
#include "dsp/SubClipper.h"
#include "dsp/FetCompressor.h"

#include <cstdio>
#include <set>
#include <vector>

using namespace md;
static int failures = 0;
#define CHECK(cond, ...) do { bool ok_ = (cond); std::printf ("[%s] ", ok_ ? "PASS" : "FAIL"); std::printf (__VA_ARGS__); std::printf ("\n"); if (! ok_) ++failures; } while (0)

constexpr double fs = 48000.0;

static std::vector<float> sine (double f, float amp, int n)
{
    std::vector<float> v ((size_t) n);
    for (int i = 0; i < n; ++i) v[(size_t) i] = amp * (float) std::sin (kTwoPi * f * i / fs);
    return v;
}
static float rms (const std::vector<float>& v, int from = 0)
{
    double s = 0; for (size_t i = (size_t) from; i < v.size(); ++i) s += (double) v[i] * v[i];
    return (float) std::sqrt (s / (double) (v.size() - (size_t) from));
}
static bool finite (const std::vector<float>& v) { for (float x : v) if (! std::isfinite (x)) return false; return true; }
template <typename P> static void run (P& proc, std::vector<float>& a, std::vector<float>& b, int block = 256)
{
    for (size_t pos = 0; pos < a.size(); pos += (size_t) block)
    {
        const int n = (int) std::min ((size_t) block, a.size() - pos);
        float* ch[2] = { a.data() + pos, b.data() + pos };
        proc.process (ch, 2, n);
    }
}

int main()
{
    const int N = (int) fs * 2;

    { // 01 sampler
        SamplerCrusher c; c.prepare (fs); c.setParams (12000.0f, 4.0f, false);
        auto a = sine (440, 0.9f, N), b = a; run (c, a, b);
        std::set<float> levels (a.begin(), a.end());
        CHECK (levels.size() <= 16, "4-bit crusher produces %zu distinct levels (<=16)", levels.size());
        int changes = 0; for (int i = 1; i < N; ++i) if (a[(size_t) i] != a[(size_t) i - 1]) ++changes;
        CHECK (changes <= 12000 * 2 + 2, "12 kHz hold changes value <= 12k times/sec (%d in 2s)", changes);
    }
    { // 02 EQ
        CassetteEQ eq; eq.prepare (fs); eq.setParams (30, 300, 4, 2, 11000);
        auto a = sine (16000, 0.5f, N), b = a; run (eq, a, b);
        const float att = gainToDb (rms (a, 4800) / (0.5f / std::sqrt (2.0f)));
        CHECK (att < -15.0f, "16 kHz attenuated by high cut: %.1f dB", att);
        CassetteEQ eq2; eq2.prepare (fs); eq2.setParams (30, 300, 4, 2, 11000);
        auto c = sine (300, 0.5f, N), d = c; run (eq2, c, d);
        const float box = gainToDb (rms (c, 4800) / (0.5f / std::sqrt (2.0f)));
        CHECK (std::abs (box - 4.0f) < 0.3f, "Box bell at 300 Hz = %.2f dB (expect +4)", box);
    }
    { // shelf inverse identity
        Biquad p, q; p.c = BiquadCoeffs::highShelf (96000, 3000, 6); q.c = BiquadCoeffs::highShelf (96000, 3000, -6);
        float err = 0; Rng r;
        for (int i = 0; i < 10000; ++i) { float x = r.next(); err = std::max (err, std::abs (q.process (p.process (x)) - x)); }
        CHECK (err < 1e-5f, "pre/de-emphasis is an exact inverse (max err %.2e)", err);
    }
    { // 03 tape saturation
        TapeSaturator t; t.prepare (fs * 2); t.setParams (0.0f, 0.0f);
        auto a = sine (1000, 0.05f, N), b = a, ref = a; run (t, a, b);
        const float g = gainToDb (rms (a, 9600) / rms (ref, 9600));
        CHECK (std::abs (g) < 0.5f, "tape drive 0 dB small-signal gain = %.2f dB", g);
        TapeSaturator t2; t2.prepare (fs * 2); t2.setParams (24.0f, 1.0f);
        auto c = sine (60, 1.0f, N), d = c; run (t2, c, d);
        float pk = 0; for (float x : c) pk = std::max (pk, std::abs (x));
        CHECK (finite (c) && pk < 2.0f, "tape drive 24 dB / bias 100%% bounded (peak %.2f)", pk);
    }
    { // wow/flutter zero modulation = pure delay
        WowFlutter w; w.prepare (fs); w.setParams (0, 0);
        std::vector<float> a ((size_t) N, 0.0f), b = a; a[1000] = 1.0f; b[1000] = 1.0f; run (w, a, b);
        const size_t expect = 1000 + (size_t) w.getLatencySamples();
        CHECK (std::abs (a[expect] - 1.0f) < 1e-6f, "W/F at 0%% is a clean %d-sample delay", w.getLatencySamples());
        WowFlutter w2; w2.prepare (fs); w2.setParams (1, 1);
        auto c = sine (1000, 0.5f, N), d = c, ref = c; run (w2, c, d);
        CHECK (finite (c) && std::abs (rms (c, 9600) - rms (ref, 9600)) < 0.01f, "W/F at 100%% keeps level (%.3f vs %.3f)", rms (c, 9600), rms (ref, 9600));
    }
    { // hiss calibration
        TapeHiss h; h.prepare (fs); h.setLevelDb (-60);
        std::vector<float> a ((size_t) N, 0.0f), b = a; run (h, a, b);
        const float lvl = gainToDb (rms (a, 4800));
        CHECK (std::abs (lvl + 60.0f) < 4.0f, "hiss at -60 dB setting measures %.1f dBFS RMS", lvl);
        TapeHiss h2; h2.prepare (fs); h2.setLevelDb (-90);
        std::vector<float> c ((size_t) N, 0.0f), d = c; run (h2, c, d);
        CHECK (rms (c, 4800) == 0.0f, "hiss at -90 is fully off");
    }
    { // 05 sub clipper
        SubClipper s; s.prepare (fs); s.setParams (120, 0, 0, 0);
        auto a = sine (2000, 0.3f, N), b = a, ref = a; run (s, a, b);
        const float g = gainToDb (rms (a, 4800) / rms (ref, 4800));
        CHECK (std::abs (g) < 0.2f, "808 clip passes highs untouched (%.2f dB at 2 kHz)", g);
        SubClipper s2; s2.prepare (fs); s2.setParams (120, 24, 1, 0);
        auto c = sine (50, 0.8f, N), d = sine (50, -0.8f, N); run (s2, c, d);
        std::vector<float> sum (c.size()); for (size_t i = 0; i < c.size(); ++i) sum[i] = c[i] + d[i];
        const float diff = rms (sum, 4800);
        CHECK (finite (c) && diff < 1e-6f, "808 clip low band is mono (L+R antiphase sub cancels: %.1e)", diff);
    }
    { // 04 FET comp
        FetCompressor f; f.prepare (fs); f.setParams (-18, 1, 0.1f, 60, 0, 1);
        auto a = sine (100, 0.5f, N), b = a, ref = a; run (f, a, b);
        const float gr = gainToDb (rms (a, N / 2) / rms (ref, N / 2));
        CHECK (gr < -8.0f && gr > -12.0f, "comp 8:1 @ -18 dB on -6 dBFS sine: %.1f dB GR (expect ~-10.5)", gr);
        CHECK (f.getAndResetPeakGr() < -8.0f, "GR meter reports reduction");
    }

    std::printf ("\n%s (%d failure%s)\n", failures ? "FAILED" : "ALL PASSED", failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
