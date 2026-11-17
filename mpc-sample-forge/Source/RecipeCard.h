#pragma once
#include "SamplePlanner.h"

namespace forge
{
    /** One file a recipe card asks for: a chord hit, a lead / soft note or a texture. */
    struct CardItem
    {
        juce::String kind;             // "CH", "LEAD", "SOFT" or "TEXTURE"
        juce::String label;            // "Dmaj9", "Fs5", "WARM_PAD_SWELL"
        juce::String fileName;         // without .wav, e.g. "DL_BHC_Dmaj9_CH"
        std::vector<int> notes;        // MIDI notes, exactly as written on the card
        int velocity = 100;
        double attackMs = 0, holdSec = 0;
        bool reverse = false;          // REVERSE_... textures play backwards
        bool shimmer = false;          // GLASS / SHIMMER textures: an octave-up shimmer delay
        juce::String engine;           // built-in sound for this item
    };

    struct MelodyNote { int note = 60; double startBeats = 0, lengthBeats = 1; };

    /** A recipe from a recipe book (e.g. a daily ChatGPT installment): what to render, with exact
        notes and file names. Read from a FORGE CARD block or, best effort, from the book's own text. */
    struct RecipeCard
    {
        juce::String name, code;       // "Blue Hour Convertible", "DL_BHC"
        juce::String key;              // "D major"
        double tempo = 90;
        bool mpcNotation = false;      // note names on the card: false = standard (C4 = middle C), true = MPC (C3)
        bool prefersFlats = false;     // the card writes notes with flats (Ab2) rather than sharps (G#2)
        juce::String soundText;        // what the card says about the sound
        juce::String chordEngine = "epiano", leadEngine = "epiano", textureEngine = "pad";
        juce::var fx;                  // effects read from the card (array, Effects.h format)
        std::vector<CardItem> items;
        std::vector<std::pair<juce::String, std::vector<int>>> chordSequence; // for CHORDS.mid, one chord per bar
        std::vector<MelodyNote> melody;                                        // for MELODY.mid
        juce::String chordsMidiName, melodyMidiName;
        juce::StringArray warnings;    // things on the card that couldn't be read

        juce::String describe() const; // the list of files with their notes
    };

    /** Reads every card in the text: FORGE CARD blocks, or numbered recipes ("1. Blue Hour ...")
        with chord tables, melody tables and file-name lists. */
    std::vector<RecipeCard> parseRecipeCards (const juce::String& text, bool mpcNotation = false);

    /** A note name as written on a card: "F#3", "F♯3", "Fs3" (file names), "Bb2", "B♭2". */
    int parseCardNote (const juce::String& token, bool mpcNotation);

    /** The samples to render for a card (exact notes and file names), sounding like `style` unless
        the card names its own effects. */
    std::vector<SampleSpec> planCard (const RecipeCard& card, const Style& style, const PlanOptions& options, juce::String& error);

    /** Writes the card's chords (one per bar) and melody as standard MIDI files. Returns the files written. */
    juce::StringArray writeCardMidi (const RecipeCard& card, const juce::File& folder);

    /** For a batch planned from a card: writes that card's .mid files into the batch folder. */
    juce::StringArray writeCardMidiForPlan (const PlanOptions& plan, const juce::File& folder);

    /** The text to give ChatGPT so every recipe ends with a FORGE CARD block. */
    juce::String forgeCardPrompt();
}
