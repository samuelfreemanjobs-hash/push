// Numerical sanity tests for every DSP stage. No JUCE required.
#include "dsp/SamplerCrusher.h"
#include "dsp/CassetteEQ.h"
#include "dsp/TapeSaturator.h"
#include "dsp/WowFlutter.h"
#include "dsp/TapeHiss.h"
#include "dsp/SubClipper.h"
#include "dsp/FetCompressor.h"
#include "dsp/DbxNR.h"
#include "dsp/TapeDropouts.h"
#include "dsp/MonoBass.h"
#include "dsp/BrickwallLimiter.h"
#include "dsp/TextureLayer.h"

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

    { // 04 dbx II: encode->decode transparent at Breath 0, compresses 2:1 in between
        for (float amp : { 0.5f, 0.05f, 0.005f })
        {
            DbxNR d; d.prepare (fs); d.setParams (0, 120, 0);
            auto a = sine (1000, amp, N), b = a, ref = a;
            float* ch[2] = { a.data(), b.data() };
            for (int pos = 0; pos < N; pos += 256) { float* c2[2] = { ch[0] + pos, ch[1] + pos }; d.encode (c2, 2, std::min (256, N - pos)); d.decode (c2, 2, std::min (256, N - pos)); }
            const float err = gainToDb (rms (a, N / 2) / rms (ref, N / 2));
            CHECK (std::abs (err) < 0.5f, "dbx encode->decode transparent at %.0f dBFS: %.2f dB", gainToDb (amp), err);
        }
        DbxNR e; e.prepare (fs); e.setParams (0, 120, 0);
        auto lo = sine (1000, 0.01f, N), lo2 = lo; float* c1[2] = { lo.data(), lo2.data() }; e.encode (c1, 2, N);
        DbxNR e2; e2.prepare (fs); e2.setParams (0, 120, 0);
        auto hi = sine (1000, 0.5f, N), hi2 = hi; float* c2[2] = { hi.data(), hi2.data() }; e2.encode (c2, 2, N);
        const float encRange = gainToDb (rms (hi, N / 2) / rms (lo, N / 2));
        CHECK (std::abs (encRange - 17.0f) < 1.5f, "dbx encoder: 34 dB input range -> %.1f dB on tape (2:1)", encRange);

        // noise reduction + breathing: hiss injected "on tape" is suppressed in gaps
        auto runHiss = [&] (bool nr, float breath) {
            DbxNR d; d.prepare (fs); d.setParams (breath, 120, 0);
            TapeHiss h; h.prepare (fs); h.setLevelDb (-50);
            const int M = N * 2; std::vector<float> a ((size_t) M, 0.0f), b = a;
            for (int i = 0; i < M; ++i) if ((i / 48000) % 2 == 0) a[(size_t) i] = b[(size_t) i] = 0.5f * (float) std::sin (kTwoPi * 200 * i / fs) * (float) std::exp (-(i % 48000) / 1500.0);
            for (int pos = 0; pos < M; pos += 256)
            {
                const int n = std::min (256, M - pos);
                float* c[2] = { a.data() + pos, b.data() + pos };
                if (nr) d.encode (c, 2, n);
                h.process (c, 2, n);
                if (nr) d.decode (c, 2, n);
            }
            double s = 0; int cnt = 0; // measure late in the 1 s silent gaps
            for (int i = 0; i < M; ++i) if ((i / 48000) % 2 == 1 && i % 48000 > 24000) { s += a[(size_t) i] * a[(size_t) i]; ++cnt; }
            return gainToDb ((float) std::sqrt (s / cnt));
        };
        const float off = runHiss (false, 0), on = runHiss (true, 0), br = runHiss (true, 1);
        CHECK (on < off - 15.0f, "dbx suppresses hiss in gaps: %.1f -> %.1f dBFS", off, on);
        CHECK (br < on - 3.0f, "Breath (mistrack) ducks the gaps further: %.1f dBFS", br);
    }
    { // dropouts
        TapeDropouts d; d.prepare (fs); d.setAmount (1.0f);
        auto a = sine (5000, 0.5f, N * 2), b = a; float* c[2] = { a.data(), b.data() }; d.process (c, 2, N * 2);
        int dips = 0; bool inDip = false;
        for (int i = 0; i < N * 2; i += 48) { float pk = 0; for (int k = 0; k < 48 && i + k < N * 2; ++k) pk = std::max (pk, std::abs (a[(size_t) (i + k)]));
            if (pk < 0.35f && ! inDip) { ++dips; inDip = true; } else if (pk > 0.45f) inDip = false; }
        CHECK (finite (a) && dips >= 3 && dips <= 25, "dropouts at 100%% produce %d audible dips in 4 s", dips);
        TapeDropouts z; z.prepare (fs); z.setAmount (0.0f);
        auto q = sine (5000, 0.5f, N), w = q, ref = q; float* cz[2] = { q.data(), w.data() }; z.process (cz, 2, N);
        CHECK (q == ref, "dropouts at 0%% are bit-transparent");
    }
    { // hiss duck
        auto run = [&] (float duck) {
            TapeHiss h; h.prepare (fs); h.setLevelDb (-50); h.setDuck (duck);
            auto a = sine (100, 0.7f, N), b = a, ref = a; float* c[2] = { a.data(), b.data() }; h.process (c, 2, N);
            for (size_t i = 0; i < a.size(); ++i) a[i] -= ref[i];
            return gainToDb (rms (a, 9600)); };
        const float d0 = run (0), d1 = run (1);
        CHECK (d1 < d0 - 12.0f, "hiss duck under a loud signal: %.1f -> %.1f dBFS", d0, d1);
    }
    { // presence dip
        CassetteEQ eq; eq.prepare (fs); eq.setParams (20, 300, 0, 1, 20000, -1.5f);
        auto a = sine (3500, 0.5f, N), b = a; run (eq, a, b);
        const float g = gainToDb (rms (a, 4800) / (0.5f / std::sqrt (2.0f)));
        CHECK (std::abs (g + 1.5f) < 0.3f, "presence dip at 3.5 kHz = %.2f dB (expect -1.5)", g);
    }

    { // master: mono bass
        MonoBass m; m.prepare (fs); m.setFrequency (120);
        auto a = sine (50, 0.5f, N), b = sine (50, -0.5f, N); float* c[2] = { a.data(), b.data() }; m.process (c, 2, N);
        std::vector<float> side (a.size()); for (size_t i = 0; i < a.size(); ++i) side[i] = 0.5f * (a[i] - b[i]);
        const float sideDb = gainToDb (rms (side, 9600) / 0.3536f);
        CHECK (sideDb < -25.0f, "mono bass: 50 Hz stereo-difference removed (%.1f dB)", sideDb);
        MonoBass m2; m2.prepare (fs); m2.setFrequency (120);
        auto h = sine (2000, 0.5f, N), h2 = sine (2000, -0.5f, N), ref = h; float* c2[2] = { h.data(), h2.data() }; m2.process (c2, 2, N);
        CHECK (std::abs (gainToDb (rms (h, 9600) / rms (ref, 9600))) < 0.1f, "mono bass leaves 2 kHz stereo intact");
    }
    { // master: brickwall limiter
        BrickwallLimiter l; l.prepare (fs); l.setParams (true, 6.0f, -0.3f);
        Rng r (7);
        std::vector<float> a ((size_t) N), b;
        for (int i = 0; i < N; ++i) a[(size_t) i] = 0.9f * r.next() * ((i % 24000) < 2000 ? 1.0f : 0.3f); // loud bursts
        b = a; float* c[2] = { a.data(), b.data() };
        for (int pos = 0; pos < N; pos += 256) { float* cc[2] = { c[0] + pos, c[1] + pos }; l.process (cc, 2, std::min (256, N - pos)); }
        float pk = 0; for (float x : a) pk = std::max (pk, std::abs (x));
        CHECK (finite (a) && pk <= dbToGain (-0.3f) + 1e-6f, "limiter holds the -0.3 dBFS ceiling with +6 dB drive (peak %.2f dBFS)", gainToDb (pk));
        CHECK (l.getAndResetPeakGr() < -3.0f, "limiter reports gain reduction");
        BrickwallLimiter z; z.prepare (fs); z.setParams (false, 0, -0.3f);
        std::vector<float> d ((size_t) N, 0.0f), e = d; d[100] = 0.7f; float* cz[2] = { d.data(), e.data() }; z.process (cz, 2, N);
        CHECK (std::abs (d[100 + (size_t) z.getLatencySamples()] - 0.7f) < 1e-6f, "limiter bypassed = clean %d-sample delay", z.getLatencySamples());
    }
    { // VCA comp mode
        FetCompressor f; f.prepare (fs); f.setParams (-18, 0, 10, 150, 0, 1, true);
        auto a = sine (100, 0.25f, N), b = a, ref = a; run (f, a, b);
        const float gr = gainToDb (rms (a, N / 2) / rms (ref, N / 2));
        CHECK (gr < -2.0f && gr > -7.0f, "VCA 4:1 glue on a -12 dBFS sine: %.1f dB GR (3-5 dB territory)", gr);
    }

    { // sampler: pitch trick == sampling at R / 2^(st/12)
        SamplerCrusher c; c.prepare (fs); c.setParams (24000.0f, 16.0f, false, 12.0f);
        auto a = sine (440, 0.9f, N), b = a; run (c, a, b);
        int changes = 0; for (int i = 1; i < N; ++i) if (a[(size_t) i] != a[(size_t) i - 1]) ++changes;
        CHECK (changes <= 12000 * 2 + 2 && changes > 11000 * 2, "pitch trick +12 st on 24 kHz = 12 kHz effective (%d holds/2 s)", changes);
        // companded DAC: finer than linear on quiet signals, coarser on loud ones
        auto quantErr = [&] (bool comp, float amp) {
            SamplerCrusher q; q.prepare (fs); q.setParams (48000.0f, 8.0f, false, 0.0f, comp);
            auto x = sine (440, amp, N), y = x, ref = x; run (q, x, y);
            for (size_t i = 0; i < x.size(); ++i) x[i] -= ref[i];
            return gainToDb (rms (x) / rms (ref)); };
        const float lq = quantErr (false, 0.02f), cq = quantErr (true, 0.02f), ll = quantErr (false, 0.9f), cl = quantErr (true, 0.9f);
        CHECK (cq < lq - 10.0f && cl > ll, "companded DAC: quiet err %.1f vs %.1f dB (finer), loud err %.1f vs %.1f dB (grittier)", cq, lq, cl, ll);
    }
    { // texture layer
        auto silenceRun = [&] (float hissDb, float dustDb, float density, float widthAmt, std::vector<float>& L, std::vector<float>& R) {
            TextureLayer t; t.prepare (fs); t.setParams (hissDb, dustDb, density, 9000.0f, widthAmt, 3.0f);
            L.assign ((size_t) N, 0.0f); R.assign ((size_t) N, 0.0f); float* c[2] = { L.data(), R.data() }; t.process (c, 2, N); };
        std::vector<float> L, R;
        silenceRun (-50, -90, 0, 1.0f, L, R);
        const float h = gainToDb (rms (L, 4800));
        CHECK (std::abs (h + 50.0f) < 1.5f, "texture hiss at -50 dB setting measures %.1f dBFS RMS", h);
        silenceRun (-50, -90, 0, 0.3f, L, R);
        std::vector<float> mid (L.size()), side (L.size());
        for (size_t i = 0; i < L.size(); ++i) { mid[i] = 0.5f * (L[i] + R[i]); side[i] = 0.5f * (L[i] - R[i]); }
        const float ws = gainToDb (rms (side, 4800) / rms (mid, 4800));
        CHECK (std::abs (ws - gainToDb (0.3f)) < 1.0f, "texture width 30%%: side/mid = %.1f dB (expect %.1f)", ws, gainToDb (0.3f));
        silenceRun (-90, -30, 0.6f, 1.0f, L, R);
        float pk = 0; for (float x : L) pk = std::max (pk, std::abs (x));
        int clicks = 0; for (size_t i = 1; i < L.size(); ++i) if (std::abs (L[i]) > 0.003f && std::abs (L[i - 1]) <= 0.003f) ++clicks;
        CHECK (finite (L) && gainToDb (pk) < -27.0f && clicks > 10, "dust at -30 dB: peak %.1f dBFS, %d audible crackles in 2 s", gainToDb (pk), clicks);
        // low end stays clean (350 Hz HPF)
        Biquad lp; lp.c = BiquadCoeffs::lowpass (fs, 100, 0.707);
        std::vector<float> low (L.size()); for (size_t i = 0; i < L.size(); ++i) low[i] = lp.process (L[i]);
        CHECK (gainToDb (rms (low, 4800) / rms (L, 4800)) < -20.0f, "texture keeps out of the sub/kick range (<100 Hz %.1f dB down)", gainToDb (rms (low, 4800) / rms (L, 4800)));
        // ducking under a loud beat
        TextureLayer t; t.prepare (fs); t.setParams (-50, -90, 0, 9000, 1, 3);
        auto a = sine (60, 0.8f, N), b = a, ref = a; float* c[2] = { a.data(), b.data() }; t.process (c, 2, N);
        for (size_t i = 0; i < a.size(); ++i) a[i] -= ref[i];
        const float ducked = gainToDb (rms (a, 4800));
        CHECK (std::abs (ducked - (h - 3.0f)) < 1.0f, "texture ducks 3 dB under the beat (%.1f vs %.1f dBFS)", ducked, h);
    }

    std::printf ("\n%s (%d failure%s)\n", failures ? "FAILED" : "ALL PASSED", failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
