#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Params.h"
#include "Pattern.h"
#include "DrumVoice.h"
#include "KitFactory.h"
#include "VintageBus.h"

class KitForgeProcessor : public juce::AudioProcessor
{
public:
    KitForgeProcessor();
    ~KitForgeProcessor() override = default;

    //==========================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "KitForge"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    // GUI-facing API (message thread)
    void setInternalPlaying (bool shouldPlay);
    bool isInternalPlaying() const noexcept { return internalPlaying.load(); }
    bool isHostPlaying() const noexcept { return hostPlaying.load(); }
    int getCurrentStep() const noexcept { return currentStep.load(); }
    int getCurrentPattern() const noexcept;
    int getTriggerCount (int track) const noexcept { return triggerCount[(size_t) track].load(); }

    void previewTrack (int track, float velocity = 0.9f, float semitones = 0.0f);

    void loadKit (const kf::Kit& kit);
    juce::String getKitName() const { return kitName; }
    void setKitName (const juce::String& n) { kitName = n; }

    // Per-pad names (message thread only)
    juce::String getVoiceName (int t) const { return voiceNames[(size_t) t]; }
    void setVoiceName (int t, const juce::String& n) { voiceNames[(size_t) t] = n; }
    const std::array<juce::String, kf::numTracks>& getVoiceNames() const { return voiceNames; }
    kf::Kit captureCurrentKit() { return kf::KitFactory::captureKit (apvts, kitName, voiceNames); }

    juce::AudioProcessorValueTreeState apvts;
    kf::PatternBank patterns;

private:
    struct Event
    {
        int offset;
        int track;
        float velocity;
        float semitones = 0.0f;
    };

    void readVoiceParams();
    void fire (int track, float velocity, float semitones);

    std::array<kf::DrumVoice, kf::numTracks> voices;
    std::array<kf::VoiceParams, kf::numTracks> voiceParams;
    std::array<float, kf::numTracks> panL {}, panR {};

    std::array<std::array<std::atomic<float>*, kf::NumVP>, kf::numTracks> raw {};
    std::atomic<float>* tempoParam = nullptr;
    std::atomic<float>* swingParam = nullptr;
    std::atomic<float>* lengthParam = nullptr;
    std::atomic<float>* patternParam = nullptr;
    std::atomic<float>* masterParam = nullptr;
    std::array<std::atomic<float>*, kf::NumBP> busRaw {};

    kf::VintageCrusher crusher;
    kf::TransientShaper shaper;
    kf::VinylNoise vinyl;

    double sampleRate = 44100.0;
    double internalPpq = 0.0;
    static constexpr juce::int64 noHit = std::numeric_limits<juce::int64>::min();
    juce::int64 lastHitKey = noHit;
    juce::SmoothedValue<float> masterGain;

    std::atomic<bool> internalPlaying { false };
    std::atomic<bool> resetInternalClock { false };
    std::atomic<bool> hostPlaying { false };
    std::atomic<int> currentStep { -1 };
    std::array<std::atomic<int>, kf::numTracks> triggerCount {};
    std::array<std::atomic<float>, kf::numTracks> previewRequests {}, previewPitches {};

    std::vector<Event> events;

    juce::String kitName { "Boom Analog" };
    std::array<juce::String, kf::numTracks> voiceNames;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KitForgeProcessor)
};
