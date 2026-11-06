#pragma once
#include "SamplePlanner.h"
#include "Effects.h"
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
        double maxTailSec = 3.0;        // after the last note-off (one-shots on pads rarely need more)
        double maxLengthSec = 15.0;
        double fadeOutMs = 25.0;
        double warmUpSec = 1.5;         // let sample-based plugins finish loading
        bool loudnessMatch = false;     // after the batch: level every file to the same perceived loudness
        double trimToSec = -1;          // override every style's end point: -1 = use the style's, 0 = no cap
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
