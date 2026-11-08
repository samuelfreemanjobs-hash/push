#include "Analysis.h"
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <map>
#include <numeric>

namespace forge
{
namespace
{
    constexpr int fftOrder = 11, fftSize = 1 << fftOrder;
    constexpr int numBands = 24, numEnvSegments = 8;

    double bandEdge (int i) // log-spaced 40 Hz .. 16 kHz
    {
        return 40.0 * std::pow (16000.0 / 40.0, (double) i / numBands);
    }
}

float momentaryLoudness (const juce::AudioBuffer<float>& audio, double sr)
{
    const int n = audio.getNumSamples();
    if (n < 16) return -100.0f;

    // K-weighting: high shelf (+4 dB above ~1.7 kHz) then high-pass at ~38 Hz (RBJ biquads)
    struct Biquad
    {
        double b0, b1, b2, a1, a2, x1 = 0, x2 = 0, y1 = 0, y2 = 0;
        double process (double x)
        {
            auto y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1; x1 = x; y2 = y1; y1 = y;
            return y;
        }
    };
    auto shelf = [sr]
    {
        const double f0 = 1681.974450955533, gainDb = 3.999843853973347, q = 0.7071752369554196;
        auto A = std::pow (10.0, gainDb / 40.0), w = juce::MathConstants<double>::twoPi * f0 / sr;
        auto alpha = std::sin (w) / (2 * q), c = std::cos (w), sq = 2 * std::sqrt (A) * alpha;
        auto a0 = (A + 1) - (A - 1) * c + sq;
        return Biquad { A * ((A + 1) + (A - 1) * c + sq) / a0, -2 * A * ((A - 1) + (A + 1) * c) / a0,
                        A * ((A + 1) + (A - 1) * c - sq) / a0, 2 * ((A - 1) - (A + 1) * c) / a0,
                        ((A + 1) - (A - 1) * c - sq) / a0 };
    };
    auto highpass = [sr]
    {
        const double f0 = 38.13547087602444, q = 0.5003270373238773;
        auto w = juce::MathConstants<double>::twoPi * f0 / sr, alpha = std::sin (w) / (2 * q), c = std::cos (w);
        auto a0 = 1 + alpha;
        return Biquad { (1 + c) / 2 / a0, -(1 + c) / a0, (1 + c) / 2 / a0, -2 * c / a0, (1 - alpha) / a0 };
    };

    std::vector<double> power ((size_t) n, 0.0); // K-weighted power summed over channels
    for (int ch = 0; ch < audio.getNumChannels(); ++ch)
    {
        auto s1 = shelf(); auto s2 = highpass();
        for (int i = 0; i < n; ++i)
        {
            auto y = s2.process (s1.process (audio.getSample (ch, i)));
            power[(size_t) i] += y * y;
        }
    }

    // loudest 400 ms window (or the whole sample if shorter), 100 ms hop
    auto win = juce::jmin (n, (int) (0.4 * sr)), hop = juce::jmax (1, (int) (0.1 * sr));
    std::vector<double> prefix ((size_t) n + 1, 0.0);
    for (int i = 0; i < n; ++i) prefix[(size_t) i + 1] = prefix[(size_t) i] + power[(size_t) i];
    double best = 0;
    for (int start = 0; start + win <= n; start += hop)
        best = juce::jmax (best, (prefix[(size_t) (start + win)] - prefix[(size_t) start]) / win);
    best = juce::jmax (best, (prefix[(size_t) n] - prefix[(size_t) (n - win)]) / win);
    return (float) (-0.691 + 10.0 * std::log10 (juce::jmax (1e-12, best)));
}

SampleFeatures analyseSample (const juce::AudioBuffer<float>& audio, double sr, int rootNote, bool measurePitch)
{
    SampleFeatures f;
    const int n = audio.getNumSamples();
    f.lengthSec = (float) (n / sr);
    if (n < 64) return f;

    // mono mix
    std::vector<float> x ((size_t) n);
    for (int i = 0; i < n; ++i)
    {
        float s = 0;
        for (int ch = 0; ch < audio.getNumChannels(); ++ch) s += audio.getSample (ch, i);
        x[(size_t) i] = s / (float) juce::jmax (1, audio.getNumChannels());
    }

    double sumSq = 0;
    for (auto v : x) { f.peak = juce::jmax (f.peak, std::abs (v)); sumSq += (double) v * v; }
    auto rms = std::sqrt (sumSq / n);
    f.crestDb = (float) juce::Decibels::gainToDecibels (f.peak / juce::jmax (1e-9, rms));

    // ---- spectrum over the first 500 ms (the part you hear on a pad hit)
    juce::dsp::FFT fft (fftOrder);
    juce::dsp::WindowingFunction<float> window (fftSize, juce::dsp::WindowingFunction<float>::hann);
    std::vector<double> power (fftSize / 2 + 1, 0.0);
    auto analysed = juce::jmin (n, (int) (0.5 * sr));
    std::vector<float> frame (fftSize * 2);
    for (int start = 0; start == 0 || start + fftSize / 2 < analysed; start += fftSize / 2)
    {
        std::fill (frame.begin(), frame.end(), 0.0f);
        for (int i = 0; i < fftSize && start + i < n; ++i) frame[(size_t) i] = x[(size_t) (start + i)];
        window.multiplyWithWindowingTable (frame.data(), fftSize);
        fft.performFrequencyOnlyForwardTransform (frame.data());
        for (int k = 0; k <= fftSize / 2; ++k) power[(size_t) k] += (double) frame[(size_t) k] * frame[(size_t) k];
    }

    auto binHz = sr / fftSize;
    double total = 0, sub = 0, mud = 0, harsh = 0, weighted = 0;
    std::vector<double> bands (numBands, 1e-12);
    for (int k = 1; k <= fftSize / 2; ++k)
    {
        auto hz = k * binHz;
        if (hz < 30 || hz > 18000) continue;
        auto p = power[(size_t) k];
        total += p; weighted += p * hz;
        if (hz < 80) sub += p;
        if (hz >= 200 && hz < 500) mud += p;
        if (hz >= 2500 && hz < 5000) harsh += p;
        for (int b = 0; b < numBands; ++b)
            if (hz >= bandEdge (b) && hz < bandEdge (b + 1)) { bands[(size_t) b] += p; break; }
    }
    if (total > 0)
    {
        f.subShare = (float) (sub / total);
        f.mudShare = (float) (mud / total);
        f.harshShare = (float) (harsh / total);
        f.centroidHz = (float) (weighted / total);
    }

    // ---- fingerprint: spectral shape (level removed) + loudness envelope
    std::vector<float> bandDb;
    for (auto b : bands) bandDb.push_back ((float) (10.0 * std::log10 (b)));
    auto maxBand = *std::max_element (bandDb.begin(), bandDb.end());
    for (auto& b : bandDb) b = juce::jmax (b - maxBand, -60.0f);
    f.fingerprint = bandDb;

    auto envLen = juce::jmin (n, (int) (0.6 * sr));
    std::vector<float> env;
    for (int s = 0; s < numEnvSegments; ++s)
    {
        auto a = envLen * s / numEnvSegments, b = envLen * (s + 1) / numEnvSegments;
        double e = 0;
        for (int i = a; i < b; ++i) e += (double) x[(size_t) i] * x[(size_t) i];
        env.push_back ((float) (10.0 * std::log10 (e / juce::jmax (1, b - a) + 1e-12)));
    }
    auto maxEnv = *std::max_element (env.begin(), env.end());
    for (auto e : env) f.fingerprint.push_back (juce::jmax (e - maxEnv, -60.0f) * 0.5f); // envelope counts half as much

    // ---- pitch clarity: normalised autocorrelation at the root's period
    if (measurePitch)
    {
        auto expected = juce::MidiMessage::getMidiNoteInHertz (rootNote);
        auto start = juce::jmin (n - 1, (int) (0.02 * sr));
        auto len = juce::jmin (n - start, (int) (0.3 * sr));
        auto period = sr / expected;
        if (len > (int) (4 * period))
        {
            const float* s = x.data() + start;
            auto corr = [&] (int lag)
            {
                double c = 0, e1 = 0, e2 = 0;
                for (int i = 0; i + lag < len; ++i) { c += (double) s[i] * s[i + lag]; e1 += (double) s[i] * s[i]; e2 += (double) s[i + lag] * s[i + lag]; }
                return c / std::sqrt (juce::jmax (1e-12, e1 * e2));
            };
            double best = -1;
            for (int lag = juce::jmax (1, (int) (period * 0.97)); lag <= (int) (period * 1.03) + 1; ++lag)
                best = juce::jmax (best, corr (lag));
            f.harmonicity = (float) juce::jlimit (0.0, 1.0, best);
        }
    }
    return f;
}

float fingerprintDistance (const SampleFeatures& a, const SampleFeatures& b)
{
    if (a.fingerprint.size() != b.fingerprint.size() || a.fingerprint.empty()) return 1e9f;
    double s = 0;
    for (size_t i = 0; i < a.fingerprint.size(); ++i)
    {
        auto d = a.fingerprint[i] - b.fingerprint[i];
        s += d * d;
    }
    return (float) std::sqrt (s / (double) a.fingerprint.size());
}

//==============================================================================
std::vector<PickVerdict> smartPick (const std::vector<PickCandidate>& cands, int target, const PickSettings& settings)
{
    const int n = (int) cands.size();
    std::vector<PickVerdict> v ((size_t) n);

    // ---- batch statistics, so "muddy" / "harsh" mean muddier / harsher than the rest of this batch
    auto meanStd = [&] (auto getter)
    {
        double s = 0, s2 = 0; int c = 0;
        for (auto& p : cands) if (p.valid) { auto x = getter (p); s += x; s2 += x * x; ++c; }
        auto mean = c > 0 ? s / c : 0.0;
        return std::make_pair (mean, std::sqrt (juce::jmax (1e-12, (c > 0 ? s2 / c : 0.0) - mean * mean)));
    };
    auto [mudMean, mudStd]     = meanStd ([] (const PickCandidate& p) { return (double) p.features.mudShare; });
    auto [harshMean, harshStd] = meanStd ([] (const PickCandidate& p) { return (double) p.features.harshShare; });

    for (int i = 0; i < n; ++i)
    {
        auto& c = cands[(size_t) i];
        auto& r = v[(size_t) i];
        if (! c.valid) { r.score = 0; r.reasons.add ("render failed"); continue; }

        auto& f = c.features;
        r.score = 100;
        auto penalise = [&r] (float points, const juce::String& why) { r.score -= points; r.reasons.add (why); };

        if (f.harmonicity >= 0)
        {
            if (f.harmonicity < 0.5f)       penalise (25, "unclear pitch");
            else if (f.harmonicity < 0.75f) penalise (10, "pitch a bit unclear");
        }

        const bool bassRegister = c.rootNote < 48; // mud and sub are expected down there

        if (c.lowIntervalViolations > 0)
            penalise ((float) juce::jmin (30, 15 * c.lowIntervalViolations), "muddy voicing: " + c.lowIntervalDetail);

        auto mudZ = (f.mudShare - mudMean) / mudStd;
        if (! bassRegister && mudZ > 2.0 && f.mudShare > 0.3f)   penalise (10, "muddier than the rest");

        auto harshZ = (f.harshShare - harshMean) / harshStd;
        if (f.harshShare > 0.4f)                                 penalise (20, "harsh (lots of 2.5-5 kHz)");
        else if (harshZ > 2.0 && f.harshShare > 0.15f)           penalise (10, "harsher than the rest");

        if (! bassRegister && f.subShare > 0.3f)          penalise (10, "low rumble");
        if (f.crestDb > 24.0f)                            penalise (15, "mostly a click / transient");
        if (f.peak < 0.05f)                               penalise (30, "almost silent");
        if (c.extraPenalty > 0)                           penalise (c.extraPenalty, c.extraReason);

        r.score = juce::jlimit (0.0f, 100.0f, r.score);
    }

    // ---- per-group quotas keep the mix (e.g. chords vs single keys, each recipe of a kit)
    std::map<juce::String, std::vector<int>> groups;
    for (int i = 0; i < n; ++i) if (cands[(size_t) i].valid) groups[cands[(size_t) i].group].push_back (i);
    int valid = 0;
    for (auto& [g, idx] : groups) valid += (int) idx.size();
    target = juce::jmin (target, valid);

    // Two levels of fair shares (largest remainder): first between recipes / styles
    // (the part before '/'), then between chords and keys inside each one.
    auto shareOut = [] (const std::map<juce::String, int>& sizes, int total)
    {
        std::map<juce::String, int> out;
        int sum = 0, given = 0;
        for (auto& [k, size] : sizes) sum += size;
        std::vector<std::pair<double, juce::String>> remainders;
        for (auto& [k, size] : sizes)
        {
            auto exact = sum > 0 ? (double) total * size / sum : 0.0;
            out[k] = juce::jmin (size, (int) std::floor (exact));
            given += out[k];
            remainders.push_back ({ exact - std::floor (exact), k });
        }
        std::stable_sort (remainders.begin(), remainders.end(), [] (auto& x, auto& y) { return x.first > y.first; });
        for (size_t i = 0; given < total && ! remainders.empty(); i = (i + 1) % remainders.size())
        {
            auto& k = remainders[i].second;
            if (out[k] < sizes.at (k)) { ++out[k]; ++given; }
            else if (std::all_of (sizes.begin(), sizes.end(), [&out] (auto& p) { return out[p.first] >= p.second; })) break;
        }
        return out;
    };

    std::map<juce::String, std::map<juce::String, int>> byStyle; // style -> group -> size
    for (auto& [g, idx] : groups) byStyle[g.upToFirstOccurrenceOf ("/", false, false)][g] = (int) idx.size();

    std::map<juce::String, int> styleSizes;
    for (auto& [st, gs] : byStyle) { int size = 0; for (auto& [g, c] : gs) size += c; styleSizes[st] = size; }

    std::map<juce::String, int> quota;
    for (auto& [st, share] : shareOut (styleSizes, target))
        for (auto& [g, q] : shareOut (byStyle[st], share))
            quota[g] = q;

    // distances once, up front
    std::vector<float> dist ((size_t) n * (size_t) n, 0.0f);
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            dist[(size_t) (i * n + j)] = dist[(size_t) (j * n + i)] = fingerprintDistance (cands[(size_t) i].features, cands[(size_t) j].features);

    std::vector<int> kept;
    std::vector<float> nearest ((size_t) n, 1e9f); // distance from each candidate to the closest pick so far

    auto take = [&] (int i)
    {
        v[(size_t) i].kept = true;
        kept.push_back (i);
        for (int j = 0; j < n; ++j) nearest[(size_t) j] = juce::jmin (nearest[(size_t) j], dist[(size_t) (i * n + j)]);
    };

    for (auto& [g, idx] : groups)
        for (int taken = 0; taken < quota[g]; ++taken)
        {
            int best = -1;
            float bestUtility = -1e9f;
            for (auto i : idx)
            {
                if (v[(size_t) i].kept) continue;
                if (settings.strictVariety && nearest[(size_t) i] < settings.duplicateDb) continue;
                auto variety = juce::jmin (nearest[(size_t) i], settings.varietyCapDb) * settings.varietyPerDb;
                auto utility = v[(size_t) i].score + variety;
                if (utility > bestUtility) { bestUtility = utility; best = i; }
            }
            if (best < 0) break;
            take (best);
        }

    for (int i = 0; i < n; ++i)
    {
        auto& r = v[(size_t) i];
        float closest = 1e9f;
        int who = -1;
        for (auto k : kept)
            if (k != i && dist[(size_t) (i * n + k)] < closest) { closest = dist[(size_t) (i * n + k)]; who = k; }
        r.nearest = closest;
        if (! r.kept && closest < settings.duplicateDb) r.similarTo = who;
    }
    return v;
}
}
