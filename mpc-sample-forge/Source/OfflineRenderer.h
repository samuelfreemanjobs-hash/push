#pragma once
#include "SamplePlanner.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>

namespace forge
{
    struct RenderSettings
    {
        double sampleRate = 44100.0;
        int blockSize = 256;
        int bitDepth = 24;
        bool normalise = true;
        float normaliseDb = -1.0f;      // peak target
        double silenceDb = -70.0;       // tail is considered finished below this
        double maxTailSec = 6.0;        // after the last note-off
        double maxLengthSec = 15.0;
        double fadeOutMs = 25.0;
        double warmUpSec = 1.5;         // let sample-based plugins finish loading
    };

    /** Plays MIDI through an AudioProcessor faster than real time and returns
        the captured, cleaned-up audio. Must be used from one thread at a time. */
    class OfflineRenderer
    {
    public:
        OfflineRenderer (juce::AudioProcessor& p, const RenderSettings& s);
        ~OfflineRenderer();

        void warmUp();
        bool render (const SampleSpec& spec, juce::AudioBuffer<float>& out, juce::String& error);

        static bool writeWav (const juce::File& file, const juce::AudioBuffer<float>& audio,
                              double sampleRate, int bitDepth, int rootNote, juce::String& error);

    private:
        void processBlock (juce::MidiBuffer& midi);
        void flush();
        float blockPeak() const;

        juce::AudioProcessor& proc;
        RenderSettings settings;
        juce::AudioBuffer<float> block;
        int outChannels = 2;
    };
}
