#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "Parameters.h"
#include "dsp/SamplerCrusher.h"
#include "dsp/CassetteEQ.h"
#include "dsp/TapeSaturator.h"
#include "dsp/WowFlutter.h"
#include "dsp/TapeHiss.h"
#include "dsp/SubClipper.h"
#include "dsp/FetCompressor.h"

class MemphisDustProcessor final : public juce::AudioProcessor
{
public:
    MemphisDustProcessor();
    ~MemphisDustProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.25; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // UI metering (read on the message thread)
    float getGainReductionDb() noexcept { return compressor.getAndResetPeakGr(); }
    float getOutputPeak() noexcept      { return outPeak.exchange (0.0f); }

private:
    void updateParameters();
    void processChunk (juce::dsp::AudioBlock<float> block);

    // Cached raw parameter pointers (lock-free reads on the audio thread)
    struct Params
    {
        std::atomic<float>* inputGain {}; std::atomic<float>* outputGain {}; std::atomic<float>* mix {};
        std::atomic<float>* outClip {};
        std::atomic<float>* crushOn {}; std::atomic<float>* crushRate {}; std::atomic<float>* crushBits {};
        std::atomic<float>* crushAA {};
        std::atomic<float>* eqOn {}; std::atomic<float>* lowCut {}; std::atomic<float>* boxFreq {};
        std::atomic<float>* boxGain {}; std::atomic<float>* boxQ {}; std::atomic<float>* highCut {};
        std::atomic<float>* tapeOn {}; std::atomic<float>* tapeDrive {}; std::atomic<float>* tapeBias {};
        std::atomic<float>* wow {}; std::atomic<float>* flutter {}; std::atomic<float>* hiss {};
        std::atomic<float>* compOn {}; std::atomic<float>* compThresh {}; std::atomic<float>* compRatio {};
        std::atomic<float>* compAttack {}; std::atomic<float>* compRelease {}; std::atomic<float>* compMakeup {};
        std::atomic<float>* compMix {};
        std::atomic<float>* subOn {}; std::atomic<float>* subFreq {}; std::atomic<float>* subDrive {};
        std::atomic<float>* subHard {}; std::atomic<float>* subLevel {};
    } p;

    md::SamplerCrusher crusher;
    md::CassetteEQ eq;
    md::TapeSaturator tape;
    md::WowFlutter wowFlutter;
    md::TapeHiss hiss;
    md::SubClipper subClip;
    md::FetCompressor compressor;

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    juce::dsp::DryWetMixer<float> dryWet { 8192 };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> inGain, outGain;

    bool crushOn = true, eqOn = true, tapeOn = true, subOn = true, compOn = true, clipOn = true;
    int maxBlock = 512, numCh = 2, currentProgram = 0;
    std::atomic<float> outPeak { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemphisDustProcessor)
};
