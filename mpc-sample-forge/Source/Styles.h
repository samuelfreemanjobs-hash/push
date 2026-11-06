#pragma once
#include "Theory.h"
#include <juce_core/juce_core.h>

namespace forge
{
    template <typename T>
    struct Weighted { T value; float weight; };

    /** Everything that makes a batch sound like a given genre. */
    struct Style
    {
        juce::String id, name, description;
        juce::String scale = "minor";                      // used when a key is chosen

        std::vector<Weighted<juce::String>> chords;        // chord quality ids
        std::vector<Weighted<Voicing>> voicings;

        float chordRatio = 0.7f;                           // share of chords vs. single-key one-shots
        float keyNote = 4, keyOctave = 2, keyFifth = 1, keyStab = 1; // weights of one-shot key types

        int chordRootLow = 48, chordRootHigh = 59;         // MIDI range for chord roots
        int keyLow = 36, keyHigh = 72;                     // MIDI range for single notes
        int velocityLow = 80, velocityHigh = 110;
        double holdLow = 1.5, holdHigh = 2.5;              // seconds a chord is held
        float stabChance = 0.15f;                          // chance a chord is a short stab
        double stabHold = 0.25;
        double strumMs = 15.0;                             // max spread between chord notes

        juce::String recipe = "epiano";                    // built-in synth engine (or "mixed")
        double maxLength = 0;                              // cap on each file in seconds (0 = no cap)

        juce::var toVar() const;
        static bool fromVar (const juce::var& v, Style& out, juce::String& error);
    };

    const juce::Array<Style>& builtInStyles();
    const Style* findBuiltInStyle (const juce::String& idOrName);

    /** Loads one style, or the first of an array of styles, from a JSON file. */
    bool loadStyleFile (const juce::File& f, Style& out, juce::String& error);
}
