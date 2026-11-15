#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

#include "../../Source/VoiceDSP.h"
#include "LicenseVerifier.h"

class DrumSynthAudioProcessor : public juce::AudioProcessor
{
public:
    DrumSynthAudioProcessor();
    ~DrumSynthAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 1.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void triggerTestHit();
    bool activateLicenseJson(const juce::String& jsonText);
    bool isPluginLicensed() const { return isLicensed.load(std::memory_order_relaxed); }
    void refreshLicenseFromDisk();

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    VoiceDSP voiceDsp;
    std::atomic<bool> isLicensed{ false };
    std::atomic<bool> uiTriggerPending{ false };
    int demoSamplesCounter = 0;
    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumSynthAudioProcessor)
};