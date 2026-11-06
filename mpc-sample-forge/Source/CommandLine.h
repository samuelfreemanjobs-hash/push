#pragma once
#include "BatchJob.h"
#include "PluginHost.h"
#include "PresetSweep.h"
#include <atomic>

namespace forge
{
    /** Headless mode: MPCSampleForge --cli --plugin X.vst3 --style neo-soul --count 100 --out ~/Samples */
    class CliRunner
    {
    public:
        static bool wantsCli (const juce::StringArray& args);

        /** Returns immediately; `done` is called with an exit code when finished. */
        void start (const juce::StringArray& args, std::function<void (int)> done);

    private:
        void fail (const juce::String& msg);

        std::function<void (int)> onDone;
        PluginHost host;
        std::unique_ptr<juce::AudioProcessor> instrument;
        std::vector<std::unique_ptr<juce::AudioProcessor>> extraInstances; // parallel VST3 copies
        std::unique_ptr<BatchJob> job;
        std::unique_ptr<PresetSweep> sweep;
        std::unique_ptr<juce::Thread> diagThread;
        std::atomic<bool> diagCancel { false };
        void runDiagnose (const juce::StringArray& args);
    };

    juce::String cliHelpText();
}
