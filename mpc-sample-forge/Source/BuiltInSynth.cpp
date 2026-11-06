#include "BuiltInSynth.h"

namespace forge
{
static const std::vector<std::tuple<Engine, const char*, const char*>>& engineTable()
{
    static const std::vector<std::tuple<Engine, const char*, const char*>> t {
        { Engine::EPiano,   "epiano",   "EPiano" },
        { Engine::Bell,     "bell",     "Bell" },
        { Engine::Organ,    "organ",    "Organ" },
        { Engine::Pluck,    "pluck",    "Pluck" },
        { Engine::Choir,    "choir",    "Choir" },
        { Engine::Supersaw, "supersaw", "Synth" },
        { Engine::EPBell,     "epbell",     "EPBell" },
        { Engine::Triangle,   "triangle",   "Triangle" },
        { Engine::Cowbell,    "cowbell",    "Cowbell" },
        { Engine::ComboOrgan, "comboorgan", "ComboOrgan" },
        { Engine::Harp,       "harp",       "Harp" },
        { Engine::Brass,      "brass",      "Brass" },
    };
    return t;
}

juce::StringArray engineIds()
{
    juce::StringArray s;
    for (auto& [r, id, name] : engineTable()) s.add (id);
    return s;
}

juce::String engineId (Engine r)
{
    for (auto& [rr, id, name] : engineTable()) if (rr == r) return id;
    return "epiano";
}

juce::String engineDisplayName (Engine r)
{
    for (auto& [rr, id, name] : engineTable()) if (rr == r) return name;
    return "EPiano";
}

bool parseEngine (const juce::String& s, Engine& out)
{
    for (auto& [r, id, name] : engineTable())
        if (s.equalsIgnoreCase (id) || s.equalsIgnoreCase (name)) { out = r; return true; }
    return false;
}

//==============================================================================
namespace
{
    constexpr double twoPi = juce::MathConstants<double>::twoPi;

    /** Topology-preserving state variable filter (Simper). */
    struct Svf
    {
        float a1 = 0, a2 = 0, a3 = 0, k = 1, ic1 = 0, ic2 = 0;

        void set (double fc, double q, double sr)
        {
            fc = juce::jlimit (10.0, sr * 0.45, fc);
            auto g = (float) std::tan (juce::MathConstants<double>::pi * fc / sr);
            k = (float) (1.0 / q);
            a1 = 1.0f / (1.0f + g * (g + k));
            a2 = g * a1;
            a3 = g * a2;
        }
        void clear() { ic1 = ic2 = 0; }

        /** returns { lowpass, bandpass } */
        std::pair<float, float> process (float x)
        {
            auto v3 = x - ic2;
            auto v1 = a1 * ic1 + a2 * v3;
            auto v2 = ic2 + a2 * ic1 + a3 * v3;
            ic1 = 2 * v1 - ic1;
            ic2 = 2 * v2 - ic2;
            return { v2, v1 };
        }
    };

    inline float polyBlep (double t, double dt)
    {
        if (t < dt)       { t /= dt;           return (float) (t + t - t * t - 1.0); }
        if (t > 1.0 - dt) { t = (t - 1.0) / dt; return (float) (t * t + t + t + 1.0); }
        return 0.0f;
    }

    inline float blepSaw (double& phase, double inc)
    {
        auto v = (float) (2.0 * phase - 1.0) - polyBlep (phase, inc);
        phase += inc;
        if (phase >= 1.0) phase -= 1.0;
        return v;
    }

    /** Band-limited square / pulse made from two saws. */
    inline float blepPulse (double& phase, double inc, double width = 0.5)
    {
        auto a = (float) (2.0 * phase - 1.0) - polyBlep (phase, inc);
        auto p2 = phase + width; if (p2 >= 1.0) p2 -= 1.0;
        auto b = (float) (2.0 * p2 - 1.0) - polyBlep (p2, inc);
        phase += inc; if (phase >= 1.0) phase -= 1.0;
        return 0.5f * (a - b);
    }

    // Hammond-style registrations: 16' 5 1/3' 8' 4' 2 2/3' 2' 1 3/5' 1 1/3' 1'
    const int registrations[][9] = {
        { 8, 8, 8, 0, 0, 0, 0, 0, 0 },
        { 8, 8, 8, 8, 0, 0, 0, 0, 0 },
        { 8, 0, 8, 8, 0, 8, 0, 0, 8 },
        { 8, 6, 8, 8, 6, 8, 4, 4, 6 },
        { 0, 0, 8, 0, 0, 0, 8, 8, 8 },
        { 6, 8, 8, 6, 0, 0, 0, 0, 0 },
        { 0, 0, 8, 8, 8, 0, 0, 0, 0 },   // every registration keeps the 8' so the pitch is unambiguous
        { 6, 6, 8, 0, 0, 0, 0, 0, 4 },
    };
    const double drawbarRatios[9] = { 0.5, 1.5, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 8.0 };

    // Vowel formants (Hz) and relative gains: aah, ooh, oh, eh, mm-ish
    const float vowels[][3] = {
        { 800, 1150, 2900 }, { 325, 700, 2700 }, { 450, 800, 2830 }, { 400, 1700, 2600 }, { 350, 900, 2400 },
    };
}

//==============================================================================
class BuiltInSynth::Voice final : public juce::SynthesiserVoice
{
public:
    explicit Voice (BuiltInSynth& o) : owner (o) {}

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<Sound*> (s) != nullptr; }
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void setCurrentPlaybackSampleRate (double newRate) override
    {
        SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);
        if (newRate > 0) delay.assign ((size_t) (newRate / 15.0) + 8, 0.0f); // Karplus-Strong line, down to ~15 Hz
    }

    void startNote (int note, float vel, juce::SynthesiserSound*, int) override
    {
        engine = owner.getEngine();
        sr = getSampleRate();
        freq = juce::MidiMessage::getMidiNoteInHertz (note);
        velocity = vel;
        time = 0;
        releasing = false;
        releaseTime = 0;
        releaseStartLevel = 1;
        amp = 0;
        done = false;

        // Timbre comes from the sample's variation seed (same for every note of a chord);
        // tiny per-note randomness comes from a separate generator.
        juce::Random v (owner.getVariation());
        bright = v.nextFloat(); decayAmt = v.nextFloat(); detune = v.nextFloat(); colour = v.nextFloat(); extra = v.nextFloat();
        juce::Random perNote (owner.getVariation() * 31 + note);

        auto noteScale = juce::jmap ((float) juce::jlimit (24, 108, note), 24.0f, 108.0f, 1.4f, 0.5f); // low notes ring longer
        level = 0.2f * (0.3f + 0.7f * vel);
        pan = juce::jmap ((float) note, 24.0f, 108.0f, -0.3f, 0.3f);

        for (auto& p : phases) p = perNote.nextDouble();
        phase = modPhase = 0;

        switch (engine)
        {
            case Engine::EPiano:
                decayTau = (1.5 + 2.5 * decayAmt) * noteScale;
                break;

            case Engine::Bell:
            {
                // ratios >= 2 keep every FM partial at or above the root, so the pitch reads clearly
                static const double ratios[] = { 3.5, 2.0, 2.76, 4.0, 5.19, 3.0 };
                modRatio = ratios[(int) (colour * 5.99f)];
                decayTau = (0.7 + 2.3 * decayAmt) * noteScale;
                break;
            }

            case Engine::Organ:
            {
                auto& reg = registrations[(int) (colour * 7.99f)];
                float sum = 0;
                for (size_t i = 0; i < 9; ++i) { drawbars[i] = (float) reg[i] / 8.0f; sum += drawbars[i]; }
                for (auto& d : drawbars) d /= juce::jmax (1.0f, sum * 0.6f);
                percussion = extra > 0.4f;
                clickLevel = 0.15f + 0.5f * bright;
                break;
            }

            case Engine::EPBell:
                decayTau = (1.2 + 1.8 * decayAmt) * noteScale;
                modRatio = 3.5;
                break;

            case Engine::Triangle:
                filter.clear(); filter2.clear();
                decayTau = (0.25 + 0.5 * decayAmt) * noteScale;
                break;

            case Engine::Cowbell:
                filter.clear();
                decayTau = 0.05 + 0.08 * decayAmt;
                break;

            case Engine::ComboOrgan:
                vibrato = extra > 0.5f;
                break;

            case Engine::Brass:
                filter.clear();
                attackSec = 0.012 + 0.03 * (1.0 - bright);
                decayTau = (0.4 + 0.8 * decayAmt) * noteScale;
                break;

            case Engine::Harp:
                // a concert harp is a bright, long-ringing plucked string
                bright = 0.3f + 0.3f * bright;      // gut / nylon strings: warm, not buzzy
                decayAmt = 0.55f + 0.45f * decayAmt;
                [[fallthrough]];

            case Engine::Pluck:
            {
                // Karplus-Strong: loop delay = sr / f, minus the half sample of the averaging filter
                period = juce::jlimit (2.0, (double) delay.size() - 6, sr / freq - 1.0);
                std::fill (delay.begin(), delay.end(), 0.0f);
                writePos = 0;
                auto t60 = (0.4 + 2.6 * decayAmt) * noteScale;
                feedback = (float) std::pow (0.001, 1.0 / (freq * t60));
                damping = 0.2f + 0.75f * bright;   // 1 = no loop filtering (bright)
                // excite with filtered noise
                float lp = 0;
                auto excite = (int) std::ceil (period) + 1;
                auto smooth = 0.15f + 0.85f * bright;
                float mean = 0;
                for (int i = 0; i < excite; ++i)
                {
                    lp += smooth * ((perNote.nextFloat() * 2.0f - 1.0f) - lp);
                    delay[(size_t) i] = lp;
                    mean += lp;
                }
                mean /= (float) excite;
                for (int i = 0; i < excite; ++i) delay[(size_t) i] -= mean; // no DC: it would ring forever
                writePos = excite;
                peakTracker = 1.0f;
                dcX = dcY = 0;
                break;
            }

            case Engine::Choir:
            {
                auto& vw = vowels[(int) (colour * 4.99f)];
                for (int i = 0; i < 3; ++i)
                {
                    formant[i].clear();
                    formant[i].set (vw[i] * (0.95 + 0.1 * extra), vw[i] / (60.0 + 40.0 * i), sr);
                }
                attackSec = 0.02 + 0.08 * (1.0 - bright);
                break;
            }

            case Engine::Supersaw:
                filter.clear();
                attackSec = 0.002;
                decayTau = (0.15 + 0.9 * decayAmt) * noteScale;
                break;
        }
    }

    void stopNote (float, bool allowTail) override
    {
        if (! allowTail) { clearCurrentNote(); return; }
        releasing = true;
        releaseTime = 0;
        releaseStartLevel = amp;
    }

    using SynthesiserVoice::renderNextBlock;

    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int num) override
    {
        if (! isVoiceActive()) return;

        const auto dt = 1.0 / sr;
        const auto gl = std::sqrt (0.5f * (1.0f - pan)), gr = std::sqrt (0.5f * (1.0f + pan));

        for (int i = 0; i < num; ++i)
        {
            float l = 0, r = 0;
            auto s = renderSample (l, r);

            // release envelope common to all recipes
            float rel = 1.0f;
            if (releasing)
            {
                auto relTime = (engine == Engine::Organ || engine == Engine::ComboOrgan) ? 0.05 : engine == Engine::Choir ? 0.3 : 0.25;
                rel = (float) std::exp (-releaseTime / (relTime / 4.0));
                releaseTime += dt;
            }

            s *= rel; l *= rel; r *= rel;
            out.addSample (0, start + i, (s * gl + l) * level);
            if (out.getNumChannels() > 1) out.addSample (1, start + i, (s * gr + r) * level);

            time += dt;

            if ((releasing && rel < 0.0003f) || done)
            {
                clearCurrentNote();
                break;
            }
        }
    }

private:
    /** Returns a mono sample; stereo-only content is added into l / r. */
    float renderSample (float& l, float& r)
    {
        const auto t = time;
        const auto inc = freq / sr;

        switch (engine)
        {
            case Engine::EPiano:
            {
                auto env = (float) std::exp (-t / decayTau);
                auto modEnv = (float) std::exp (-t / (0.3 * decayTau));
                auto index = (0.6 + 2.2 * bright) * (0.4 + 0.6 * velocity) * modEnv;
                auto tine = 0.15f * bright * (float) std::sin (phase * twoPi * 7.0) * (float) std::exp (-t / 0.08);
                auto s = (float) std::sin (twoPi * phase + index * std::sin (twoPi * phase)) + tine;
                phase += inc; if (phase >= 1.0) phase -= 1.0;
                auto attack = (float) juce::jmin (1.0, t / 0.002);
                if (env < 0.0003f) done = true;
                return s * env * attack;
            }

            case Engine::Bell:
            {
                auto env = (float) std::exp (-t / decayTau);
                auto modEnv = (float) std::exp (-t / (0.25 * decayTau));
                auto index = (1.0 + 4.0 * bright) * (0.5 + 0.5 * velocity) * modEnv;
                auto s = (float) std::sin (twoPi * phase + index * std::sin (twoPi * modPhase));
                // a fast-decaying strike partial gives the "tine" attack
                s += 0.25f * (float) std::sin (twoPi * phase * 2.76) * (float) std::exp (-t / 0.12);
                phase += inc;            if (phase >= 1.0) phase -= 1.0;
                modPhase += inc * modRatio; if (modPhase >= 1.0) modPhase -= 1.0;
                auto attack = (float) juce::jmin (1.0, t / 0.001);
                if (env < 0.0003f) done = true;
                return s * env * attack * 0.8f;
            }

            case Engine::Organ:
            {
                float s = 0;
                for (size_t i = 0; i < 9; ++i)
                    if (drawbars[i] > 0)
                        s += drawbars[i] * (float) std::sin (twoPi * phase * drawbarRatios[i]);
                if (percussion)
                    s += 0.6f * (float) std::sin (twoPi * phase * (extra > 0.7f ? 3.0 : 2.0)) * (float) std::exp (-t / 0.18);
                if (t < 0.006) // key click
                    s += clickLevel * (clickRng.nextFloat() * 2.0f - 1.0f) * (float) (1.0 - t / 0.006);
                phase += inc; if (phase >= 8.0) phase -= 8.0; // keep sub-harmonics continuous
                amp = (float) juce::jmin (1.0, t / 0.004);
                return s * amp * 0.7f;
            }

            case Engine::EPBell:
            {
                // Rhodes-style FM tine layered with a soft 2-operator sine bell
                auto env = (float) std::exp (-t / decayTau);
                auto modEnv = (float) std::exp (-t / (0.3 * decayTau));
                auto index = (0.5 + 1.6 * bright) * (0.4 + 0.6 * velocity) * modEnv;
                auto ep = (float) std::sin (twoPi * phase + index * std::sin (twoPi * phase));
                auto bellIndex = (0.6 + 1.2 * colour) * std::exp (-t / (0.18 * decayTau));
                auto bell = (float) std::sin (twoPi * phase * 2.0 + bellIndex * std::sin (twoPi * modPhase)) * (float) std::exp (-t / (0.5 * decayTau));
                phase += inc; if (phase >= 1.0) phase -= 1.0;
                modPhase += inc * modRatio * 2.0; if (modPhase >= 1.0) modPhase -= 1.0;
                auto attack = (float) juce::jmin (1.0, t / 0.0015);
                if (env < 0.0003f) done = true;
                return (ep + 0.35f * bell) * env * attack;
            }

            case Engine::Triangle:
            {
                // triangle + sub-octave sine, 4-pole low-pass with an instant-attack filter envelope
                auto tri = (float) (phase < 0.5 ? 4.0 * phase - 1.0 : 3.0 - 4.0 * phase);
                auto sub = (float) std::sin (twoPi * modPhase);
                phase += inc; if (phase >= 1.0) phase -= 1.0;
                modPhase += inc * 0.5; if (modPhase >= 1.0) modPhase -= 1.0;
                auto fenv = std::exp (-t / 0.06); // ~180 ms to settle
                auto cutoff = 2800.0 * (1.0 + 0.45 * 10.0 * fenv * (0.5 + bright));
                filter.set (cutoff, 0.54, sr);
                filter2.set (cutoff, 1.31, sr);
                auto x = filter2.process (filter.process (tri + 0.5f * sub).first).first;
                auto env = (float) std::exp (-t / decayTau);
                if (env < 0.0003f) done = true;
                return x * env;
            }

            case Engine::Cowbell:
            {
                // TR-808 style: two square waves 800/540 apart, band-limited, two-stage decay.
                // The upper tone is the played note: it dominates after the 800 Hz-3.5 kHz band-pass,
                // so that is the pitch you hear (G5 gives the classic 784 / 529 Hz pair).
                auto a = blepPulse (phases[0], inc);
                auto b = blepPulse (phases[1], inc / 1.4815);
                filter.set (freq * 1.2, 1.0, sr);
                auto x = filter.process (a + b).second;
                auto env = (float) (0.75 * std::exp (-t / decayTau) + 0.25 * std::exp (-t / (decayTau * 6.0)));
                auto attack = (float) juce::jmin (1.0, t / 0.0005);
                if (env < 0.0003f) done = true;
                return x * env * attack * 1.4f;
            }

            case Engine::ComboOrgan:
            {
                // transistor combo organ (Farfisa / Vox): bright pulse stops, 8' + 4' (+ 2')
                auto vib = vibrato ? 1.0 + 0.003 * std::sin (twoPi * 6.2 * t) : 1.0;
                auto s8 = blepPulse (phases[0], inc * vib, 0.5);
                auto s4 = blepPulse (phases[1], inc * 2.0 * vib, 0.35 + 0.15 * colour);
                auto s2 = blepPulse (phases[2], inc * 4.0 * vib, 0.5);
                amp = (float) juce::jmin (1.0, t / 0.003);
                return (s8 + 0.55f * s4 + 0.25f * bright * s2) * amp * 0.5f;
            }

            case Engine::Brass:
            {
                // bright 80s analog brass (detuned saws) with a horn-like soft layer, filter swell
                float saws = 0;
                const double spread[] = { -1.0, 0.0, 1.0 };
                for (int v = 0; v < 3; ++v)
                    saws += blepSaw (phases[(size_t) v], inc * (1.0 + spread[v] * (0.002 + 0.003 * detune)));
                auto horn = (float) (std::sin (twoPi * phases[3]) + 0.4 * std::sin (2.0 * twoPi * phases[3]));
                phases[3] += inc; if (phases[3] >= 1.0) phases[3] -= 1.0;
                auto swell = juce::jmin (1.0, t / (attackSec * 2.5));
                auto cutoff = freq * (1.5 + (3.0 + 7.0 * bright) * swell * (0.6 + 0.4 * std::exp (-t / 0.25)));
                filter.set (cutoff, 0.9, sr);
                auto x = filter.process (saws * 0.33f + 0.5f * horn).first;
                amp = (float) (juce::jmin (1.0, t / attackSec) * (0.7 + 0.3 * std::exp (-t / decayTau)));
                return x * amp * 1.2f;
            }

            case Engine::Harp:
            case Engine::Pluck:
            {
                // Read three consecutive delayed samples and apply the symmetric
                // (linear-phase) loop filter [c, 1-2c, c]: its delay is exactly one
                // sample at every frequency, so tuning stays exact.
                auto size = (int) delay.size();
                auto readPos = (double) writePos - period;
                while (readPos < 0) readPos += size;
                auto i0 = (int) readPos;
                auto frac = (float) (readPos - i0);
                auto tap = [&] (int back)
                {
                    auto j = ((i0 - back) % size + size) % size;
                    auto k = (j + 1) % size;
                    return delay[(size_t) j] + frac * (delay[(size_t) k] - delay[(size_t) j]);
                };
                auto c = 0.25f * (1.0f - damping);
                auto y = feedback * (c * tap (0) + (1.0f - 2.0f * c) * tap (1) + c * tap (2));
                delay[(size_t) writePos] = y;
                writePos = (writePos + 1) % size;
                peakTracker = juce::jmax (std::abs (y), peakTracker * 0.9995f);
                if (t > 0.05 && peakTracker < 0.0002f) done = true;
                // DC blocker on the output
                auto blocked = y - dcX + 0.995f * dcY;
                dcX = y; dcY = blocked;
                return blocked * 1.6f;
            }

            case Engine::Choir:
            {
                auto vib = 1.0 + 0.0025 * std::sin (twoPi * 5.2 * t) * juce::jlimit (0.0, 1.0, (t - 0.15) / 0.4);
                float src = 0;
                const double spread[] = { -1.0, -0.33, 0.33, 1.0 };
                for (int v = 0; v < 4; ++v)
                    src += blepSaw (phases[(size_t) v], inc * vib * (1.0 + spread[v] * (0.001 + 0.003 * detune)));
                src *= 0.25f;
                float s = 0;
                const float g[] = { 1.0f, 0.7f, 0.35f };
                for (int f = 0; f < 3; ++f) s += g[f] * formant[f].process (src).second;
                amp = (float) juce::jmin (1.0, t / attackSec);
                return s * amp * 2.5f;
            }

            case Engine::Supersaw:
            {
                const double spread[] = { -1.0, -0.55, -0.2, 0.0, 0.2, 0.55, 1.0 };
                auto cents = 3.0 + 15.0 * detune;
                float left = 0, right = 0, centre = 0;
                for (int v = 0; v < 7; ++v)
                {
                    auto ratio = std::pow (2.0, spread[v] * cents / 1200.0);
                    auto sample = blepSaw (phases[(size_t) v], inc * ratio);
                    if (v == 3) centre += sample;
                    else if (v % 2 == 0) left += sample;
                    else right += sample;
                }
                auto filterEnv = std::exp (-t / (0.08 + 0.4 * extra));
                auto cutoff = freq * (1.2 + (2.0 + 30.0 * bright) * filterEnv) + 150.0;
                filter.set (cutoff, 0.8 + 1.5 * colour, sr);
                auto mono = filter.process ((1.5f * centre + 0.5f * (left + right)) * 0.25f).first;
                // stereo detune: keep the sides a bit wider than the mono signal
                auto side = 0.15f * (left - right) * (float) filterEnv;
                auto env = (float) (0.55 + 0.45 * std::exp (-t / decayTau));
                amp = (float) juce::jmin (1.0, t / attackSec) * env;
                l = side * amp; r = -side * amp;
                return mono * amp * 1.3f;
            }
        }
        return 0;
    }

    BuiltInSynth& owner;
    Engine engine = Engine::EPiano;
    double sr = 44100, freq = 440, time = 0, releaseTime = 0, phase = 0, modPhase = 0, modRatio = 3.5;
    double decayTau = 2, attackSec = 0.002, period = 100;
    float velocity = 1, level = 0.2f, pan = 0, amp = 0, releaseStartLevel = 1;
    float bright = 0.5f, decayAmt = 0.5f, detune = 0.5f, colour = 0.5f, extra = 0.5f;
    bool releasing = false, done = false, percussion = false;
    std::array<double, 8> phases {};
    std::array<float, 9> drawbars {};
    float clickLevel = 0.2f;
    juce::Random clickRng;
    std::vector<float> delay;
    int writePos = 0;
    float feedback = 0.99f, damping = 0.5f, peakTracker = 1.0f, dcX = 0, dcY = 0;
    // (peakTracker resets per note in startNote)
    Svf formant[3], filter, filter2;
    bool vibrato = false;
};

//==============================================================================
BuiltInSynth::BuiltInSynth()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    for (int i = 0; i < 24; ++i) synth.addVoice (new Voice (*this));
    synth.addSound (new Sound());
}

void BuiltInSynth::getStateInformation (juce::MemoryBlock& mb)
{
    juce::XmlElement xml ("BuiltInSynth");
    xml.setAttribute ("engine", engineId (getEngine()));
    xml.setAttribute ("variation", juce::String (getVariation()));
    copyXmlToBinary (xml, mb);
}

void BuiltInSynth::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
    {
        Engine r;
        if (parseEngine (xml->getStringAttribute ("engine"), r)) setEngine (r);
        setVariation (xml->getStringAttribute ("variation", "1").getLargeIntValue());
    }
}
}
