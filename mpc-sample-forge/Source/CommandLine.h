#pragma once
#include "BatchJob.h"
#include "PluginHost.h"

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
        std::unique_ptr<BatchJob> job;
    };

    juce::String cliHelpText();
}
