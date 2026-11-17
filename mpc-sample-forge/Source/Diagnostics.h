#pragma once
#include "PluginHost.h"
#include <juce_audio_devices/juce_audio_devices.h>
#include <atomic>
#include <functional>

namespace forge
{
    //==========================================================================
    /** Persistent log file (MPCSampleForge/Logs/forge.log in the user's app-data
        folder). Also installs a crash handler that writes a stack trace there. */
    namespace Log
    {
        void init (const juce::String& appName);
        void write (const juce::String& message);
        juce::File file();
        void shutdown();
    }

    //==========================================================================
    struct DiagResult
    {
        enum class Level { Info, Pass, Warn, Fail };
        Level level;
        juce::String area, message;

        juce::String toString() const;
    };

    struct DiagSummary
    {
        int pass = 0, warn = 0, fail = 0;
        void add (const DiagResult& r);
        juce::String toString() const;
    };

    /** Everything about this machine / setup that helps to track a problem down. */
    juce::String systemReport (PluginHost& host, juce::AudioDeviceManager* devices,
                               juce::AudioProcessor* instrument, const juce::String& instrumentPath);

    /** Renders every built-in engine, every recipe and a WAV round trip, and
        (if given) a test note through the loaded plugin. Can take a few seconds;
        run it off the message thread. `plugin` must not be in use elsewhere. */
    void runSelfTest (std::function<void (const DiagResult&)> report,
                      juce::AudioProcessor* plugin,
                      const std::atomic<bool>& cancel);
}
