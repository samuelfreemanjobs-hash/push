#include "Styles.h"
#include "BuiltInSynth.h"

namespace forge
{
// Built-in styles. The same schema is accepted from user JSON files, so any of
// these can be dumped with --dump-style and edited.
static const char* builtInStyleJson = R"JSON(
[
 { "id": "neo-soul", "name": "Neo-Soul",
   "description": "Lush 9ths, 11ths and 13ths, rootless voicings, rolled like a Rhodes player.",
   "scale": "dorian",
   "chords": { "m9": 6, "maj9": 5, "m11": 5, "9": 3, "13": 3, "maj7s11": 2, "7s9": 2, "9sus4": 3, "m7": 2, "69": 2 },
   "voicings": { "rootless": 5, "drop2": 4, "spread": 3, "shell": 2, "open": 1 },
   "chordRatio": 0.75, "keyTypes": { "note": 3, "octave": 2, "fifth": 1, "stab": 2 },
   "chordRootRange": [45, 57], "keyRange": [36, 72], "velocity": [70, 100],
   "hold": [1.6, 2.8], "stabChance": 0.12, "stabHold": 0.3, "strumMs": 40 },

 { "id": "lofi", "name": "Lo-Fi Hip Hop",
   "description": "Mellow jazzy 7ths and 9ths, soft velocities, close and drop-2 voicings.",
   "scale": "minor",
   "chords": { "maj7": 5, "m7": 5, "m9": 4, "maj9": 4, "9": 2, "m11": 2, "69": 2, "7sus4": 1, "m7b5": 1 },
   "voicings": { "close": 3, "drop2": 4, "rootless": 3, "inv1": 2, "inv2": 1 },
   "chordRatio": 0.7, "keyTypes": { "note": 4, "octave": 2, "fifth": 1, "stab": 1 },
   "chordRootRange": [45, 57], "keyRange": [40, 76], "velocity": [55, 85],
   "hold": [1.8, 3.0], "stabChance": 0.1, "stabHold": 0.35, "strumMs": 30 },

 { "id": "trap", "name": "Dark Trap",
   "description": "Minor triads, diminished and sus colours, wide low voicings for 808-heavy beats.",
   "scale": "harmonicminor",
   "chords": { "min": 6, "madd9": 4, "m7": 3, "dim": 3, "sus2": 2, "5": 2, "mmaj7": 2, "maj": 2, "aug": 1, "madd11": 2 },
   "voicings": { "close": 3, "open": 3, "spread": 3, "inv1": 2, "inv2": 2 },
   "chordRatio": 0.6, "keyTypes": { "note": 5, "octave": 3, "fifth": 2, "stab": 1 },
   "chordRootRange": [43, 55], "keyRange": [36, 79], "velocity": [85, 120],
   "hold": [1.2, 2.2], "stabChance": 0.25, "stabHold": 0.2, "strumMs": 5 },

 { "id": "drill", "name": "UK / NY Drill",
   "description": "Sinister minor and phrygian colours, tense intervals, short stabs and long dark chords.",
   "scale": "phrygian",
   "chords": { "min": 5, "madd9": 3, "dim": 3, "m7b5": 2, "mmaj7": 3, "5": 2, "sus4": 2, "maj": 2, "aug": 1 },
   "voicings": { "close": 3, "open": 3, "spread": 2, "inv1": 2, "inv2": 2 },
   "chordRatio": 0.6, "keyTypes": { "note": 5, "octave": 3, "fifth": 1, "stab": 2 },
   "chordRootRange": [43, 55], "keyRange": [36, 79], "velocity": [90, 120],
   "hold": [1.0, 2.0], "stabChance": 0.3, "stabHold": 0.18, "strumMs": 4 },

 { "id": "rnb", "name": "Modern R&B",
   "description": "Silky maj9 / m9 / m11 chords with 9sus4 turnarounds and gentle rolls.",
   "scale": "minor",
   "chords": { "maj9": 5, "m9": 5, "m11": 4, "9sus4": 3, "maj13": 2, "m7": 2, "maj7": 2, "add9": 2, "69": 1 },
   "voicings": { "rootless": 4, "drop2": 4, "spread": 3, "open": 2 },
   "chordRatio": 0.75, "keyTypes": { "note": 3, "octave": 2, "fifth": 1, "stab": 1 },
   "chordRootRange": [45, 57], "keyRange": [40, 76], "velocity": [65, 100],
   "hold": [1.8, 3.0], "stabChance": 0.1, "stabHold": 0.3, "strumMs": 35 },

 { "id": "gospel", "name": "Gospel",
   "description": "Big church chords: 13ths, altered dominants, 6/9s and walk-up colours.",
   "scale": "major",
   "chords": { "maj9": 4, "13": 4, "7s9": 3, "7b9": 3, "69": 3, "m11": 3, "9sus4": 3, "13b9": 2, "dim7": 2, "m9": 3, "maj13": 2 },
   "voicings": { "spread": 4, "rootless": 3, "drop2": 3, "shell": 2, "open": 2 },
   "chordRatio": 0.8, "keyTypes": { "note": 2, "octave": 3, "fifth": 1, "stab": 2 },
   "chordRootRange": [43, 55], "keyRange": [36, 72], "velocity": [80, 115],
   "hold": [1.5, 2.6], "stabChance": 0.2, "stabHold": 0.3, "strumMs": 25 },

 { "id": "house", "name": "Deep House",
   "description": "Classic house stabs and pads: m7, m9, m11 and organ-style sus chords.",
   "scale": "minor",
   "chords": { "m7": 5, "m9": 5, "m11": 4, "7sus4": 2, "9sus4": 2, "maj7": 2, "maj9": 2, "quartal": 2 },
   "voicings": { "close": 4, "inv1": 3, "inv2": 3, "drop2": 2 },
   "chordRatio": 0.75, "keyTypes": { "note": 3, "octave": 3, "fifth": 2, "stab": 3 },
   "chordRootRange": [48, 59], "keyRange": [36, 72], "velocity": [90, 115],
   "hold": [0.8, 1.8], "stabChance": 0.45, "stabHold": 0.22, "strumMs": 0 },

 { "id": "jazz", "name": "Jazz",
   "description": "ii-V-I vocabulary: drop-2 and shell voicings, altered dominants, m7b5s.",
   "scale": "major",
   "chords": { "m7": 4, "7": 3, "maj7": 4, "m9": 3, "13": 3, "7b9": 2, "7s9": 2, "m7b5": 3, "dim7": 1, "maj9": 3, "69": 2, "7s5": 1 },
   "voicings": { "drop2": 5, "shell": 4, "rootless": 4, "close": 1 },
   "chordRatio": 0.8, "keyTypes": { "note": 3, "octave": 2, "fifth": 1, "stab": 2 },
   "chordRootRange": [43, 57], "keyRange": [40, 76], "velocity": [60, 100],
   "hold": [1.4, 2.4], "stabChance": 0.2, "stabHold": 0.35, "strumMs": 12 },

 { "id": "boom-bap", "name": "Boom Bap",
   "description": "Dusty sample-style chords: minor 7ths, 9ths, 6ths and soulful major 7ths.",
   "scale": "dorian",
   "chords": { "m7": 5, "m9": 4, "maj7": 4, "m6": 2, "6": 2, "9": 3, "min": 2, "7": 2, "m11": 2 },
   "voicings": { "close": 4, "drop2": 3, "inv1": 2, "rootless": 2 },
   "chordRatio": 0.7, "keyTypes": { "note": 4, "octave": 2, "fifth": 1, "stab": 2 },
   "chordRootRange": [45, 57], "keyRange": [38, 74], "velocity": [75, 105],
   "hold": [1.2, 2.2], "stabChance": 0.25, "stabHold": 0.3, "strumMs": 18 },

 { "id": "afrobeats", "name": "Afrobeats",
   "description": "Bright, warm major 7ths, add9s and 6ths with plucky stabs.",
   "scale": "major",
   "chords": { "maj7": 4, "add9": 4, "6": 3, "m7": 4, "maj9": 3, "sus2": 2, "maj": 2, "m9": 2, "69": 2 },
   "voicings": { "close": 4, "inv1": 3, "inv2": 3, "open": 2 },
   "chordRatio": 0.7, "keyTypes": { "note": 4, "octave": 2, "fifth": 1, "stab": 3 },
   "chordRootRange": [48, 60], "keyRange": [43, 79], "velocity": [80, 110],
   "hold": [1.0, 2.0], "stabChance": 0.35, "stabHold": 0.2, "strumMs": 8 },

 { "id": "pop", "name": "Pop",
   "description": "Clean triads, add9s and sus chords in friendly inversions.",
   "scale": "major",
   "chords": { "maj": 5, "min": 5, "add9": 3, "sus2": 3, "sus4": 2, "maj7": 2, "m7": 2, "madd9": 2, "6": 1 },
   "voicings": { "close": 4, "inv1": 3, "inv2": 3, "open": 2, "spread": 1 },
   "chordRatio": 0.7, "keyTypes": { "note": 4, "octave": 2, "fifth": 2, "stab": 2 },
   "chordRootRange": [48, 60], "keyRange": [40, 79], "velocity": [80, 110],
   "hold": [1.2, 2.2], "stabChance": 0.2, "stabHold": 0.25, "strumMs": 6 },

 { "id": "cinematic", "name": "Cinematic",
   "description": "Huge open fifths, sus2, minor add9 and lydian colours spread across the keyboard.",
   "scale": "minor",
   "chords": { "5": 3, "sus2": 4, "madd9": 4, "maj7s11": 3, "min": 3, "maj": 2, "quintal": 2, "mmaj9": 2, "maj9s11": 2 },
   "voicings": { "spread": 5, "open": 4, "close": 1 },
   "chordRatio": 0.75, "keyTypes": { "note": 3, "octave": 4, "fifth": 3, "stab": 1 },
   "chordRootRange": [38, 52], "keyRange": [28, 72], "velocity": [70, 115],
   "hold": [2.5, 4.0], "stabChance": 0.1, "stabHold": 0.4, "strumMs": 10 },

 { "id": "ambient", "name": "Ambient",
   "description": "Slow, floating quartal and suspended clusters with long holds.",
   "scale": "lydian",
   "chords": { "quartal": 4, "quintal": 3, "sus2": 4, "maj9": 3, "maj7s11": 3, "add9": 3, "9sus4": 2, "m9": 2 },
   "voicings": { "spread": 4, "open": 4, "close": 1, "inv1": 1, "inv2": 1, "drop2": 1 },
   "chordRatio": 0.8, "keyTypes": { "note": 3, "octave": 3, "fifth": 3, "stab": 0 },
   "chordRootRange": [40, 55], "keyRange": [36, 84], "velocity": [55, 90],
   "hold": [3.0, 5.0], "stabChance": 0.0, "stabHold": 0.4, "strumMs": 60 },

 { "id": "synthwave", "name": "Synthwave",
   "description": "80s minor triads, add9s and sus chords, punchy and close.",
   "scale": "minor",
   "chords": { "min": 5, "maj": 4, "madd9": 3, "add9": 3, "sus2": 3, "sus4": 2, "maj7": 2, "5": 2 },
   "voicings": { "close": 4, "inv1": 3, "inv2": 2, "open": 2 },
   "chordRatio": 0.7, "keyTypes": { "note": 4, "octave": 3, "fifth": 2, "stab": 2 },
   "chordRootRange": [45, 57], "keyRange": [36, 76], "velocity": [90, 120],
   "hold": [1.2, 2.2], "stabChance": 0.25, "stabHold": 0.25, "strumMs": 0 },

 { "id": "bell-tines", "name": "Bell Tines",
   "description": "16 Levels: glassy FM bells and tines, mostly single notes, short and clean for chromatic riffs.",
   "recipe": "bell", "scale": "minor", "maxLength": 2.0,
   "chords": { "5": 3, "sus2": 2, "maj": 1, "min": 1, "add9": 1 },
   "voicings": { "close": 3, "open": 2 },
   "chordRatio": 0.15, "keyTypes": { "note": 7, "octave": 2, "fifth": 1, "stab": 0 },
   "chordRootRange": [60, 71], "keyRange": [55, 84], "velocity": [75, 115],
   "hold": [0.25, 0.7], "stabChance": 0.3, "stabHold": 0.12, "strumMs": 0 },

 { "id": "organ-stabs", "name": "Organ Stabs",
   "description": "16 Levels: drawbar organ stabs and single hits with key click, cut short for rhythmic chops.",
   "recipe": "organ", "scale": "dorian", "maxLength": 1.0,
   "chords": { "min": 4, "m7": 4, "maj": 3, "7": 2, "m9": 2, "sus4": 2, "9": 1, "5": 2 },
   "voicings": { "close": 4, "inv1": 3, "inv2": 3 },
   "chordRatio": 0.55, "keyTypes": { "note": 4, "octave": 3, "fifth": 2, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [43, 72], "velocity": [95, 120],
   "hold": [0.2, 0.5], "stabChance": 0.7, "stabHold": 0.16, "strumMs": 0 },

 { "id": "plucks", "name": "Plucks",
   "description": "16 Levels: plucked-string transients (Karplus-Strong), bright to muted, tight decays.",
   "recipe": "pluck", "scale": "minor", "maxLength": 1.5,
   "chords": { "5": 3, "min": 1, "maj": 1, "sus2": 1 },
   "voicings": { "close": 2, "open": 1 },
   "chordRatio": 0.1, "keyTypes": { "note": 7, "octave": 2, "fifth": 2, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [40, 79], "velocity": [80, 120],
   "hold": [0.1, 0.35], "stabChance": 0.3, "stabHold": 0.1, "strumMs": 6 },

 { "id": "choir", "name": "Choir Notes",
   "description": "16 Levels: formant choir 'aah / ooh / oh' notes and small chords; great pitched down on the pads.",
   "recipe": "choir", "scale": "minor", "maxLength": 2.5,
   "chords": { "5": 3, "min": 3, "maj": 2, "sus2": 2, "madd9": 1 },
   "voicings": { "close": 2, "open": 3, "spread": 1 },
   "chordRatio": 0.3, "keyTypes": { "note": 6, "octave": 3, "fifth": 2, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [45, 74], "velocity": [70, 105],
   "hold": [0.7, 1.5], "stabChance": 0.0, "stabHold": 0.3, "strumMs": 0 },

 { "id": "detuned-synth", "name": "Detuned Synth",
   "description": "16 Levels: detuned supersaw notes and power stabs with a snappy filter envelope.",
   "recipe": "supersaw", "scale": "minor", "maxLength": 1.5,
   "chords": { "5": 4, "min": 3, "maj": 2, "sus2": 2, "m7": 1 },
   "voicings": { "close": 3, "open": 2, "inv1": 1 },
   "chordRatio": 0.3, "keyTypes": { "note": 6, "octave": 3, "fifth": 3, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [36, 72], "velocity": [90, 120],
   "hold": [0.2, 0.6], "stabChance": 0.5, "stabHold": 0.15, "strumMs": 0 },

 { "id": "one-shot-palette", "name": "One-Shot Palette",
   "description": "16 Levels: a mixed kit of bells, organ, plucks, choir, synth and keys - one sound per pad.",
   "recipe": "mixed", "scale": "minor", "maxLength": 1.8,
   "chords": { "5": 3, "min": 3, "maj": 2, "m7": 2, "sus2": 2 },
   "voicings": { "close": 3, "open": 2, "inv1": 1 },
   "chordRatio": 0.2, "keyTypes": { "note": 7, "octave": 2, "fifth": 2, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [45, 79], "velocity": [80, 115],
   "hold": [0.2, 0.8], "stabChance": 0.5, "stabHold": 0.14, "strumMs": 0 }
]
)JSON";

static juce::var weightsToVar (const std::vector<Weighted<juce::String>>& w)
{
    auto* obj = new juce::DynamicObject();
    for (auto& e : w) obj->setProperty (e.value, e.weight);
    return juce::var (obj);
}

static juce::var rangeVar (double a, double b)
{
    juce::Array<juce::var> arr { a, b };
    return juce::var (arr);
}

juce::var Style::toVar() const
{
    auto* o = new juce::DynamicObject();
    o->setProperty ("id", id);
    o->setProperty ("name", name);
    o->setProperty ("description", description);
    o->setProperty ("scale", scale);
    o->setProperty ("chords", weightsToVar (chords));

    std::vector<Weighted<juce::String>> v;
    for (auto& e : voicings) v.push_back ({ voicingId (e.value), e.weight });
    o->setProperty ("voicings", weightsToVar (v));

    o->setProperty ("chordRatio", chordRatio);
    auto* kt = new juce::DynamicObject();
    kt->setProperty ("note", keyNote);
    kt->setProperty ("octave", keyOctave);
    kt->setProperty ("fifth", keyFifth);
    kt->setProperty ("stab", keyStab);
    o->setProperty ("keyTypes", juce::var (kt));
    o->setProperty ("chordRootRange", rangeVar (chordRootLow, chordRootHigh));
    o->setProperty ("keyRange", rangeVar (keyLow, keyHigh));
    o->setProperty ("velocity", rangeVar (velocityLow, velocityHigh));
    o->setProperty ("hold", rangeVar (holdLow, holdHigh));
    o->setProperty ("stabChance", stabChance);
    o->setProperty ("stabHold", stabHold);
    o->setProperty ("strumMs", strumMs);
    o->setProperty ("recipe", recipe);
    o->setProperty ("maxLength", maxLength);
    return juce::var (o);
}

template <typename T>
static void readRange (const juce::var& v, const char* key, T& lo, T& hi)
{
    auto r = v[key];
    if (r.isArray() && r.size() == 2)
    {
        lo = (T) (double) r[0];
        hi = (T) (double) r[1];
        if (hi < lo) std::swap (lo, hi);
    }
}

bool Style::fromVar (const juce::var& v, Style& s, juce::String& error)
{
    if (! v.isObject()) { error = "style must be a JSON object"; return false; }

    s = Style();
    s.id = v["id"].toString();
    s.name = v.getProperty ("name", s.id).toString();
    s.description = v["description"].toString();
    if (s.id.isEmpty()) s.id = s.name.toLowerCase().replaceCharacter (' ', '-');
    if (s.id.isEmpty()) { error = "style needs an \"id\" or \"name\""; return false; }

    if (v.hasProperty ("scale")) s.scale = v["scale"].toString();
    std::vector<int> tmp;
    if (! getScale (s.scale, tmp))
    {
        error = "unknown scale '" + s.scale + "' (use one of: " + allScaleNames().joinIntoString (", ") + ")";
        return false;
    }

    if (auto* chords = v["chords"].getDynamicObject())
        for (auto& p : chords->getProperties())
        {
            if (findQuality (p.name.toString()) == nullptr)
            {
                juce::StringArray ids;
                for (auto& q : allQualities()) ids.add (q.id);
                error = "unknown chord quality '" + p.name.toString() + "' (known: " + ids.joinIntoString (", ") + ")";
                return false;
            }
            if ((float) p.value > 0)
                s.chords.push_back ({ p.name.toString(), (float) p.value });
        }

    if (s.chords.empty()) { error = "style '" + s.id + "' needs at least one entry in \"chords\""; return false; }

    if (auto* vo = v["voicings"].getDynamicObject())
        for (auto& p : vo->getProperties())
        {
            Voicing voicing;
            if (! parseVoicing (p.name.toString(), voicing))
            {
                error = "unknown voicing '" + p.name.toString() + "' (known: " + allVoicingIds().joinIntoString (", ") + ")";
                return false;
            }
            if ((float) p.value > 0)
                s.voicings.push_back ({ voicing, (float) p.value });
        }

    if (s.voicings.empty())
        s.voicings.push_back ({ Voicing::Close, 1.0f });

    s.chordRatio = juce::jlimit (0.0f, 1.0f, (float) v.getProperty ("chordRatio", s.chordRatio));

    auto kt = v["keyTypes"];
    if (kt.isObject())
    {
        s.keyNote   = juce::jmax (0.0f, (float) kt.getProperty ("note", 0));
        s.keyOctave = juce::jmax (0.0f, (float) kt.getProperty ("octave", 0));
        s.keyFifth  = juce::jmax (0.0f, (float) kt.getProperty ("fifth", 0));
        s.keyStab   = juce::jmax (0.0f, (float) kt.getProperty ("stab", 0));
        if (s.keyNote + s.keyOctave + s.keyFifth + s.keyStab <= 0) s.keyNote = 1;
    }

    readRange (v, "chordRootRange", s.chordRootLow, s.chordRootHigh);
    readRange (v, "keyRange", s.keyLow, s.keyHigh);
    readRange (v, "velocity", s.velocityLow, s.velocityHigh);
    readRange (v, "hold", s.holdLow, s.holdHigh);

    s.chordRootLow  = juce::jlimit (24, 96, s.chordRootLow);
    s.chordRootHigh = juce::jlimit (s.chordRootLow + 11, 108, s.chordRootHigh);
    s.keyLow  = juce::jlimit (21, 108, s.keyLow);
    s.keyHigh = juce::jlimit (s.keyLow, 108, s.keyHigh);
    s.velocityLow  = juce::jlimit (1, 127, s.velocityLow);
    s.velocityHigh = juce::jlimit (s.velocityLow, 127, s.velocityHigh);
    s.holdLow  = juce::jlimit (0.05, 20.0, s.holdLow);
    s.holdHigh = juce::jlimit (s.holdLow, 20.0, s.holdHigh);

    s.stabChance = juce::jlimit (0.0f, 1.0f, (float) v.getProperty ("stabChance", s.stabChance));
    s.stabHold   = juce::jlimit (0.03, 5.0, (double) v.getProperty ("stabHold", s.stabHold));
    s.strumMs    = juce::jlimit (0.0, 250.0, (double) v.getProperty ("strumMs", s.strumMs));
    s.maxLength  = juce::jlimit (0.0, 30.0, (double) v.getProperty ("maxLength", 0.0));

    s.recipe = v.getProperty ("recipe", "epiano").toString().toLowerCase();
    Recipe r;
    if (s.recipe != "mixed" && ! parseRecipe (s.recipe, r))
    {
        error = "unknown recipe '" + s.recipe + "' (use one of: " + recipeIds().joinIntoString (", ") + ", mixed)";
        return false;
    }
    return true;
}

const juce::Array<Style>& builtInStyles()
{
    static const juce::Array<Style> styles = []
    {
        juce::Array<Style> result;
        auto parsed = juce::JSON::parse (builtInStyleJson);
        jassert (parsed.isArray());

        for (auto& v : *parsed.getArray())
        {
            Style s;
            juce::String err;
            if (Style::fromVar (v, s, err))
                result.add (s);
            else
                jassertfalse;
        }
        return result;
    }();
    return styles;
}

const Style* findBuiltInStyle (const juce::String& idOrName)
{
    for (auto& s : builtInStyles())
        if (s.id.equalsIgnoreCase (idOrName) || s.name.equalsIgnoreCase (idOrName))
            return &s;
    return nullptr;
}

bool loadStyleFile (const juce::File& f, Style& out, juce::String& error)
{
    if (! f.existsAsFile()) { error = "style file not found: " + f.getFullPathName(); return false; }

    juce::var v;
    auto result = juce::JSON::parse (f.loadFileAsString(), v);
    if (result.failed()) { error = "JSON error in " + f.getFileName() + ": " + result.getErrorMessage(); return false; }

    if (v.isArray() && v.size() > 0)
        v = v[0];

    return Style::fromVar (v, out, error);
}
}
