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
        juce::String stemHead, stemTail; // fileStem = head + "_" + index + "_" + tail (lets Smart Pick renumber)
        juce::String kind;          // Chord, Stab, Key, Octave, Fifth
        juce::String label;         // e.g. "Ebm9" or "C3"
        juce::String voicing;
        int rootNote = 60;          // written into the WAV as the MIDI unity note
        juce::String engine;        // built-in synth engine for this sample
        juce::int64 variation = 1;  // timbre seed for the built-in synth
        Processing processing;      // trim / envelope / FX from the style
        juce::String styleId, tag;  // which style (or kit member) made this sample
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
                                       // (-1 = off, styleRoot = use each style's / recipe's own root)
        static constexpr int styleRoot = -2;
        juce::String engine;           // override the style's engine (built-in synth only)
        bool builtIn = false;          // rendering with the built-in synth (engine names go in file names)
    };

    /** Creates a list of unique, musically sensible samples for a style. */
    std::vector<SampleSpec> planSamples (const Style& style, const PlanOptions& options, juce::String& error);

    /** Renumbers 1..n and rebuilds the file names (after Smart Pick has chosen). */
    void renumberSamples (std::vector<SampleSpec>& specs);

    /** Removes characters that are awkward on the MPC / in file systems. */
    juce::String safeFileName (const juce::String& s);
}
