#pragma once
#include <juce_core/juce_core.h>
#include <vector>

namespace forge
{
    /** A chord quality: an id used in style files, the suffix used in names
        and the intervals (in semitones above the root) that make it up. */
    struct ChordQuality
    {
        juce::String id;
        juce::String suffix;
        std::vector<int> intervals;
    };

    const std::vector<ChordQuality>& allQualities();
    const ChordQuality* findQuality (const juce::String& id);

    enum class Voicing { Close, Inversion1, Inversion2, Drop2, Open, Rootless, BassShell, Spread };

    juce::String voicingId (Voicing v);
    bool parseVoicing (const juce::String& id, Voicing& out);
    juce::StringArray allVoicingIds();

    /** Scale as a set of pitch classes relative to the tonic. */
    bool getScale (const juce::String& name, std::vector<int>& out);
    juce::StringArray allScaleNames();

    /** Note names use flats (Db, Eb ...) so file names stay free of '#'. */
    juce::String pitchClassName (int pitchClass);
    /** Parses "C", "Db", "C#", "eb" ... into 0-11, or -1. */
    int parsePitchClass (const juce::String& name);
    /** MIDI note name with the MPC / Ableton convention: 60 = C3. */
    juce::String midiNoteName (int midiNote);
    /** Parses "C3", "Eb2", "F#-1" (60 = C3) or a plain MIDI number; -1 if invalid. */
    int parseMidiNote (const juce::String& s);

    /** Builds the MIDI notes of a chord voicing. The result is sorted, has no
        duplicates and is kept inside the piano range. */
    std::vector<int> buildVoicing (int rootMidi, const std::vector<int>& intervals, Voicing v);

    /** Arranging rule ("low interval limits"): intervals stacked too low sound muddy,
        e.g. a third below ~C3 or a second below ~E3 (scientific pitch, 60 = C4).
        Returns how many adjacent intervals break it; `detail` describes the first. */
    int lowIntervalViolations (std::vector<int> notes, juce::String* detail = nullptr);

    /** True if every chord tone of `intervals` built on `rootPc` belongs to the
        scale `scale` built on `tonicPc`. */
    bool chordFitsScale (int rootPc, const std::vector<int>& intervals, int tonicPc, const std::vector<int>& scale);
}
