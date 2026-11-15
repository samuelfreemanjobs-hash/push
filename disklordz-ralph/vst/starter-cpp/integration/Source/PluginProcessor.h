#pragma once

#include <JuceHeader.h>
#include "../../PadSampleStore.h"

/** Minimal processor stub for DLZ-VST-007 — extend with TrapCoreDSP voices in your Windows tree. */
class DetroitPhonkTrapAudioProcessor : public juce::AudioProcessor
{
public:
    DetroitPhonkTrapAudioProcessor();
    ~DetroitPhonkTrapAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Detroit Phonk Trap Maker"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioBuffer<float>* getPadBuffer(int i) { return padStore.getPadBufferForDrag(i); }
    double getPadSampleRate(int i) const { return padStore.getPadSampleRate(i); }
    juce::MidiMessageSequence getPadMidiPattern(int i) const { return padStore.getPadMidiPattern(i); }
    void loadUserWavOntoPad(int i, const juce::File& f) { padStore.loadUserWavOntoPad(i, f); }
    double getLastKnownBpm() const { return padStore.getLastKnownBpm(); }

    PadSampleStore padStore;

private:
    juce::AudioProcessorValueTreeState apvts;
    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetroitPhonkTrapAudioProcessor)
};