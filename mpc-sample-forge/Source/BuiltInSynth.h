#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

namespace forge
{
    /** The built-in sound engines ("recipes") for one-shot melodic material. */
    enum class Engine { EPiano, Bell, Organ, Pluck, Choir, Supersaw, EPBell, Triangle, Cowbell, ComboOrgan, Harp, Brass,
                        Lead, Whistle, Bass, MoogBass };

    juce::StringArray engineIds();   // epiano, bell, organ, pluck, choir, supersaw, epbell, triangle, cowbell, comboorgan, harp, brass
    juce::String engineId (Engine r);
    juce::String engineDisplayName (Engine r);           // "Bell", "Organ" ...
    bool parseEngine (const juce::String& id, Engine& out);

    /** A small multi-engine synth so the app makes real material without any
        VST3 installed. Each sample gets its own "variation" seed, which
        randomises the timbre within the engine (brightness, decay, drawbars,
        vowel, detune ...), so 100 renders give 100 different sounds. */
    class BuiltInSynth final : public juce::AudioProcessor
    {
    public:
        BuiltInSynth();

        void setEngine (Engine r)            { engine.store ((int) r); }
        Engine getEngine() const             { return (Engine) engine.load(); }
        void setVariation (juce::int64 seed) { variation.store (seed); }
        juce::int64 getVariation() const     { return variation.load(); }

        const juce::String getName() const override { return "Built-in Synth"; }
        void prepareToPlay (double sr, int) override { synth.setCurrentPlaybackSampleRate (sr); }
        void releaseResources() override {}
        void reset() override { synth.allNotesOff (0, false); }

        using AudioProcessor::processBlock;
        void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override
        {
            buffer.clear();
            synth.renderNextBlock (buffer, midi, 0, buffer.getNumSamples());
        }

        bool isBusesLayoutSupported (const BusesLayout& l) const override
        {
            return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
        }

        double getTailLengthSeconds() const override { return 3.0; }
        bool acceptsMidi() const override { return true; }
        bool producesMidi() const override { return false; }
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram (int) override {}
        const juce::String getProgramName (int) override { return "Default"; }
        void changeProgramName (int, const juce::String&) override {}
        void getStateInformation (juce::MemoryBlock&) override;
        void setStateInformation (const void*, int) override;

    private:
        struct Sound : juce::SynthesiserSound
        {
            bool appliesToNote (int) override { return true; }
            bool appliesToChannel (int) override { return true; }
        };
        class Voice;

        std::atomic<int> engine { (int) Engine::EPiano };
        std::atomic<juce::int64> variation { 1 };
        juce::Synthesiser synth;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BuiltInSynth)
    };
}
