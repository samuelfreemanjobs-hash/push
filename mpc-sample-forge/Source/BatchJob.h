#pragma once
#include "OfflineRenderer.h"
#include <functional>
#include <atomic>

namespace forge
{
    struct BatchRequest
    {
        Style style;
        PlanOptions plan;
        RenderSettings render;
        juce::File outputFolder;
        juce::String instrumentName;
        bool splitBanks = false;   // put every 16 files into Bank_A, Bank_B ... (one MPC pad bank each)
        std::vector<int> registers;  // extra renders of every kept sample, in semitones (e.g. -12, +12)
        int smartPickTarget = 0;   // > 0: the specs are candidates; keep this many of the best (Smart Pick)
        int variety = 1;           // near-duplicate removal: 0 off, 1 normal (only near-identical), 2 strict
        std::vector<juce::AudioProcessor*> extraInstances; // more copies of the instrument for parallel rendering

        // "Octave copies for keepers": render only the register copies of these existing files
        bool registersOnly = false;
        std::vector<juce::File> existingFiles;              // same order as the specs
    };

    /** How a sample was made, so it can be re-rendered later (e.g. octave copies of keepers). */
    juce::var specToVar (const SampleSpec& s);
    bool specFromVar (const juce::var& v, SampleSpec& out);

    /** Renders a whole plan on a background thread. Callbacks arrive on the
        message thread. The processor must not be used by anything else
        (e.g. an audio device) while the job runs. */
    class BatchJob final : public juce::Thread
    {
    public:
        BatchJob (juce::AudioProcessor& p, BatchRequest r, std::vector<SampleSpec> specs);
        ~BatchJob() override;

        std::function<void (int done, int total, const juce::String& message)> onProgress;
        std::function<void (bool ok, const juce::String& summary)> onFinished;

        void run() override;

        /** Seconds of audio rendered and wall-clock seconds spent (for time estimates). */
        double renderedAudioSeconds() const { return (double) audioMicros.load() / 1.0e6; }
        double elapsedSeconds() const       { return elapsed; }

    private:
        using Task = std::function<void (int index, OfflineRenderer& renderer, juce::AudioProcessor& instrument)>;
        /** Runs n render tasks spread over every available instrument instance. */
        void renderParallel (int n, const Task& task);
        void runRegistersOnly();
        void post (int done, int total, const juce::String& msg);
        juce::String mpcSetupSheet() const;
        juce::File bankFolder (int index, const juce::File& base) const;
        juce::File bankFolder (int index) const { return bankFolder (index, request.outputFolder); }

        juce::AudioProcessor& proc;
        BatchRequest request;
        std::vector<SampleSpec> specs;

        std::vector<juce::AudioProcessor*> workers;               // proc + extra instances
        std::vector<std::unique_ptr<juce::AudioProcessor>> ownedWorkers; // extra built-in synths we created
        std::atomic<juce::int64> audioMicros { 0 };
        std::atomic<int> progressDone { 0 };
        juce::CriticalSection failureLock;
        juce::StringArray failures;
        double elapsed = 0;
    };

    juce::String describeSpec (const SampleSpec& s);
}
