#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PhonkDSP.h"
#include "RcuSlot.h"

namespace ids
{
    // kit (re-renders the one-shots)
    inline constexpr auto kitSeed = "kit_seed";      inline constexpr auto kickTune = "kick_tune";
    inline constexpr auto kickDecay = "kick_decay";  inline constexpr auto snareTune = "snare_tune";
    inline constexpr auto snareDecay = "snare_decay"; inline constexpr auto hatDecay = "hat_decay";
    inline constexpr auto cowTune = "cow_tune";      inline constexpr auto grit = "grit";
    inline constexpr auto bitDepth = "bit_depth";    inline constexpr auto hiss = "hiss";
    inline constexpr auto tone = "tone";
    // tape bus (live)
    inline constexpr auto busDrive = "bus_drive";    inline constexpr auto busHiss = "bus_hiss";
    inline constexpr auto busTone = "bus_tone";      inline constexpr auto wow = "wow";
    inline constexpr auto master = "master_db";
    // mix (live)
    inline constexpr auto lvlKick = "lvl_kick";      inline constexpr auto lvlSnare = "lvl_snare";
    inline constexpr auto lvlHat = "lvl_hat";        inline constexpr auto lvlCow = "lvl_cow";
    // sequencer
    inline constexpr auto bpm = "bpm";               inline constexpr auto swing = "swing";
    inline constexpr auto bars = "bars";             inline constexpr auto variation = "variation";
    inline constexpr auto rolls = "rolls";           inline constexpr auto cowbell = "cowbell";
    inline constexpr auto patSeed = "pat_seed";      inline constexpr auto run = "seq_run";
    inline constexpr auto hostSync = "host_sync";
}

class PhonkProcessor : public juce::AudioProcessor,
                       private juce::AudioProcessorValueTreeState::Listener,
                       private juce::Timer
{
public:
    PhonkProcessor();
    ~PhonkProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    using AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Phonk Factory"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 1.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // ---- UI-facing API (message thread) ----
    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;                 // pads trigger notes through this
    const phonk::Pattern& uiPattern() const { return patternForUi; }
    float padFlash (int pad) const { return flash[(size_t) pad].load(); }
    void decayFlashes (float amount);
    float currentStep() const { return stepForUi.load(); }       // < 0 when stopped
    void rerollKit();                                            // new kit noise seed
    void newPattern();                                           // new pattern seed
    bool exportKitTo (const juce::File& folder, std::function<void (int)> onDone);
    juce::File renderLoopFile();                                 // temp WAV of the current loop (for drag & drop)
    bool isExporting() const { return exportThread.isThreadRunning(); }

private:
    struct Voice { const float* data = nullptr; int len = 0, pos = 0; float gain = 0; int group = 0; bool choke = false; float env = 1.f; bool active = false; };
    struct Trigger { int sample; int pad; float vel; };

    class Worker : public juce::Thread     // renders kits off the audio/message threads
    {
    public:
        explicit Worker (PhonkProcessor& o) : juce::Thread ("Phonk kit renderer"), owner (o) {}
        void run() override;
        PhonkProcessor& owner;
        std::atomic<bool> again { false };
    };
    class ExportThread : public juce::Thread
    {
    public:
        ExportThread() : juce::Thread ("Phonk export") {}
        void run() override { if (job) job(); }
        std::function<void()> job;
    };

    void parameterChanged (const juce::String& id, float) override;
    void timerCallback() override;
    phonk::KitSettings kitSettings() const;
    phonk::PatternSettings patternSettings() const;
    phonk::BusSettings busSettings() const;
    phonk::MixSettings mixSettings() const;
    float raw (const char* id) const { return apvts.getRawParameterValue (id)->load(); }
    void publishPattern();
    void trigger (Voice* pool, int pad, float vel);

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    RcuSlot<phonk::Kit> kitSlot;
    RcuSlot<phonk::Pattern> patternSlot;
    phonk::Pattern patternForUi;
    std::atomic<bool> kitDirty { true }, patternDirty { true };
    std::atomic<double> sampleRateAtomic { 44100.0 };
    Worker worker { *this };
    ExportThread exportThread;

    // audio-thread state
    static constexpr int kMaxVoices = 32, kMaxTriggers = 512;
    std::array<Voice, kMaxVoices> voices;
    std::array<Trigger, kMaxTriggers> triggers;
    phonk::BusFx bus;
    phonk::BusSettings lastBus;
    double internalSteps = 0, lastEndSteps = 0;
    bool wasRunning = false;
    float lastMaster = 1.f;
    std::array<std::atomic<float>, phonk::kNumPads> flash;
    std::atomic<float> stepForUi { -1.f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhonkProcessor)
};
