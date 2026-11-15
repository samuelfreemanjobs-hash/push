#pragma once

#include <JuceHeader.h>
#include "../PadSampleStore.h"
#include <atomic>
#include <memory>
#include <vector>

class Trap808Voice;
class TrapSamplerSound;

class DetroitPhonkTrapAudioProcessor : public juce::AudioProcessor
{
public:
    DetroitPhonkTrapAudioProcessor();
    ~DetroitPhonkTrapAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
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

    juce::AudioProcessorValueTreeState& getApvts() { return apvts; }
    juce::AudioBuffer<float>* getPadBuffer(int i);
    double getPadSampleRate(int i) const { return padStore.getPadSampleRate(i); }
    juce::MidiMessageSequence getPadMidiPattern(int i) const { return padStore.getPadMidiPattern(i); }
    void loadUserWavOntoPad(int i, const juce::File& f);
    void ensurePadPreview(int pad);
    double getLastKnownBpm() const { return padStore.getLastKnownBpm(); }

    bool isLicensedForAudio() const { return isLicensed.load(std::memory_order_relaxed); }
    void refreshLicenseFromDisk();

    PadSampleStore padStore;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;
    juce::Synthesiser synthesiser;
    std::vector<std::unique_ptr<TrapSamplerSound>> sounds;
    double currentSampleRate = 44100.0;
    std::atomic<bool> isLicensed{ true };
    int demoDipSampleCounter = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetroitPhonkTrapAudioProcessor)
};