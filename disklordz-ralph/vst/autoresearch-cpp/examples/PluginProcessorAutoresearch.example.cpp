/**
 * REFERENCE ONLY — not compiled. Shows how to wire VoiceDSP into Detroit Phonk Trap Maker
 * after autoresearch wins are ported from Source/VoiceDSP.h.
 *
 * Merge pattern: include VoiceDSP.h from autoresearch-cpp/Source, or move tuned math into Trap808Voice.
 */
#include <JuceHeader.h>
#include "../Source/VoiceDSP.h"

class AutoresearchProcessorExample : public juce::AudioProcessor
{
public:
    AutoresearchProcessorExample()
        : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
    {
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        voice.prepare(sampleRate, samplesPerBlock);
    }

    void releaseResources() override { voice.reset(); }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override
    {
        juce::ScopedNoDenormals noDenormals;
        buffer.clear();

        for (const auto meta : midi)
        {
            const auto msg = meta.getMessage();
            if (msg.isNoteOn())
                voice.triggerNote(msg.getNoteNumber(), msg.getFloatVelocity());
            if (msg.isAllSoundOff())
                voice.reset();
        }

        voice.processBlock(buffer);
    }

    const juce::String getName() const override { return "AutoresearchExample"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

private:
    VoiceDSP voice;
};