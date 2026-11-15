#pragma once

#include <JuceHeader.h>
#include <atomic>

class WebDrumAudioProcessor : public juce::AudioProcessor
{
public:
    WebDrumAudioProcessor();
    ~WebDrumAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.25; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void triggerKickFromUi();
    juce::AudioProcessorValueTreeState& getApvts() { return apvts; }

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>& getGainSmoother() { return gainSmoother; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> gainSmoother;

    std::atomic<bool> kickPending{ false };
    float phase = 0.0f;
    float env = 0.0f;
    bool kickActive = false;
    double sampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebDrumAudioProcessor)
};