#pragma once
#include "Styles.h"

namespace forge
{
    struct NoteEvent
    {
        int note = 60;
        int velocity = 100;
        double startSec = 0;   // offset from the start of the sample (strum)
        double lengthSec = 1;  // how long the key is held
    };

    struct SampleSpec
    {
        int index = 0;
        juce::String fileStem;      // file name without extension
        juce::String kind;          // Chord, Stab, Key, Octave, Fifth
        juce::String label;         // e.g. "Ebm9" or "C3"
        juce::String voicing;
        int rootNote = 60;          // written into the WAV as the MIDI unity note
        juce::String recipe;        // built-in synth engine for this sample
        juce::int64 variation = 1;  // timbre seed for the built-in synth
        std::vector<NoteEvent> notes;

        double holdSec() const;
    };

    struct PlanOptions
    {
        int count = 100;
        juce::String key = "random";   // "random" or a tonic such as "C", "F#", "Bb"
        juce::String scale;            // empty = the style's scale
        juce::int64 seed = 1;
        juce::String prefix;           // empty = style name
        int fixedRoot = -1;            // 16 Levels mode: every sample is built on this MIDI note
        juce::String recipe;           // override the style's recipe (built-in synth only)
        bool builtIn = false;          // rendering with the built-in synth (recipe names go in file names)
    };

    /** Creates a list of unique, musically sensible samples for a style. */
    std::vector<SampleSpec> planSamples (const Style& style, const PlanOptions& options, juce::String& error);

    /** Removes characters that are awkward on the MPC / in file systems. */
    juce::String safeFileName (const juce::String& s);
}
