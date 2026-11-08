#pragma once
#include <juce_core/juce_core.h>
#include <vector>

namespace forge
{
    /** A chord written by name, e.g. "Cm", "G#", "F#5". */
    struct NamedChord
    {
        int rootPc = 0;
        juce::String quality;   // chord quality id: "min", "maj" or "5"
    };

    struct NamedProgression
    {
        juce::String text;      // as written, e.g. "Cm-Gm-G#-Fm"
        std::vector<NamedChord> chords;
    };

    /** One entry of a progression library: a main progression plus two alternates
        that extend it into a full song, with tips on using it. */
    struct ProgressionSet
    {
        int number = 0;
        int tonicPc = 0;
        bool minor = true;
        juce::String keyName, tip;
        NamedProgression main, alt1, alt2;
    };

    /** The 20 trap progression sets (60 progressions). */
    const std::vector<ProgressionSet>& progressionLibrary (const juce::String& id);

    bool parseChordName (const juce::String& name, NamedChord& out);

    /** Roman numeral of a chord relative to a tonic, chromatic-aware: i, bVI, V5 ... */
    juce::String romanNumeralFor (const NamedChord& c, int tonicPc);
}
