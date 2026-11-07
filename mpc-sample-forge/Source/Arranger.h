#pragma once
#include "SamplePlanner.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace forge
{
    /** One chord on the progression timeline. */
    struct ChordBlock
    {
        int rootPc = 0;
        juce::String quality = "maj7";     // chord quality id (Theory.h)
        double beats = 4;                  // how long it lasts (4 = one bar of 4/4)

        std::vector<int> intervals() const;
        juce::String name() const;                     // "Dm9"
        juce::String roman (int tonicPc) const;        // "ii9" style numeral relative to the key
    };

    /** A note of a generated part, in beats from the start of the loop. */
    struct PartNote { int note = 60; double start = 0, length = 1; int velocity = 100; };

    /** One instrument track: keys, bass, pad, lead or drums. */
    struct Part
    {
        juce::String id, name;             // "keys", "Keys"
        juce::String recipeId;             // which one-shot recipe sounds it in loops (engine + FX)
        int channel = 1;                   // MIDI channel (drums = 10)
        bool enabled = true;
        std::vector<PartNote> notes;
    };

    /** A genre preset: how a progression in this style is written and played. Built from the recipes:
        every part (keys, bass, pad / strings, lead, drums) has its own recipe. */
    struct SongPreset
    {
        juce::String id, name, description;
        juce::String scale = "major";      // "major", "minor", "dorian" ...
        juce::StringArray keys;            // keys it is often in (used when the key is random)
        double tempoLow = 80, tempoHigh = 96, swing = 0;  // swing 0..1 (1 = full triplet feel)
        juce::StringArray progressions;    // templates in numerals: "ii:m9 V:13 I:maj9 vi:m9" ("*2" = 2 beats)
        juce::String keysRecipe, bassRecipe, padRecipe, leadRecipe;
        juce::String keysPattern, bassPattern, drumPattern;
        int leadDensity = 1;               // 0 sparse, 1 medium, 2 busy
    };

    const std::vector<SongPreset>& songPresets();
    const SongPreset* findSongPreset (const juce::String& id);

    struct Arrangement
    {
        juce::String presetId;
        int tonic = 0;
        juce::String scale = "major";
        double tempo = 90, swing = 0;
        juce::int64 seed = 1;
        std::vector<ChordBlock> blocks;
        std::vector<Part> parts;           // keys, bass, pad, lead, drums

        double totalBeats() const;
        double lengthSeconds() const { return totalBeats() * 60.0 / tempo; }
        juce::String progressionText() const;   // "ii:m9 V:13 I:maj9 vi:m9"
    };

    /** Reads a typed progression: numerals with qualities ("ii:m9 V:13 Imaj9 bVII:9"), or chord
        symbols ("Dm9 G13 Cmaj9"); "*2" after a chord = 2 beats. */
    bool parseProgression (const juce::String& text, int tonicPc, std::vector<ChordBlock>& out, juce::String& error);

    /** Writes a progression in the preset's style: 4, 8 or 16 bars. */
    std::vector<ChordBlock> generateProgression (const SongPreset& preset, int tonicPc, int bars, juce::int64 seed);

    /** Writes the parts that play the blocks: keys comping, bass line, pad / strings, a lead melody
        and drums, in the preset's style. Parts keep their recipe / enabled settings if already set. */
    void generateParts (Arrangement& arrangement, const SongPreset& preset);

    /** MIDI files: <name>.mid (every part on its own track), <name>_<part>.mid each, and
        <name>_chords.mid (block chords). Returns the files written. */
    juce::StringArray writeArrangementMidi (const Arrangement& arrangement, const juce::File& folder, const juce::String& name);

    /** Renders the melodic parts with the built-in synth and each part's recipe FX into loops of
        exactly the arrangement's length (tails folded back to the start, so they loop seamlessly).
        stems[i] matches parts[i] (empty if the part is off / drums). Drums stay MIDI-only. */
    bool renderArrangement (const Arrangement& arrangement, double sampleRate, juce::AudioBuffer<float>& mix,
                            std::vector<juce::AudioBuffer<float>>& stems, juce::String& error);

    /** Renders and writes <name>_loop.wav plus one stem per part. Returns the files written. */
    juce::StringArray writeArrangementLoops (const Arrangement& arrangement, const juce::File& folder, const juce::String& name,
                                             double sampleRate, int bitDepth, juce::String& error);
}
