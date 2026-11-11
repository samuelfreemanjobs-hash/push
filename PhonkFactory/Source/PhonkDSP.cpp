#include "PhonkDSP.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>

namespace phonk
{
static constexpr double kPi = 3.14159265358979323846;

const char* voiceName (int v)
{
    static const char* names[] = { "Kick", "Snare", "ClosedHat", "OpenHat", "Cowbell" };
    return (v >= 0 && v < NumVoices) ? names[v] : "?";
}

//==============================================================================
Rng::Rng (uint64_t seed) : s (seed * 0x9E3779B97F4A7C15ULL + 0xD1B54A32D192ED03ULL) { next(); next(); }

uint64_t Rng::next()
{
    uint64_t z = (s += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}
double Rng::uniform() { return (double) (next() >> 11) * (1.0 / 9007199254740992.0); }
double Rng::gauss()
{
    const double u1 = std::max (uniform(), 1e-12), u2 = uniform();
    return std::sqrt (-2.0 * std::log (u1)) * std::cos (2.0 * kPi * u2);
}
int Rng::range (int n) { return n <= 1 ? 0 : (int) (next() % (uint64_t) n); }
bool Rng::chance (double p) { return uniform() < p; }

//==============================================================================
Biquad Biquad::make (bool hp, double fc, double sr, double q)
{
    fc = std::clamp (fc, 10.0, sr * 0.49);
    const double w0 = 2.0 * kPi * fc / sr, c = std::cos (w0), alpha = std::sin (w0) / (2.0 * q);
    const double a0 = 1.0 + alpha;
    Biquad f;
    f.b0 = (hp ? (1.0 + c) : (1.0 - c)) * 0.5 / a0;
    f.b1 = (hp ? -(1.0 + c) : (1.0 - c)) / a0;
    f.b2 = f.b0;
    f.a1 = -2.0 * c / a0;
    f.a2 = (1.0 - alpha) / a0;
    return f;
}

Biquad Biquad::makeFirstOrder (bool hp, double fc, double sr)
{
    fc = std::clamp (fc, 10.0, sr * 0.49);
    const double k = std::tan (kPi * fc / sr);
    Biquad f;
    f.b0 = (hp ? 1.0 : k) / (1.0 + k);
    f.b1 = hp ? -f.b0 : f.b0;
    f.a1 = (k - 1.0) / (1.0 + k);
    return f;
}

ButterFilter::ButterFilter (int order, double fc, double sr, bool hp)
{
    // Butterworth poles: Q_k = 1 / (2 cos(pi (2k+1) / (2N))) for each conjugate pair.
    for (int k = 0; k < order / 2; ++k)
        sections.push_back (Biquad::make (hp, fc, sr, 1.0 / (2.0 * std::cos (kPi * (2 * k + 1) / (2.0 * order)))));
    if (order % 2) sections.push_back (Biquad::makeFirstOrder (hp, fc, sr));
}
float ButterFilter::process (float x) noexcept
{
    for (auto& s : sections) x = s.process (x);
    return x;
}
void ButterFilter::process (std::vector<float>& v) noexcept
{
    for (auto& x : v) x = process (x);
}

//==============================================================================
void cassetteFx (std::vector<float>& a, const CassetteParams& p, double sr, Rng& rng, bool normalize)
{
    const double td = std::tanh (p.drive);
    const double steps = std::pow (2.0, std::round (p.bitDepth) - 1.0);
    ButterFilter lpf (2, p.lpfHz, sr, false);
    for (auto& s : a)
    {
        double x = std::tanh (s * p.drive) / td;                       // tape saturation
        x = std::round (x * steps) / steps;                           // SP-1200 style quantisation
        if (std::abs (x) > 0.001) x += rng.gauss() * p.hiss;          // gated hiss
        s = lpf.process ((float) x);                                  // roll off the digital top
    }
    if (normalize)
    {
        float peak = 0.f;
        for (float s : a) peak = std::max (peak, std::abs (s));
        if (peak > 0.f) for (auto& s : a) s = s / peak * 0.89f;       // -1 dB
    }
}

static void fadeTail (std::vector<float>& a, double sr, double ms = 4.0)
{
    const int n = std::min<int> ((int) a.size(), std::max (1, (int) (sr * ms * 0.001)));
    for (int i = 0; i < n; ++i) a[a.size() - 1 - i] *= (float) i / n;
}

static size_t numSamples (double sr, double seconds) { return std::max<size_t> (16, (size_t) (sr * seconds)); }

//==============================================================================
std::vector<float> generateKick (double sr, Rng& rng, double f0, double f1, double punch, double dur, const CassetteParams& fx)
{
    const size_t n = numSamples (sr, dur);
    std::vector<float> out (n);
    double phase = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
        const double t = (double) i / sr;
        phase += 2.0 * kPi * (f1 + (f0 - f1) * std::exp (-t * punch)) / sr;   // exponential pitch drop
        const double click = rng.gauss() * 0.3 * std::exp (-t * 140.0);
        out[i] = (float) ((std::sin (phase) + click) * std::exp (-t * 5.5 * 0.55 / dur));
    }
    Rng fxRng (rng.next());
    cassetteFx (out, fx, sr, fxRng);
    fadeTail (out, sr);
    return out;
}

std::vector<float> generateSnare (double sr, Rng& rng, double tone, double dm, const CassetteParams& fx)
{
    const size_t n = numSamples (sr, 0.35 * std::max (1.0, dm));
    std::vector<float> out (n);
    ButterFilter hp (2, 1100.0, sr, true), lp (2, 6800.0, sr, false);
    double phase = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
        const double t = (double) i / sr;
        phase += 2.0 * kPi * (tone + 90.0 * std::exp (-t * 45.0)) / sr;
        const double body = std::sin (phase) * std::exp (-t * 18.0 / dm);
        const double wires = lp.process (hp.process ((float) rng.gauss())) * std::exp (-t * 12.0 / dm);
        out[i] = (float) (body * 0.65 + wires * 0.85);
    }
    Rng fxRng (rng.next());
    cassetteFx (out, fx, sr, fxRng);
    fadeTail (out, sr);
    return out;
}

std::vector<float> generateHat (double sr, Rng& rng, bool open, double dm, double ratio, const CassetteParams& fx)
{
    const size_t n = numSamples (sr, (open ? 0.32 : 0.09) * std::max (1.0, dm));
    std::vector<float> out (n);
    static const double freqs[] = { 205.3, 304.4, 369.6, 522.7, 540.0, 800.0 };   // 808-style metallic square stack
    ButterFilter hp (3, 6500.0, sr, true);
    const double decay = (open ? 7.5 : 48.0) / dm;
    for (size_t i = 0; i < n; ++i)
    {
        const double t = (double) i / sr;
        double metal = 0.0;
        for (double f : freqs) { const double s = std::sin (2.0 * kPi * f * ratio * t); metal += (s > 0) - (s < 0); }
        metal /= 6.0;
        out[i] = (float) (hp.process ((float) (metal + rng.gauss() * 0.5)) * std::exp (-t * decay));
    }
    Rng fxRng (rng.next());
    cassetteFx (out, fx, sr, fxRng);
    fadeTail (out, sr);
    return out;
}

std::vector<float> generateCowbell (double sr, Rng& rng, double root, const CassetteParams& fx)
{
    const size_t n = numSamples (sr, 0.38);
    std::vector<float> out (n);
    ButterFilter hp (2, 900.0, sr, true), lp (2, 4200.0, sr, false);
    for (size_t i = 0; i < n; ++i)
    {
        const double t = (double) i / sr;
        const double a = std::sin (2.0 * kPi * root * t), b = std::sin (2.0 * kPi * root * 1.481 * t);
        const double sq = ((a > 0) - (a < 0)) + ((b > 0) - (b < 0));
        out[i] = (float) (lp.process (hp.process ((float) sq)) * std::exp (-t * 9.5));
    }
    Rng fxRng (rng.next());
    cassetteFx (out, fx, sr, fxRng);
    fadeTail (out, sr);
    return out;
}

Kit renderKit (const KitSettings& k, double sr)
{
    Kit kit;
    kit.sampleRate = sr;
    auto ratio = [] (double semis) { return std::pow (2.0, semis / 12.0); };
    auto fx = [&] (double drive, double lpf)
    {
        CassetteParams p;
        p.drive = std::max (0.2, drive * k.grit);
        p.bitDepth = k.bitDepth;
        p.hiss = k.hiss;
        p.lpfHz = lpf * k.tone;
        return p;
    };
    for (int i = 0; i < kVariations; ++i)
    {
        const int v = i + 1;   // variation numbering follows the original script (1..4)
        auto rng = [&] (int voice) { return Rng ((uint64_t) k.seed * 1000003ULL + (uint64_t) padIndex (voice, i) * 7919ULL); };
        { auto r = rng (Kick);
          const double kt = ratio (k.kickTune);
          kit.samples[padIndex (Kick, i)] = generateKick (sr, r, (125 + v * 8) * kt, (42 + v * 2) * kt, 18.0,
                                                          0.55 * k.kickDecay, fx (3.2, 7500)); }
        { auto r = rng (Snare);
          kit.samples[padIndex (Snare, i)] = generateSnare (sr, r, (175 + v * 12) * ratio (k.snareTune), k.snareDecay, fx (2.8, 9000)); }
        { auto r = rng (ClosedHat);
          kit.samples[padIndex (ClosedHat, i)] = generateHat (sr, r, false, k.hatDecay * (1.0 + 0.12 * (i - 1.5)),
                                                              1.0 + 0.025 * (i - 1.5), fx (2.0, 11000)); }
        { auto r = rng (OpenHat);
          kit.samples[padIndex (OpenHat, i)] = generateHat (sr, r, true, k.hatDecay * (1.0 + 0.12 * (i - 1.5)),
                                                            1.0 + 0.025 * (i - 1.5), fx (2.0, 11000)); }
        { auto r = rng (Cowbell);
          kit.samples[padIndex (Cowbell, i)] = generateCowbell (sr, r, (520 + v * 15) * ratio (k.cowTune), fx (2.6, 8000)); }
    }
    return kit;
}

//==============================================================================
void BusFx::prepare (double newSr, uint64_t seed)
{
    sr = newSr;
    rng = Rng (seed);
    delay.assign ((size_t) std::max (4096.0, sr * 0.01), 0.f);
    setSettings (st);
    reset();
}

void BusFx::setSettings (const BusSettings& s)
{
    st = s;
    lpf = [&] { auto f = Biquad::make (false, s.lpfHz, sr, 0.70710678); f.z1 = lpf.z1; f.z2 = lpf.z2; return f; }();
    steps = std::pow (2.0, std::round (s.bitDepth) - 1.0);
    tanhDrive = std::tanh (std::max (0.05, s.drive));
}

void BusFx::reset()
{
    lpf.reset();
    std::fill (delay.begin(), delay.end(), 0.f);
    writePos = 0;
    lfoPhase = 0.0;
}

float BusFx::process (float in) noexcept
{
    // wow & flutter: modulated fractional delay (0.5 wow ~ the +-18 samples @44.1k of the original script)
    const int size = (int) delay.size();
    delay[(size_t) writePos] = in;
    const double depth = st.wow * 0.0008 * sr;
    const double d = depth + 2.0 + depth * std::sin (lfoPhase);
    lfoPhase += 2.0 * kPi * wowHz / sr;
    if (lfoPhase > 2.0 * kPi) lfoPhase -= 2.0 * kPi;
    double rp = (double) writePos - d;
    if (rp < 0) rp += size;
    const int i0 = (int) rp, i1 = (i0 + 1) % size;
    const float fr = (float) (rp - i0);
    float x = delay[(size_t) i0] * (1.f - fr) + delay[(size_t) i1] * fr;
    if (++writePos >= size) writePos = 0;

    double y = std::tanh (x * st.drive) / tanhDrive;
    y = std::round (y * steps) / steps;
    if (std::abs (y) > 0.001) y += rng.gauss() * st.hiss;
    return lpf.process ((float) y);
}

//==============================================================================
double eventTime (const PatternEvent& e, double swing)
{
    return e.step + e.micro + ((e.step & 1) ? swing : 0.0);
}

Pattern generatePattern (const PatternSettings& ps)
{
    Pattern pat;
    pat.bars = ps.bars;
    Rng r ((uint64_t) ps.seed * 7919ULL + 13ULL);
    const int kv = r.range (kVariations), sv = r.range (kVariations), hv = r.range (kVariations),
              ov = r.range (kVariations), cv = r.range (kVariations);

    auto add = [&] (int bar, int step, int voice, int var, double vel, double micro = 0.0)
    {
        const double h = 1.0 + (r.uniform() - 0.5) * ps.humanize * 0.5;
        PatternEvent e;
        e.step = (uint8_t) (bar * 16 + step);
        e.voice = (uint8_t) voice;
        e.variation = (uint8_t) var;
        e.micro = (float) micro;
        e.velocity = (float) std::clamp (vel * h, 0.05, 1.0);
        pat.events.push_back (e);
    };

    static const int kickBase[] = { 0, 3, 7, 10, 14 };   // classic Memphis groove from the original script
    static const int snareBase[] = { 4, 12 };
    static const int ohBase[] = { 2, 10 };

    for (int bar = 0; bar < ps.bars; ++bar)
    {
        const double pm = ps.variation * (bar == 0 ? 0.4 : 1.0);   // bar 1 stays close to the classic pattern
        std::array<bool, 16> kick {}, snare {}, oh {}, hatOff {};
        std::array<float, 16> snareVel {};
        for (int s : kickBase) kick[(size_t) s] = true;
        for (int s : snareBase) { snare[(size_t) s] = true; snareVel[(size_t) s] = 0.90f; }
        for (int s : ohBase) oh[(size_t) s] = true;

        for (int s = 1; s < 16; ++s)                         // kick mutation
        {
            if (kick[(size_t) s] && r.chance (pm * 0.30)) kick[(size_t) s] = false;
            else if (! kick[(size_t) s] && s != 4 && s != 12 && r.chance (pm * 0.16)) kick[(size_t) s] = true;
        }
        if (r.chance (pm * 0.4)) { const int g = r.chance (0.5) ? 15 : 13; snare[(size_t) g] = true; snareVel[(size_t) g] = 0.32f; }
        if (r.chance (pm * 0.5))                              // slide an open hat
        {
            static const int ohMoves[] = { 6, 14, 15 };
            const int from = ohBase[r.range (2)], to = ohMoves[r.range (3)];
            oh[(size_t) from] = false; oh[(size_t) to] = true;
        }
        for (int s = 0; s < 16; ++s) if (r.chance (pm * 0.10)) hatOff[(size_t) s] = true;   // hat drop-outs

        // hat roll: triplet burst replacing two 16ths, or a 32nd double-hit
        int rollStep = -1; bool triplet = true;
        if (r.chance (ps.rolls * (bar % 2 ? 1.0 : 0.6))) { rollStep = r.chance (0.5) ? 6 : 14; triplet = r.chance (0.65); }
        if (rollStep >= 0) { oh[(size_t) rollStep] = oh[(size_t) rollStep + 1] = false; hatOff[(size_t) rollStep] = hatOff[(size_t) rollStep + 1] = true; }

        for (int s = 0; s < 16; ++s)
        {
            if (kick[(size_t) s]) add (bar, s, Kick, kv, 0.95);
            if (snare[(size_t) s]) add (bar, s, Snare, sv, snareVel[(size_t) s]);
            if (oh[(size_t) s]) add (bar, s, OpenHat, ov, 0.55);
            else if (! hatOff[(size_t) s]) add (bar, s, ClosedHat, hv, (s % 2 == 0) ? 0.55 : 0.35);
        }
        if (rollStep >= 0)
        {
            if (triplet) { add (bar, rollStep, ClosedHat, hv, 0.40); add (bar, rollStep, ClosedHat, hv, 0.50, 2.0 / 3.0);
                           add (bar, rollStep, ClosedHat, hv, 0.65, 4.0 / 3.0); }
            else         { add (bar, rollStep, ClosedHat, hv, 0.50); add (bar, rollStep, ClosedHat, hv, 0.40, 0.5);
                           add (bar, rollStep + 1, ClosedHat, hv, 0.50); add (bar, rollStep + 1, ClosedHat, hv, 0.45, 0.5); }
        }
        static const int cowSteps[] = { 3, 6, 11, 14 };
        for (int s : cowSteps) if (r.chance (ps.cowbell * 0.5 * (bar == 0 ? 0.6 : 1.0))) add (bar, s, Cowbell, cv, 0.60);
    }
    return pat;
}

//==============================================================================
std::vector<float> renderLoop (const Kit& kit, const Pattern& pat, double bpm, double swing,
                               const MixSettings& mix, const BusSettings& bus, bool normalize)
{
    const double sr = kit.sampleRate;
    const double stepSamples = 60.0 / bpm / 4.0 * sr;
    const size_t len = (size_t) std::llround (pat.totalSteps() * stepSamples);
    std::vector<float> loop (len, 0.f);

    for (auto& e : pat.events)
    {
        const auto& smp = kit.samples[(size_t) padIndex (e.voice, e.variation)];
        const double t = std::fmod (eventTime (e, swing), (double) pat.totalSteps());
        const size_t start = (size_t) std::llround (t * stepSamples);
        const float g = e.velocity * mix.level[e.voice];
        for (size_t i = 0; i < smp.size(); ++i) loop[(start + i) % len] += smp[i] * g;   // tails wrap = seamless loop
    }

    const double secs = (double) len / sr;
    BusFx fx;
    fx.prepare (sr, 7);
    fx.setSettings (bus);
    fx.setWowRate (std::max (1.0, std::round (1.8 * secs)) / secs);   // whole LFO cycles per loop
    for (size_t i = 0; i < len; ++i) fx.process (loop[i]);            // pass 1 primes the delay/filter state
    for (auto& s : loop) s = fx.process (s);                          // pass 2 is the result

    if (normalize)
    {
        float peak = 0.f;
        for (float s : loop) peak = std::max (peak, std::abs (s));
        if (peak > 0.f) for (auto& s : loop) s = s / peak * 0.89f;
    }
    return loop;
}

bool writeWav (const std::string& path, const std::vector<float>& mono, double sr, int bits)
{
    bits = (bits == 24) ? 24 : 16;
    std::ofstream f (path, std::ios::binary);
    if (! f) return false;
    const uint32_t bytes = (uint32_t) mono.size() * (uint32_t) (bits / 8), rate = (uint32_t) sr;
    auto w32 = [&] (uint32_t v) { for (int i = 0; i < 4; ++i) f.put ((char) ((v >> (8 * i)) & 0xff)); };
    auto w16 = [&] (uint16_t v) { f.put ((char) (v & 0xff)); f.put ((char) (v >> 8)); };
    f.write ("RIFF", 4); w32 (36 + bytes); f.write ("WAVEfmt ", 8);
    w32 (16); w16 (1); w16 (1); w32 (rate); w32 (rate * (bits / 8)); w16 ((uint16_t) (bits / 8)); w16 ((uint16_t) bits);
    f.write ("data", 4); w32 (bytes);
    for (float s : mono)
    {
        const double c = std::clamp ((double) s, -1.0, 1.0);
        if (bits == 16) w16 ((uint16_t) (int16_t) std::lround (c * 32767.0));
        else { const int32_t v = (int32_t) std::lround (c * 8388607.0);
               f.put ((char) (v & 0xff)); f.put ((char) ((v >> 8) & 0xff)); f.put ((char) ((v >> 16) & 0xff)); }
    }
    return (bool) f;
}

int exportKit (const std::string& folder, const ExportOptions& o, const std::function<void (const std::string&)>& log)
{
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::create_directories (folder, ec);
    int count = 0;
    auto save = [&] (const std::string& name, const std::vector<float>& a)
    {
        if (writeWav ((fs::path (folder) / name).string(), a, o.sampleRate, o.bits)) { ++count; if (log) log (name); }
    };

    const Kit kit = renderKit (o.kit, o.sampleRate);
    for (int v = 0; v < NumVoices; ++v)
        for (int i = 0; i < kVariations; ++i)
            save (std::string ("Phonk_") + voiceName (v) + "_0" + std::to_string (i + 1) + ".wav", kit.samples[(size_t) padIndex (v, i)]);

    auto loopWith = [&] (double bpm, PatternSettings ps, const std::string& name)
    {
        save (name, renderLoop (kit, generatePattern (ps), bpm, o.swing, o.mix, o.bus));
    };
    for (int bpm : { 112, 118, 124 })                       // classic loops = the original script's three breaks
    {
        PatternSettings ps; ps.variation = 0; ps.rolls = 0; ps.cowbell = 0; ps.bars = 2; ps.seed = 1;
        loopWith (bpm, ps, "Memphis_Loop_" + std::to_string (bpm) + "BPM.wav");
    }
    for (int i = 0; i < o.extraLoops; ++i)                  // seeded variations
    {
        PatternSettings ps = o.pattern; ps.seed = o.pattern.seed + 1 + i; ps.variation = std::max (0.5, o.pattern.variation);
        const int bpm = 104 + ((i * 7 + o.pattern.seed) % 5) * 6;
        loopWith (bpm, ps, "Memphis_Loop_" + std::to_string (bpm) + "BPM_Var" + std::to_string (i + 1) + ".wav");
    }
    loopWith (o.currentBpm, o.pattern, "Memphis_Loop_Current_" + std::to_string ((int) std::lround (o.currentBpm)) + "BPM.wav");
    return count;
}
}
