// A tiny VST3 instrument used to test MPC Sample Forge's preset sweep against a real
// plugin: 12 programs, each built to exercise one check (duplicates, silence, arps, FX,
// slow attack, harshness ...), plus a few continuous parameters for "variations".
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>

namespace
{
    struct Patch
    {
        const char* name;
        int wave;          // 0 sine+harmonics (EP), 1 bell (inharmonic), 2 saw, 3 noise, 4 sub sine
        float bright;      // harmonic amount
        float attackSec, decaySec, sustain;
        float gateHz;      // > 0: chops the sound like an arp / trance gate
        float level;
    };

    const Patch patches[] = {
        { "EP Soft Keys",        0, 0.35f, 0.004f, 2.5f, 0.55f, 0, 1 },
        { "Rhodes Classic",      0, 0.60f, 0.003f, 2.0f, 0.50f, 0, 1 },
        { "Bell Glass",          1, 0.50f, 0.002f, 1.2f, 0.00f, 0, 1 },
        { "Init",                2, 1.00f, 0.002f, 0.1f, 1.00f, 0, 1 },
        { "Arp Pulse",           2, 0.50f, 0.002f, 0.1f, 0.80f, 8, 1 },
        { "Bubbles",             0, 0.40f, 0.002f, 0.2f, 0.90f, 7, 1 },
        { "Silent Pad",          0, 0.30f, 0.010f, 1.0f, 0.50f, 0, 0 },
        { "Noise Wash FX",       3, 0.50f, 0.300f, 2.0f, 0.70f, 0, 1 },
        { "Deep Sub",            4, 0.00f, 0.005f, 1.5f, 0.80f, 0, 1 },
        { "Rhodes Classic Copy", 0, 0.60f, 0.003f, 2.0f, 0.50f, 0, 1 },
        { "Bright Saw Lead",     2, 1.00f, 0.002f, 0.5f, 0.90f, 0, 1 },
        { "Slow Strings",        2, 0.30f, 0.700f, 2.0f, 0.90f, 0, 1 },
    };
    constexpr int numPatches = (int) (sizeof (patches) / sizeof (patches[0]));
}

class ForgeTestSynth final : public juce::AudioProcessor
{
public:
    ForgeTestSynth()
        : AudioProcessor (BusesProperties().withOutput ("Out", juce::AudioChannelSet::stereo(), true))
    {
        addParameter (brightness = new juce::AudioParameterFloat ({ "brightness", 1 }, "Brightness", 0.0f, 1.0f, 0.5f));
        addParameter (decay      = new juce::AudioParameterFloat ({ "decay", 1 }, "Decay", 0.0f, 1.0f, 0.5f));
        addParameter (cutoff     = new juce::AudioParameterFloat ({ "cutoff", 1 }, "Filter Cutoff", 0.0f, 1.0f, 0.7f));
        addParameter (volume     = new juce::AudioParameterFloat ({ "volume", 1 }, "Volume", 0.0f, 1.0f, 0.8f));
    }

    const juce::String getName() const override { return "ForgeTestSynth"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isSynth() const { return true; }
    double getTailLengthSeconds() const override { return 1.0; }
    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

    int getNumPrograms() override { return numPatches; }
    int getCurrentProgram() override { return program; }
    void setCurrentProgram (int p) override { program = juce::jlimit (0, numPatches - 1, p); }
    const juce::String getProgramName (int p) override { return juce::isPositiveAndBelow (p, numPatches) ? patches[p].name : ""; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& data) override
    {
        juce::MemoryOutputStream out (data, false);
        out.writeInt (program);
        out.writeFloat (*brightness); out.writeFloat (*decay); out.writeFloat (*cutoff); out.writeFloat (*volume);
    }
    void setStateInformation (const void* d, int size) override
    {
        juce::MemoryInputStream in (d, (size_t) size, false);
        setCurrentProgram (in.readInt());
        *brightness = in.readFloat(); *decay = in.readFloat(); *cutoff = in.readFloat(); *volume = in.readFloat();
    }

    void prepareToPlay (double sr, int) override { sampleRate = sr; for (auto& v : voices) v.active = false; }
    void releaseResources() override {}

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override
    {
        buffer.clear();
        const auto& patch = patches[program];
        int pos = 0;
        auto renderTo = [&] (int end)
        {
            for (auto& v : voices)
                if (v.active)
                    for (int i = pos; i < end; ++i)
                    {
                        auto s = v.next (patch, (float) sampleRate, *brightness, *decay, *cutoff, rng) * *volume * 0.25f * patch.level;
                        buffer.addSample (0, i, s);
                        buffer.addSample (1, i, s);
                    }
            pos = end;
        };
        for (const auto meta : midi)
        {
            renderTo (meta.samplePosition);
            auto m = meta.getMessage();
            if (m.isNoteOn())
            {
                for (auto& v : voices) if (! v.active) { v.start (m.getNoteNumber(), m.getFloatVelocity()); break; }
            }
            else if (m.isNoteOff())
                for (auto& v : voices) if (v.active && v.note == m.getNoteNumber()) v.releasing = true;
            else if (m.isAllNotesOff() || m.isAllSoundOff())
                for (auto& v : voices) v.active = false;
        }
        renderTo (buffer.getNumSamples());
    }

private:
    struct Voice
    {
        bool active = false, releasing = false;
        int note = 60;
        float velocity = 1, env = 0, phase = 0, phase2 = 0, phaseSub = 0, gatePhase = 0, lp = 0;
        double t = 0;

        void start (int n, float vel) { active = true; releasing = false; note = n; velocity = vel; env = 0; phase = phase2 = phaseSub = gatePhase = 0; lp = 0; t = 0; }

        float next (const Patch& p, float sr, float bright, float decayParam, float cutoff, juce::Random& rng)
        {
            const float freq = 440.0f * std::pow (2.0f, (note - 69) / 12.0f);
            const float dt = 1.0f / sr;
            t += dt;
            // envelope: attack, decay to sustain, release
            if (releasing) env *= std::exp (-dt / 0.15f);
            else if (t < p.attackSec) env = (float) (t / p.attackSec);
            else
            {
                auto d = p.decaySec * (0.4f + 1.2f * decayParam);
                env = p.sustain + (1.0f - p.sustain) * std::exp (-(float) (t - p.attackSec) / d);
            }
            if (releasing && env < 1.0e-4f) { active = false; return 0; }

            phase += freq * dt; phase -= std::floor (phase);
            phase2 += freq * 2.76f * dt; phase2 -= std::floor (phase2);
            phaseSub += freq * 0.5f * dt; phaseSub -= std::floor (phaseSub);
            const float tw = juce::MathConstants<float>::twoPi;
            const float h = p.bright * (0.5f + bright);
            float s = 0;
            switch (p.wave)
            {
                case 0: s = std::sin (tw * phase) + 0.3f * h * std::sin (2 * tw * phase) + 0.12f * h * std::sin (3 * tw * phase); break;
                case 1: s = std::sin (tw * phase) + 0.6f * h * std::sin (tw * phase2) * std::exp (-(float) t * 3.0f); break;
                case 2: s = (2.0f * phase - 1.0f) * (0.4f + 0.6f * h); break;
                case 3: s = rng.nextFloat() * 2.0f - 1.0f; break;
                case 4: s = std::sin (tw * phaseSub); break;
            }
            // one-pole low-pass ("Filter Cutoff")
            const float a = std::exp (-tw * (200.0f + 12000.0f * cutoff * cutoff) * dt);
            lp = (1.0f - a) * s + a * lp;
            float out = lp * env * velocity;
            if (p.gateHz > 0)
            {
                gatePhase += p.gateHz * dt; gatePhase -= std::floor (gatePhase);
                out *= gatePhase < 0.45f ? 1.0f : 0.0f;
            }
            return out;
        }
    };

    Voice voices[16];
    juce::Random rng;
    double sampleRate = 44100;
    int program = 0;
    juce::AudioParameterFloat* brightness = nullptr;
    juce::AudioParameterFloat* decay = nullptr;
    juce::AudioParameterFloat* cutoff = nullptr;
    juce::AudioParameterFloat* volume = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ForgeTestSynth(); }
