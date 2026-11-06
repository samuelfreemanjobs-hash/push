#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace forge
{
    /** A small FM electric-piano so the app works (and can be tested) without
        any VST3 installed. Real sounds should come from a loaded VST3. */
    class BuiltInKeys final : public juce::AudioProcessor
    {
    public:
        BuiltInKeys()
            : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
        {
            for (int i = 0; i < 24; ++i) synth.addVoice (new Voice());
            synth.addSound (new Sound());
        }

        const juce::String getName() const override { return "Built-in FM Keys"; }
        void prepareToPlay (double sr, int) override { synth.setCurrentPlaybackSampleRate (sr); }
        void releaseResources() override {}
        void reset() override { synth.allNotesOff (0, false); }

        void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override
        {
            buffer.clear();
            synth.renderNextBlock (buffer, midi, 0, buffer.getNumSamples());
        }

        using AudioProcessor::processBlock;

        bool isBusesLayoutSupported (const BusesLayout& l) const override
        {
            return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
        }

        double getTailLengthSeconds() const override { return 2.0; }
        bool acceptsMidi() const override { return true; }
        bool producesMidi() const override { return false; }
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram (int) override {}
        const juce::String getProgramName (int) override { return "Default"; }
        void changeProgramName (int, const juce::String&) override {}
        void getStateInformation (juce::MemoryBlock&) override {}
        void setStateInformation (const void*, int) override {}

    private:
        struct Sound : juce::SynthesiserSound
        {
            bool appliesToNote (int) override { return true; }
            bool appliesToChannel (int) override { return true; }
        };

        struct Voice : juce::SynthesiserVoice
        {
            bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<Sound*> (s) != nullptr; }

            void startNote (int note, float vel, juce::SynthesiserSound*, int) override
            {
                freq = juce::MidiMessage::getMidiNoteInHertz (note);
                level = 0.18f * (0.25f + 0.75f * vel);
                brightness = 0.6f + 1.8f * vel;
                carrierPhase = modPhase = 0;
                time = 0;
                releasing = false;
                releaseGain = 1;
                // lower notes ring longer, like a real tine
                decay = juce::jmap ((float) note, 24.0f, 108.0f, 3.5f, 0.9f);
                pan = juce::jmap ((float) note, 24.0f, 108.0f, -0.35f, 0.35f);
            }

            void stopNote (float, bool allowTail) override
            {
                if (allowTail) releasing = true;
                else clearCurrentNote();
            }

            void pitchWheelMoved (int) override {}
            void controllerMoved (int, int) override {}

            using SynthesiserVoice::renderNextBlock;

            void renderNextBlock (juce::AudioBuffer<float>& out, int start, int num) override
            {
                if (! isVoiceActive()) return;

                auto sr = getSampleRate();
                auto dt = 1.0 / sr;
                auto releaseCoef = (float) std::exp (-1.0 / (0.25 * sr));
                auto l = std::sqrt (0.5f * (1.0f - pan)), r = std::sqrt (0.5f * (1.0f + pan));

                for (int i = 0; i < num; ++i)
                {
                    auto env = (float) std::exp (-time / decay);
                    auto modEnv = (float) std::exp (-time / (0.35 * decay));
                    auto attack = (float) juce::jmin (1.0, time / 0.002);

                    auto mod = std::sin (modPhase) * brightness * modEnv;
                    auto bell = 0.12f * (float) std::sin (carrierPhase * 7.0) * (float) std::exp (-time / 0.15);
                    auto s = (float) std::sin (carrierPhase + mod) + bell;
                    s *= level * env * attack * releaseGain;

                    out.addSample (0, start + i, s * l);
                    if (out.getNumChannels() > 1) out.addSample (1, start + i, s * r);

                    carrierPhase += juce::MathConstants<double>::twoPi * freq * dt;
                    modPhase += juce::MathConstants<double>::twoPi * freq * dt;
                    time += dt;

                    if (releasing) releaseGain *= releaseCoef;

                    if (releaseGain < 0.0005f || env < 0.0005f)
                    {
                        clearCurrentNote();
                        break;
                    }
                }
            }

            double freq = 440, carrierPhase = 0, modPhase = 0, time = 0;
            float level = 0, brightness = 1, decay = 2, pan = 0, releaseGain = 1;
            bool releasing = false;
        };

        juce::Synthesiser synth;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BuiltInKeys)
    };
}
