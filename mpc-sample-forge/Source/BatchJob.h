#pragma once
#include "OfflineRenderer.h"
#include <functional>

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
    };

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

    private:
        void post (int done, int total, const juce::String& msg);
        juce::String mpcSetupSheet() const;
        juce::File bankFolder (int index, const juce::File& base) const;
        juce::File bankFolder (int index) const { return bankFolder (index, request.outputFolder); }

        juce::AudioProcessor& proc;
        BatchRequest request;
        std::vector<SampleSpec> specs;
    };

    juce::String describeSpec (const SampleSpec& s);
}
