#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace forge
{
    /** Finds and loads VST3 instruments. Use from the message thread. */
    class PluginHost
    {
    public:
        PluginHost();

        /** Paths of every .vst3 bundle in the standard VST3 folders (no loading). */
        juce::StringArray findInstalledVST3() const;

        std::unique_ptr<juce::AudioPluginInstance> load (const juce::String& pathOrId, double sampleRate,
                                                         int blockSize, juce::String& error);

        juce::AudioPluginFormatManager& getFormatManager() { return formatManager; }

    private:
        juce::AudioPluginFormat* vst3() const;
        juce::AudioPluginFormatManager formatManager;
    };

    /** A ".forgesound" file: which plugin was used plus its full state (the
        sound you dialled in), so a batch can be repeated or run headless. */
    struct SoundState
    {
        juce::String pluginPath, pluginName;
        juce::MemoryBlock state;

        bool save (const juce::File& f) const;
        bool load (const juce::File& f, juce::String& error);
    };

    SoundState captureState (juce::AudioProcessor& p, const juce::String& pluginPath);
}
