#include "Effects.h"
#include <cmath>

namespace forge
{
namespace
{
    constexpr double pi = juce::MathConstants<double>::pi;
    constexpr double twoPi = juce::MathConstants<double>::twoPi;

    double num (const juce::var& o, const char* key, double def) { return o.hasProperty (key) ? (double) o[key] : def; }
    juce::String str (const juce::var& o, const char* key, const char* def) { return o.hasProperty (key) ? o[key].toString() : juce::String (def); }

    //--------------------------------------------------------------------------
    /** Simper SVF: one 12 dB/oct stage with low / band / high outputs. */
    struct Svf
    {
        double a1 = 0, a2 = 0, a3 = 0, k = 1, ic1 = 0, ic2 = 0;
        void set (double fc, double q, double sr)
        {
            fc = juce::jlimit (5.0, sr * 0.49, fc);
            auto g = std::tan (pi * fc / sr);
            k = 1.0 / q;
            a1 = 1.0 / (1.0 + g * (g + k)); a2 = g * a1; a3 = g * a2;
        }
        void tick (double x, double& lp, double& bp, double& hp)
        {
            auto v3 = x - ic2;
            auto v1 = a1 * ic1 + a2 * v3;
            auto v2 = ic2 + a2 * ic1 + a3 * v3;
            ic1 = 2 * v1 - ic1; ic2 = 2 * v2 - ic2;
            lp = v2; bp = v1; hp = x - k * v1 - v2;
        }
    };

    enum class FilterKind { Low, High };

    /** 2-pole or 4-pole (Butterworth-aligned) filter, resonance on the last stage. */
    void filterBuffer (juce::AudioBuffer<float>& b, double sr, FilterKind kind, double fc, int poles, double resonance)
    {
        auto stages = poles >= 4 ? 2 : 1;
        const double butterQ[2][2] = { { 0.7071, 0 }, { 0.5412, 1.3066 } };
        auto res = juce::jlimit (0.0, 0.95, resonance);

        for (int ch = 0; ch < b.getNumChannels(); ++ch)
        {
            Svf f[2];
            for (int s = 0; s < stages; ++s)
            {
                auto q = butterQ[stages - 1][s];
                if (s == stages - 1) q = q + res * res * 8.0; // resonance
                f[s].set (fc, q, sr);
            }
            auto* d = b.getWritePointer (ch);
            for (int i = 0; i < b.getNumSamples(); ++i)
            {
                double x = d[i], lp, bp, hp;
                for (int s = 0; s < stages; ++s)
                {
                    f[s].tick (x, lp, bp, hp);
                    x = kind == FilterKind::Low ? lp : hp;
                }
                d[i] = (float) x;
            }
        }
    }

    /** RBJ peaking EQ. */
    void peakEq (juce::AudioBuffer<float>& b, double sr, double fc, double gainDb, double q)
    {
        auto A = std::pow (10.0, gainDb / 40.0);
        auto w = twoPi * juce::jlimit (10.0, sr * 0.45, fc) / sr;
        auto alpha = std::sin (w) / (2.0 * q);
        auto a0 = 1 + alpha / A;
        auto b0 = (1 + alpha * A) / a0, b1 = -2 * std::cos (w) / a0, b2 = (1 - alpha * A) / a0;
        auto a1 = -2 * std::cos (w) / a0, a2 = (1 - alpha / A) / a0;

        for (int ch = 0; ch < b.getNumChannels(); ++ch)
        {
            double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
            auto* d = b.getWritePointer (ch);
            for (int i = 0; i < b.getNumSamples(); ++i)
            {
                auto x = (double) d[i];
                auto y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
                x2 = x1; x1 = x; y2 = y1; y1 = y;
                d[i] = (float) y;
            }
        }
    }

    float peakOf (const juce::AudioBuffer<float>& b) { return b.getMagnitude (0, b.getNumSamples()); }

    /** Waveshaper that keeps the overall level roughly where it was. */
    template <typename Fn>
    void shape (juce::AudioBuffer<float>& b, Fn fn)
    {
        auto before = peakOf (b);
        if (before <= 0) return;
        for (int ch = 0; ch < b.getNumChannels(); ++ch)
        {
            auto* d = b.getWritePointer (ch);
            for (int i = 0; i < b.getNumSamples(); ++i)
                d[i] = fn (d[i] / before);
        }
        auto after = peakOf (b);
        if (after > 0) b.applyGain (before / after);
    }

    void dcBlock (juce::AudioBuffer<float>& b)
    {
        for (int ch = 0; ch < b.getNumChannels(); ++ch)
        {
            float x1 = 0, y1 = 0;
            auto* d = b.getWritePointer (ch);
            for (int i = 0; i < b.getNumSamples(); ++i)
            {
                auto y = d[i] - x1 + 0.9995f * y1;
                x1 = d[i]; y1 = y; d[i] = y;
            }
        }
    }

    /** Reads a circular buffer at a fractional delay (linear interpolation). */
    struct DelayLine
    {
        std::vector<float> buf;
        int w = 0;
        explicit DelayLine (int size = 1) : buf ((size_t) juce::jmax (4, size), 0.0f) {}
        void push (float x) { buf[(size_t) w] = x; w = (w + 1) % (int) buf.size(); }
        float read (double delaySamples) const
        {
            auto size = (int) buf.size();
            auto pos = (double) w - 1.0 - juce::jlimit (0.0, (double) size - 3, delaySamples);
            while (pos < 0) pos += size;
            auto i0 = (int) pos; auto frac = (float) (pos - i0);
            return buf[(size_t) i0] + frac * (buf[(size_t) ((i0 + 1) % size)] - buf[(size_t) i0]);
        }
    };

    /** Delay-line pitch shifter (two crossfaded read heads), used for shimmer. */
    struct PitchShifter
    {
        DelayLine line;
        double phase = 0, ratio = 2, window = 2048;
        PitchShifter (double sr, double semitones) : line ((int) (0.1 * sr)), ratio (std::pow (2.0, semitones / 12.0)), window (0.05 * sr) {}
        float process (float x)
        {
            line.push (x);
            phase += (1.0 - ratio) / window;
            phase -= std::floor (phase);
            float out = 0;
            for (int h = 0; h < 2; ++h)
            {
                auto p = std::fmod (phase + 0.5 * h, 1.0);
                auto gain = (float) std::sin (pi * p);
                out += gain * gain * line.read (p * window + 1.0);
            }
            return out;
        }
    };

    //--------------------------------------------------------------------------
    void tapeSaturation (juce::AudioBuffer<float>& b, double sr, double drive)
    {
        auto g = 1.0 + 5.0 * juce::jlimit (0.0, 1.0, drive);
        shape (b, [g] (float x) { return (float) std::tanh (g * x); }); // symmetric -> odd harmonics
        filterBuffer (b, sr, FilterKind::Low, 13000.0, 2, 0.0);          // tape head roll-off
    }

    void tubeOverdrive (juce::AudioBuffer<float>& b, double drive)
    {
        auto g = 1.0 + 4.0 * juce::jlimit (0.0, 1.0, drive);
        shape (b, [g] (float x) { return (float) std::tanh (g * (x + 0.25 * x * x)); }); // asymmetric -> even harmonics
        dcBlock (b);
    }

    void overdrive (juce::AudioBuffer<float>& b, double sr, double drive, double tone)
    {
        peakEq (b, sr, 1200.0, 3.0, 0.7); // push the mids into the clipper
        auto g = 1.5 + 10.0 * juce::jlimit (0.0, 1.0, drive);
        shape (b, [g] (float x) { return (float) std::tanh (g * x); });
        filterBuffer (b, sr, FilterKind::Low, tone, 2, 0.0);
    }

    void bitcrush (juce::AudioBuffer<float>& b, int bits, int downsample)
    {
        auto peak = peakOf (b);
        if (peak <= 0) return;
        auto steps = std::pow (2.0, juce::jlimit (2, 24, bits) - 1);
        downsample = juce::jmax (1, downsample);
        for (int ch = 0; ch < b.getNumChannels(); ++ch)
        {
            auto* d = b.getWritePointer (ch);
            float held = 0;
            for (int i = 0; i < b.getNumSamples(); ++i)
            {
                if (i % downsample == 0)
                    held = (float) (std::round (d[i] / peak * steps) / steps) * peak;
                d[i] = held;
            }
        }
    }

    void wowFlutter (juce::AudioBuffer<float>& b, double sr, double amount, juce::Random& rng)
    {
        amount = juce::jlimit (0.0, 1.0, amount);
        auto base = 0.01 * sr;
        auto wowDepth = 0.003 * sr * amount, flutterDepth = 0.0004 * sr * amount;
        auto wowRate = 0.45 + 0.3 * rng.nextDouble(), flutterRate = 6.0 + 2.0 * rng.nextDouble();
        auto wowPhase = rng.nextDouble() * twoPi;
        auto n = b.getNumSamples();

        for (int ch = 0; ch < b.getNumChannels(); ++ch)
        {
            DelayLine line ((int) (base + wowDepth + flutterDepth) * 2 + 8);
            std::vector<float> out ((size_t) n);
            auto* d = b.getWritePointer (ch);
            for (int i = 0; i < n + (int) base; ++i)
            {
                line.push (i < n ? d[i] : 0.0f);
                auto t = i / sr;
                auto delay = base + wowDepth * std::sin (twoPi * wowRate * t + wowPhase) + flutterDepth * std::sin (twoPi * flutterRate * t);
                auto j = i - (int) base;
                if (j >= 0 && j < n) out[(size_t) j] = line.read (delay); // centred on `base`, so output lines up
            }
            std::copy (out.begin(), out.end(), d);
        }
    }

    void tapeHiss (juce::AudioBuffer<float>& b, double sr, double levelDb, juce::Random& rng)
    {
        auto peak = peakOf (b);
        if (peak <= 0) return;
        juce::AudioBuffer<float> noise (b.getNumChannels(), b.getNumSamples());
        for (int ch = 0; ch < noise.getNumChannels(); ++ch)
            for (int i = 0; i < noise.getNumSamples(); ++i)
                noise.setSample (ch, i, rng.nextFloat() * 2.0f - 1.0f);
        filterBuffer (noise, sr, FilterKind::High, 1500.0, 2, 0.0);
        filterBuffer (noise, sr, FilterKind::Low, 9000.0, 2, 0.0);
        auto np = peakOf (noise);
        if (np > 0) noise.applyGain (peak * juce::Decibels::decibelsToGain ((float) levelDb) / np * 3.0f);
        for (int ch = 0; ch < b.getNumChannels(); ++ch)
            b.addFrom (ch, 0, noise, ch, 0, b.getNumSamples());
    }

    void makeStereo (juce::AudioBuffer<float>& b)
    {
        if (b.getNumChannels() == 1)
        {
            juce::AudioBuffer<float> s (2, b.getNumSamples());
            s.copyFrom (0, 0, b, 0, 0, b.getNumSamples());
            s.copyFrom (1, 0, b, 0, 0, b.getNumSamples());
            b = s;
        }
    }

    void chorus (juce::AudioBuffer<float>& b, double sr, double rate, double depth, double mix)
    {
        makeStereo (b);
        auto n = b.getNumSamples();
        auto dry = b;
        const double baseMs[2] = { 11.0, 17.0 };
        auto depthSamples = juce::jlimit (0.0, 1.0, depth) * 0.004 * sr;
        for (int ch = 0; ch < 2; ++ch)
        {
            DelayLine taps[2] { DelayLine ((int) (0.03 * sr)), DelayLine ((int) (0.03 * sr)) };
            auto* in = dry.getReadPointer (ch);
            auto* out = b.getWritePointer (ch);
            for (int i = 0; i < n; ++i)
            {
                float wet = 0;
                for (int t = 0; t < 2; ++t)
                {
                    taps[t].push (in[i]);
                    auto lfo = std::sin (twoPi * rate * i / sr + (ch == 0 ? 0.0 : pi * 0.5) + t * pi);
                    wet += taps[t].read (baseMs[t] * 0.001 * sr + depthSamples * (0.5 + 0.5 * lfo));
                }
                out[i] = (float) ((1.0 - mix) * in[i] + mix * 0.5 * wet);
            }
        }
    }

    void widen (juce::AudioBuffer<float>& b, double sr, double width)
    {
        makeStereo (b);
        auto n = b.getNumSamples();
        DelayLine haas ((int) (0.03 * sr));
        auto* l = b.getWritePointer (0);
        auto* r = b.getWritePointer (1);
        for (int i = 0; i < n; ++i)
        {
            auto mid = 0.5f * (l[i] + r[i]);
            auto side = 0.5f * (l[i] - r[i]);
            haas.push (mid);
            // decorrelated side from a short-delayed copy of the mid, so even mono sources spread
            side = side * (float) width + (float) (0.35 * (width - 1.0)) * haas.read (0.012 * sr);
            l[i] = mid + side; r[i] = mid - side;
        }
    }

    void compressor (juce::AudioBuffer<float>& b, double sr, double thresholdDb, double ratio, double attackMs, double releaseMs, double makeupDb)
    {
        auto peak = peakOf (b);
        if (peak <= 0) return;
        b.applyGain (1.0f / peak); // threshold is relative to the sample's peak
        auto att = std::exp (-1.0 / (attackMs * 0.001 * sr)), rel = std::exp (-1.0 / (releaseMs * 0.001 * sr));
        double env = 0;
        for (int i = 0; i < b.getNumSamples(); ++i)
        {
            double level = 0;
            for (int ch = 0; ch < b.getNumChannels(); ++ch) level = std::max (level, (double) std::abs (b.getSample (ch, i)));
            env = level > env ? att * env + (1 - att) * level : rel * env + (1 - rel) * level;
            auto envDb = juce::Decibels::gainToDecibels (env, -120.0);
            auto over = envDb - thresholdDb;
            auto gainDb = (over > 0 ? -over * (1.0 - 1.0 / ratio) : 0.0) + makeupDb;
            auto g = (float) juce::Decibels::decibelsToGain (gainDb);
            for (int ch = 0; ch < b.getNumChannels(); ++ch) b.setSample (ch, i, b.getSample (ch, i) * g);
        }
        b.applyGain (peak);
    }

    void reverb (juce::AudioBuffer<float>& b, double sr, const juce::var& o)
    {
        makeStereo (b);
        auto kind = str (o, "kind", "plate").toLowerCase();
        auto decay = num (o, "decay", kind == "room" ? 0.5 : kind.contains ("hall") ? 2.2 : 1.0);
        auto mix = juce::jlimit (0.0, 1.0, num (o, "mix", 0.2));

        juce::Reverb::Parameters p;
        p.roomSize = (float) juce::jlimit (0.1, 0.98, 0.25 + decay * 0.28);
        p.damping  = (float) (kind == "darkhall" ? 0.85 : kind == "plate" ? 0.2 : kind == "room" ? 0.45 : 0.5);
        p.width    = 1.0f;
        p.wetLevel = 1.0f;
        p.dryLevel = 0.0f;

        juce::Reverb verb;
        verb.setSampleRate (sr);
        verb.setParameters (p);
        auto wet = b;
        verb.processStereo (wet.getWritePointer (0), wet.getWritePointer (1), wet.getNumSamples());

        // shape the wet signal: plates are bright, dark halls lose their top end
        if (kind == "darkhall") filterBuffer (wet, sr, FilterKind::Low, 2500.0, 2, 0.0);
        if (kind == "room")     filterBuffer (wet, sr, FilterKind::Low, 7000.0, 2, 0.0);
        filterBuffer (wet, sr, FilterKind::High, 150.0, 2, 0.0);

        auto dp = peakOf (b), wp = peakOf (wet);
        if (wp > 0) wet.applyGain (dp / wp * 0.6f);
        for (int ch = 0; ch < 2; ++ch)
        {
            b.applyGain (ch, 0, b.getNumSamples(), (float) (1.0 - 0.5 * mix));
            b.addFrom (ch, 0, wet, ch, 0, b.getNumSamples(), (float) mix);
        }
    }

    double syncedTime (const juce::var& o, double tempo)
    {
        auto sync = str (o, "sync", "");
        if (sync.isNotEmpty())
        {
            auto beat = 60.0 / juce::jmax (20.0, tempo);
            auto parts = juce::StringArray::fromTokens (sync.removeCharacters ("dD."), "/", "");
            auto value = parts.size() == 2 ? 4.0 * parts[0].getDoubleValue() / juce::jmax (1.0, parts[1].getDoubleValue()) : 1.0;
            if (sync.endsWithIgnoreCase ("d") || sync.endsWith ("."))
                value *= 1.5; // dotted
            return beat * value;
        }
        return num (o, "timeMs", 250.0) * 0.001;
    }

    void delay (juce::AudioBuffer<float>& b, double sr, const juce::var& o, double tempo, bool shimmer)
    {
        makeStereo (b);
        auto time = syncedTime (o, tempo) * sr;
        auto feedback = juce::jlimit (0.0, 0.95, num (o, "feedback", 0.15));
        auto mix = juce::jlimit (0.0, 1.0, num (o, "mix", 0.25));
        auto tape = shimmer || (bool) o.getProperty ("tape", true);
        auto semis = num (o, "semitones", 12.0);

        for (int ch = 0; ch < 2; ++ch)
        {
            DelayLine line ((int) (time + 0.02 * sr) + 8);
            PitchShifter shifter (sr, semis);
            float lp = 0;
            auto* d = b.getWritePointer (ch);
            auto offset = ch == 1 && shimmer ? 0.013 * sr : 0.0; // a little stereo spread on the shimmer
            for (int i = 0; i < b.getNumSamples(); ++i)
            {
                auto wobble = tape ? 0.0006 * sr * std::sin (twoPi * 0.7 * i / sr) : 0.0;
                auto echo = line.read (time + offset + wobble);
                auto fb = shimmer ? shifter.process (echo) : echo;
                if (tape) { lp += 0.35f * (fb - lp); fb = lp; } // each repeat a bit darker
                line.push (d[i] + (float) feedback * fb);
                d[i] = (float) ((1.0 - 0.5 * mix) * d[i] + mix * echo);
            }
        }
    }

    void rotary (juce::AudioBuffer<float>& b, double sr, const juce::var& o)
    {
        makeStereo (b);
        auto fast = str (o, "speed", "fast").equalsIgnoreCase ("fast");
        auto hornRate = fast ? 6.8 : 0.8, drumRate = fast ? 5.9 : 0.7;
        auto mix = juce::jlimit (0.0, 1.0, num (o, "mix", 1.0));
        auto n = b.getNumSamples();
        auto dry = b;

        // split at ~800 Hz: horn (top) and drum (bottom) rotate at different speeds
        juce::AudioBuffer<float> mono (1, n);
        mono.copyFrom (0, 0, dry, 0, 0, n);
        mono.applyGain (0.5f);
        mono.addFrom (0, 0, dry, 1, 0, n, 0.5f);
        auto low = mono;
        filterBuffer (low, sr, FilterKind::Low, 800.0, 2, 0.0);
        auto high = mono;
        filterBuffer (high, sr, FilterKind::High, 800.0, 2, 0.0);

        DelayLine horn ((int) (0.01 * sr));
        auto* l = b.getWritePointer (0);
        auto* r = b.getWritePointer (1);
        for (int i = 0; i < n; ++i)
        {
            auto t = i / sr;
            auto hp = twoPi * hornRate * t, dp = twoPi * drumRate * t;
            horn.push (high.getSample (0, i));
            auto doppler = horn.read (0.0015 * sr + 0.0003 * sr * std::sin (hp));
            auto hornL = doppler * (float) (0.65 + 0.35 * std::cos (hp)), hornR = doppler * (float) (0.65 - 0.35 * std::cos (hp));
            auto drum = low.getSample (0, i) * (float) (0.85 + 0.15 * std::sin (dp));
            l[i] = (float) ((1.0 - mix) * l[i] + mix * (drum + hornL));
            r[i] = (float) ((1.0 - mix) * r[i] + mix * (drum + hornR));
        }
    }
}

//==============================================================================
juce::StringArray effectTypes()
{
    return { "lowpass", "highpass", "bandpass", "peak", "tape", "tube", "overdrive", "bitcrush", "wowflutter",
             "hiss", "chorus", "widen", "compressor", "reverb", "delay", "shimmer", "rotary", "gain" };
}

bool validateFx (const juce::var& fx, juce::String& error)
{
    if (fx.isVoid()) return true;
    if (! fx.isArray()) { error = "\"fx\" must be an array of effects"; return false; }
    for (auto& e : *fx.getArray())
    {
        auto type = e["type"].toString().toLowerCase();
        if (! effectTypes().contains (type))
        {
            error = "unknown effect '" + type + "' (known: " + effectTypes().joinIntoString (", ") + ")";
            return false;
        }
    }
    return true;
}

void varispeed (juce::AudioBuffer<float>& b, double semitones)
{
    if (std::abs (semitones) < 1e-6 || b.getNumSamples() < 4) return;
    auto speed = std::pow (2.0, semitones / 12.0);
    auto outLen = (int) std::floor ((b.getNumSamples() - 2) / speed);
    juce::AudioBuffer<float> out (b.getNumChannels(), juce::jmax (1, outLen));
    for (int ch = 0; ch < b.getNumChannels(); ++ch)
    {
        juce::WindowedSincInterpolator interp;
        interp.process (speed, b.getReadPointer (ch), out.getWritePointer (ch), out.getNumSamples(), b.getNumSamples(), 0);
    }
    b = out;
}

void applyAmpEnvelope (juce::AudioBuffer<float>& b, double sr, double attackMs, double decayMs)
{
    auto n = b.getNumSamples();
    auto attack = (int) (attackMs * 0.001 * sr);
    if (attack > 0)
        for (int ch = 0; ch < b.getNumChannels(); ++ch)
            b.applyGainRamp (ch, 0, juce::jmin (attack, n), 0.0f, 1.0f);

    if (decayMs > 0)
    {
        // exponential curve that lands exactly on silence at decayMs
        const double k = 3.0;
        auto T = decayMs * 0.001 * sr;
        auto floorV = std::exp (-k);
        for (int i = 0; i < n; ++i)
        {
            auto x = juce::jmin (1.0, i / T);
            auto g = (float) ((std::exp (-k * x) - floorV) / (1.0 - floorV));
            for (int ch = 0; ch < b.getNumChannels(); ++ch)
                b.setSample (ch, i, b.getSample (ch, i) * g);
        }
    }
}

void applyFxChain (juce::AudioBuffer<float>& b, double sr, const juce::var& fx, double tempo, juce::int64 seed)
{
    if (! fx.isArray()) return;
    juce::Random rng (seed);

    for (auto& o : *fx.getArray())
    {
        auto type = o["type"].toString().toLowerCase();

        if (type == "lowpass")        filterBuffer (b, sr, FilterKind::Low,  num (o, "freq", 5000), (int) num (o, "poles", 2), num (o, "resonance", 0));
        else if (type == "highpass")  filterBuffer (b, sr, FilterKind::High, num (o, "freq", 150),  (int) num (o, "poles", 2), num (o, "resonance", 0));
        else if (type == "bandpass")
        {
            filterBuffer (b, sr, FilterKind::High, num (o, "low", 800), (int) num (o, "poles", 2), 0);
            filterBuffer (b, sr, FilterKind::Low,  num (o, "high", 3500), (int) num (o, "poles", 2), 0);
        }
        else if (type == "peak")       peakEq (b, sr, num (o, "freq", 1000), num (o, "gainDb", 3), num (o, "q", 1.0));
        else if (type == "tape")       tapeSaturation (b, sr, num (o, "drive", 0.3));
        else if (type == "tube")       tubeOverdrive (b, num (o, "drive", 0.3));
        else if (type == "overdrive")  overdrive (b, sr, num (o, "drive", 0.4), num (o, "tone", 6000));
        else if (type == "bitcrush")   bitcrush (b, (int) num (o, "bits", 12), (int) num (o, "downsample", 1));
        else if (type == "wowflutter") wowFlutter (b, sr, num (o, "amount", 0.3), rng);
        else if (type == "hiss")       tapeHiss (b, sr, num (o, "levelDb", -42), rng);
        else if (type == "chorus")     chorus (b, sr, num (o, "rate", 0.8), num (o, "depth", 0.25), num (o, "mix", 0.5));
        else if (type == "widen")      widen (b, sr, num (o, "width", 1.6));
        else if (type == "compressor") compressor (b, sr, num (o, "thresholdDb", -12), num (o, "ratio", 4), num (o, "attackMs", 1),
                                                   num (o, "releaseMs", 80), num (o, "makeupDb", 0));
        else if (type == "reverb")     reverb (b, sr, o);
        else if (type == "delay")      delay (b, sr, o, tempo, false);
        else if (type == "shimmer")    delay (b, sr, o, tempo, true);
        else if (type == "rotary")     rotary (b, sr, o);
        else if (type == "gain")       b.applyGain (juce::Decibels::decibelsToGain ((float) num (o, "db", 0)));
    }
}
}
