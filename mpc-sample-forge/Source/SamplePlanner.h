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
        juce::String shortTag;      // compact tag for short file names
        int pad = 0;                // scale banks: pad 1-16 inside its bank (also the number in the file name)
        juce::String bankInfo;      // scale banks: e.g. "C minor - chords"
        juce::String group;         // progression banks: the progression this pad belongs to
        int progressionSet = 0;     // progression-library banks: the set number (1-20), 0 = none
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
        bool scaleBank = false;        // 16 pads per sound, climbing the scale (notes or diatonic chords)
        juce::String bankType;         // "notes" / "chords" (empty = the style's)
        bool shortNames = false;       // "014_EPBell_C3" instead of "JetLifeEPBell_C3_014_Key_C3" (easier on a small screen)
        int progressionNumber = 0;     // progression-library recipes: only this set (1-20), 0 = a mix chosen by the seed
    };

    /** Creates a list of unique, musically sensible samples for a style. */
    std::vector<SampleSpec> planSamples (const Style& style, const PlanOptions& options, juce::String& error);

    /** The same sample moved by `semitones` (e.g. -12 = an octave down): notes, root,
        label and file name all follow. Returns false if it would leave the MIDI range. */
    bool transposeSpec (const SampleSpec& in, int semitones, SampleSpec& out);

    /** One diatonic chord of a key, as the chord buttons and progression banks use it. */
    struct DiatonicChord
    {
        int degree = 0;               // 0-based scale degree
        int rootPc = 0;
        juce::String roman, name;     // "ii", "Dm9"
        std::vector<int> intervals;
    };

    /** The chord on each degree of the scale, using the style's own chord types where they
        fit the key (its most-weighted fitting type), else the scale's 7th chord / triad.
        `variety` > 0 picks weighted-randomly instead (different chords per bank). */
    std::vector<DiatonicChord> diatonicChords (const Style& style, int tonic, const std::vector<int>& scale,
                                               juce::int64 variety = 0);

    /** Close voicing of a chord from `rootMidi`, then inverted: 1 = lowest note up an octave, etc. */
    std::vector<int> invertChord (int rootMidi, const std::vector<int>& intervals, int inversion);

    /** Common progressions (0-based degrees) that suit a scale: e.g. ii-V-I-vi in major. */
    std::vector<std::pair<juce::String, std::vector<int>>> progressionsFor (const std::vector<int>& scale);

    /** Renumbers 1..n and rebuilds the file names (after Smart Pick has chosen). */
    void renumberSamples (std::vector<SampleSpec>& specs);

    /** Removes characters that are awkward on the MPC / in file systems. */
    juce::String safeFileName (const juce::String& s);
}
