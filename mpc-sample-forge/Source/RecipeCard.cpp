#include "RecipeCard.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <map>
#include <set>

namespace forge
{
//==============================================================================
// Small text helpers

static juce::String normalise (juce::String t)
{
    // the book's typography -> plain text
    return t.replace (juce::CharPointer_UTF8 ("\xe2\x99\xaf"), "#")      // sharp sign
            .replace (juce::CharPointer_UTF8 ("\xe2\x99\xad"), "b")      // flat sign
            .replace (juce::CharPointer_UTF8 ("\xc2\xb7"), " ")          // middle dot
            .replace (juce::CharPointer_UTF8 ("\xe2\x80\x93"), "-")      // en dash
            .replace (juce::CharPointer_UTF8 ("\xe2\x80\x94"), "-")      // em dash
            .replace (juce::CharPointer_UTF8 ("\xe2\x80\x99"), "'")
            .replace ("\t", " ").replace ("\r", "");
}

static juce::StringArray words (const juce::String& line)
{
    auto t = juce::StringArray::fromTokens (line, " ,;", "");
    t.trim();
    t.removeEmptyStrings();
    return t;
}

int parseCardNote (const juce::String& tokenIn, bool mpcNotation)
{
    auto token = normalise (tokenIn).trim();
    if (token.length() < 2 || token.length() > 5) return -1;
    auto letter = juce::CharacterFunctions::toUpperCase (token[0]);
    if (letter < 'A' || letter > 'G') return -1;
    int i = 1;
    int accidental = 0;
    if (token[i] == '#' || (token[i] == 's' && i + 1 < token.length() && (juce::CharacterFunctions::isDigit (token[i + 1]) || token[i + 1] == '-'))) { accidental = 1; ++i; }
    else if (token[i] == 'b' && i + 1 < token.length() && (juce::CharacterFunctions::isDigit (token[i + 1]) || token[i + 1] == '-')) { accidental = -1; ++i; }
    auto octaveText = token.substring (i);
    if (octaveText.isEmpty() || ! octaveText.trimCharactersAtStart ("-").containsOnly ("0123456789") || octaveText.length() > 2) return -1;
    static const int base[] = { 9, 11, 0, 2, 4, 5, 7 };   // A B C D E F G
    const int pc = (base[letter - 'A'] + accidental + 12) % 12;
    const int octave = octaveText.getIntValue();
    // B#3 / Cb4 wrap across the octave line
    int midi = pc + 12 * (octave + (mpcNotation ? 2 : 1));
    if (letter == 'B' && accidental == 1) midi += 12;
    if (letter == 'C' && accidental == -1) midi -= 12;
    return juce::isPositiveAndBelow (midi, 128) ? midi : -1;
}

/** "F#5" -> "Fs5": how sharps are written in the book's file names. */
static juce::String fileNoteName (int midi, bool mpcNotation, bool flats = false)
{
    auto pc = pitchClassName (midi % 12);
    if (flats) return pc + juce::String (midi / 12 - (mpcNotation ? 2 : 1));
    static const std::map<juce::String, juce::String> sharpNames { { "Db", "Cs" }, { "Eb", "Ds" }, { "Gb", "Fs" }, { "Ab", "Gs" }, { "Bb", "As" } };
    auto it = sharpNames.find (pc);
    auto name = it != sharpNames.end() ? it->second : pc;
    return name + juce::String (midi / 12 - (mpcNotation ? 2 : 1));
}

static juce::String safeLabel (const juce::String& s)
{
    return s.replace ("#", "s").retainCharacters ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-");
}

static bool looksLikeChordName (const juce::String& t)
{
    if (t.isEmpty() || t[0] < 'A' || t[0] > 'G') return false;
    if (parseCardNote (t, false) >= 0) return false;                        // that's a note
    return t.length() <= 12 && ! t.containsAnyOf (":=");
}

/** Built-in sounds named in a card's sound description. */
static void enginesFromText (const juce::String& textIn, RecipeCard& card)
{
    auto text = textIn.toLowerCase();
    struct W { const char* word; const char* engine; };
    static const W table[] = {
        { "wurli", "wurli" }, { "electric piano", "epiano" }, { "e-piano", "epiano" }, { "e.piano", "epiano" }, { "rhodes", "epiano" },
        { "epiano", "epiano" }, { " ep ", "epiano" }, { "keys", "epiano" }, { "dx", "epbell" }, { "bell", "bell" }, { "glock", "bell" },
        { "music box", "musicbox" }, { "organ", "organ" }, { "clav", "clav" }, { "harp", "harp" }, { "pizz", "pizz" },
        { "string", "strings" }, { "brass", "brass" }, { "horn", "brass" }, { "sax", "sax" }, { "flute", "flute" }, { "choir", "choir" },
        { "vocal", "choir" }, { "talkbox", "talkbox" }, { "guitar", "pluck" }, { "pluck", "pluck" }, { "supersaw", "supersaw" },
        { "saw", "supersaw" }, { "lead", "lead" }, { "synth", "lead" }, { "808", "808" }, { "bass", "bass" }, { "pad", "pad" } };
    juce::StringArray found;
    for (auto& w : table)
        if ((" " + text + " ").contains (w.word) && ! found.contains (w.engine)) found.add (w.engine);
    found.removeString ("pad");
    const bool pad = text.contains ("pad");
    if (! found.isEmpty()) card.chordEngine = card.leadEngine = found[0];
    else if (pad) card.chordEngine = card.leadEngine = "pad";
    for (auto& f : found) if (f == "lead" || f == "bell" || f == "flute" || f == "sax" || f == "pluck") { card.leadEngine = f; break; }
    card.textureEngine = pad ? "pad" : found.contains ("strings") ? "strings" : "pad";
}

/** Effects described in words ("chorus at 15% wet", "dotted-eighth delay", "short plate reverb",
    "high-pass around 120 Hz") or as a FORGE "fx:" list ("chorus 15, delay 1/8d 10, plate 20, highpass 120"). */
static juce::var effectsFromText (const juce::String& textIn)
{
    auto text = textIn.toLowerCase().replace ("high pass", "high-pass").replace ("highpass", "high-pass")
                                    .replace ("low pass", "low-pass").replace ("lowpass", "low-pass");
    juce::Array<juce::var> fx;
    auto numberAfter = [&text] (const juce::String& word, double fallback)
    {
        auto at = text.indexOf (word);
        if (at < 0) return fallback;
        auto rest = text.substring (at + word.length(), at + word.length() + 40);
        for (int i = 0; i < rest.length(); ++i)
            if (juce::CharacterFunctions::isDigit (rest[i])) return rest.substring (i).getDoubleValue();
        return fallback;
    };
    auto add = [&fx] (std::initializer_list<std::pair<const char*, juce::var>> props)
    {
        auto* o = new juce::DynamicObject();
        for (auto& [k, v] : props) o->setProperty (k, v);
        fx.add (juce::var (o));
    };
    if (text.contains ("high-pass")) add ({ { "type", "highpass" }, { "freq", juce::jlimit (20.0, 2000.0, numberAfter ("high-pass", 120)) }, { "poles", 2 } });
    if (text.contains ("low-pass"))  add ({ { "type", "lowpass" }, { "freq", juce::jlimit (300.0, 18000.0, numberAfter ("low-pass", 6000)) }, { "poles", 2 } });
    if (text.contains ("tape") || text.contains ("saturat")) add ({ { "type", "tape" }, { "drive", 0.3 } });
    if (text.contains ("chorus"))
        add ({ { "type", "chorus" }, { "rate", 0.7 }, { "depth", 0.3 }, { "mix", juce::jlimit (0.05, 1.0, numberAfter ("chorus", 25) / 100.0) } });
    if (text.contains ("delay"))
    {
        juce::String sync = "1/8";
        if (text.contains ("dotted") || text.contains ("1/8d") || text.contains ("1/8.")) sync = "1/8d";
        else if (text.contains ("quarter") || text.contains ("1/4")) sync = "1/4";
        else if (text.contains ("1/16") || text.contains ("sixteenth")) sync = "1/16";
        auto mix = text.contains ("quiet") || text.contains ("subtle") ? 0.1 : juce::jlimit (0.03, 0.6, numberAfter ("delay", 15) / 100.0);
        if (mix > 0.6 || mix < 0.03) mix = 0.15;
        add ({ { "type", "delay" }, { "sync", sync }, { "feedback", 0.25 }, { "mix", mix } });
    }
    if (text.contains ("reverb") || text.contains ("plate") || text.contains (" hall") || text.contains ("room"))
    {
        juce::String kind = text.contains ("plate") ? "plate" : text.contains ("hall") ? "hall" : text.contains ("room") ? "room" : "plate";
        auto decay = text.contains ("short") ? 0.9 : text.contains ("long") ? 2.4 : 1.4;
        add ({ { "type", "reverb" }, { "kind", kind }, { "decay", decay }, { "mix", 0.2 } });
    }
    return fx.isEmpty() ? juce::var() : juce::var (fx);
}

static double beatsFor (const juce::String& d)
{
    auto t = d.toLowerCase().trim();
    double v = t.startsWith ("w") ? 4 : t.startsWith ("h") ? 2 : t.startsWith ("q") ? 1 : t.startsWith ("e") ? 0.5 : t.startsWith ("s") ? 0.25 : 1;
    if (t.endsWith (".") || t.startsWith ("d")) v *= 1.5;
    return v;
}

//==============================================================================
/** Reads a note and remembers whether the card spells notes with flats. */
static int cardNote (RecipeCard& card, const juce::String& token)
{
    auto midi = parseCardNote (token, card.mpcNotation);
    if (midi >= 0 && token.length() > 2 && token[1] == 'b') card.prefersFlats = true;
    return midi;
}

//==============================================================================
// FORGE CARD blocks

static void readForgeLine (RecipeCard& card, const juce::String& lineIn)
{
    auto line = lineIn.trim();
    if (line.isEmpty()) return;
    auto lower = line.toLowerCase();
    auto value = line.fromFirstOccurrenceOf (":", false, false).trim();
    if (lower.startsWith ("name:"))      { card.name = value; return; }
    if (lower.startsWith ("code:"))      { card.code = safeLabel (value); return; }
    if (lower.startsWith ("key:"))       { card.key = value; return; }
    if (lower.startsWith ("tempo:"))     { card.tempo = juce::jlimit (30.0, 300.0, value.getDoubleValue()); return; }
    if (lower.startsWith ("notation:"))  { card.mpcNotation = value.containsIgnoreCase ("mpc"); return; }
    if (lower.startsWith ("sound:"))     { card.soundText = value; enginesFromText (value, card); return; }
    if (lower.startsWith ("fx:"))        { card.fx = effectsFromText (value); return; }

    if (! line.contains ("=")) { card.warnings.add ("not understood: " + line); return; }
    auto head = line.upToFirstOccurrenceOf ("=", false, false).trim();
    auto rest = line.fromFirstOccurrenceOf ("=", false, false);
    auto bodyAndOptions = juce::StringArray::fromTokens (rest, "|", "");
    auto headWords = words (head);
    if (headWords.isEmpty()) return;
    auto kind = headWords[0].toUpperCase();

    if (kind == "MELODY")
    {
        double barStart = 0;
        for (auto& bar : bodyAndOptions)
        {
            double pos = 0;
            for (auto& tok : words (bar))
            {
                auto note = tok.upToFirstOccurrenceOf ("/", false, false);
                auto len = beatsFor (tok.fromFirstOccurrenceOf ("/", false, false));
                auto midi = cardNote (card, note);
                if (midi >= 0) card.melody.push_back ({ midi, barStart + pos, len });
                pos += len;
            }
            barStart += 4;
        }
        return;
    }

    CardItem item;
    for (auto& tok : words (bodyAndOptions[0]))
    {
        auto midi = cardNote (card, tok);
        if (midi >= 0) item.notes.push_back (midi);
        else card.warnings.add ("not a note: " + tok + " (in " + head + ")");
    }
    for (int k = 1; k < bodyAndOptions.size(); ++k)
    {
        auto opt = bodyAndOptions[k].trim().toLowerCase();
        auto number = opt.retainCharacters ("0123456789.").getDoubleValue();
        if (opt.startsWith ("vel")) item.velocity = juce::jlimit (1, 127, (int) number);
        else if (opt.startsWith ("attack")) item.attackMs = number;
        else if (opt.startsWith ("hold")) item.holdSec = opt.contains ("bar") ? -number : number;   // negative = bars
        else if (opt.startsWith ("bar")) item.holdSec = -number;
    }
    if (item.notes.empty()) return;

    if (kind == "CH" || kind == "CHORD")
    {
        item.kind = "CH";
        item.label = headWords.size() > 1 ? headWords[1] : "Chord" + juce::String ((int) card.chordSequence.size() + 1);
        card.chordSequence.push_back ({ item.label, item.notes });
        card.items.push_back (item);
    }
    else if (kind == "LEAD" || kind == "SOFT" || kind == "NOTE" || kind == "NOTES")
    {
        for (auto n : item.notes)
        {
            CardItem one = item;
            one.kind = kind == "SOFT" ? "SOFT" : "LEAD";
            if (one.kind == "SOFT" && item.velocity == 100) one.velocity = 60;
            one.notes = { n };
            one.label = fileNoteName (n, card.mpcNotation, card.prefersFlats);
            card.items.push_back (one);
        }
    }
    else if (kind == "TEXTURE" || kind == "TEX")
    {
        item.kind = "TEXTURE";
        item.label = headWords.size() > 1 ? safeLabel (headWords[1]).toUpperCase() : "TEXTURE";
        card.items.push_back (item);
    }
    else card.warnings.add ("unknown line type: " + kind);
}

//==============================================================================
// The book's own text (best effort)

static void readBookText (RecipeCard& card, const juce::StringArray& lines)
{
    juce::String all;
    juce::StringArray wavNames, midNames;
    std::vector<std::pair<juce::String, std::vector<int>>> tableChords;
    std::map<int, std::vector<MelodyNote>> melodyBars;

    for (auto& raw : lines)
    {
        auto line = normalise (raw).trim();
        all << line << "\n";
        auto lower = line.toLowerCase().trimCharactersAtStart ("-* ");
        if (lower.startsWith ("key:"))   { card.key = line.fromFirstOccurrenceOf (":", false, false).trim(); continue; }
        if (lower.startsWith ("tempo:")) { card.tempo = juce::jlimit (30.0, 300.0, line.fromFirstOccurrenceOf (":", false, false).getDoubleValue()); continue; }
        if (lower.startsWith ("main sound:") || lower.startsWith ("sound:"))
        {
            card.soundText = line.fromFirstOccurrenceOf (":", false, false).trim();
            continue;
        }

        // file names: DL_BHC_Dmaj9_CH.wav ...
        for (auto& w : words (line))
        {
            if (w.endsWithIgnoreCase (".wav") && w.contains ("_")) wavNames.add (w.dropLastCharacters (4));
            else if (w.endsWithIgnoreCase (".mid") && w.contains ("_")) midNames.add (w.dropLastCharacters (4));
        }
        if (line.containsIgnoreCase (".wav") || line.containsIgnoreCase (".mid")) continue;

        auto toks = words (line);
        // melody rows: "1  1: F#5 q; 2: A5 e; 2&: C#6 e; ..."
        if (toks.size() > 2 && toks[0].containsOnly ("0123456789") && line.contains (":"))
        {
            const int bar = toks[0].getIntValue();
            std::vector<MelodyNote> notes;
            auto cells = juce::StringArray::fromTokens (line.fromFirstOccurrenceOf (toks[0], false, false), ";", "");
            for (auto& cell : cells)
            {
                if (! cell.contains (":")) continue;
                auto beatText = cell.upToFirstOccurrenceOf (":", false, false).trim();
                auto after = words (cell.fromFirstOccurrenceOf (":", false, false));
                if (after.isEmpty() || ! beatText.retainCharacters ("0123456789").isNotEmpty()) continue;
                auto midi = cardNote (card, after[0]);
                if (midi < 0) continue;
                double beat = beatText.retainCharacters ("0123456789").getDoubleValue() - 1.0 + (beatText.contains ("&") ? 0.5 : 0.0);
                notes.push_back ({ midi, (bar - 1) * 4.0 + beat, after.size() > 1 ? beatsFor (after[1]) : 1.0 });
            }
            if (! notes.empty()) { melodyBars[bar] = notes; continue; }
        }

        // chord rows: "1  Dmaj9  D3 F#3 A3 C#4 E4"
        for (int i = 0; i < toks.size(); ++i)
        {
            if (! looksLikeChordName (toks[i])) continue;
            std::vector<int> notes;
            for (int j = i + 1; j < toks.size(); ++j)
            {
                auto midi = cardNote (card, toks[j]);
                if (midi < 0) break;
                notes.push_back (midi);
            }
            if (notes.size() >= 2) { tableChords.push_back ({ toks[i], notes }); break; }
        }
    }

    if (card.soundText.isEmpty())
        card.soundText = all.fromFirstOccurrenceOf ("sound", false, true).upToFirstOccurrenceOf ("\n", false, false);
    enginesFromText (card.soundText + " " + all.fromFirstOccurrenceOf ("sound design", false, true).upToFirstOccurrenceOf ("render", false, true), card);
    card.fx = effectsFromText (all.fromFirstOccurrenceOf ("sound design", false, true).upToFirstOccurrenceOf ("render and", false, true));

    for (auto& [bar, notes] : melodyBars) for (auto& n : notes) card.melody.push_back (n);
    card.chordSequence = tableChords;

    // the file list says what to render and how to name it
    if (! wavNames.isEmpty())
    {
        auto parts = juce::StringArray::fromTokens (wavNames[0], "_", "");
        if (parts.size() >= 3) card.code = parts[0] + "_" + parts[1];
    }
    auto codePrefix = card.code.isNotEmpty() ? card.code + "_" : juce::String();
    std::set<juce::String> usedChords;
    for (auto& name : wavNames)
    {
        auto body = name.startsWith (codePrefix) ? name.substring (codePrefix.length()) : name;
        auto kind = body.fromLastOccurrenceOf ("_", false, false).toUpperCase();
        auto label = body.upToLastOccurrenceOf ("_", false, false);
        CardItem item;
        item.fileName = name;
        if (kind == "CH")
        {
            for (auto& [chordName, notes] : tableChords)
                if (safeLabel (chordName.replace ("/", "")) .equalsIgnoreCase (label) || safeLabel (chordName).equalsIgnoreCase (label))
                {
                    item.kind = "CH"; item.label = label; item.notes = notes;
                    usedChords.insert (chordName);
                    break;
                }
            if (item.notes.empty()) { card.warnings.add ("no notes for " + name + " in the chord table"); continue; }
        }
        else if (kind == "LEAD" || kind == "SOFT")
        {
            auto midi = parseCardNote (label, card.mpcNotation);
            if (midi < 0) { card.warnings.add ("can't read the note in " + name); continue; }
            item.kind = kind; item.label = label; item.notes = { midi };
            if (kind == "SOFT") item.velocity = 60;
        }
        else
        {
            // a texture: WARM_PAD_SWELL, REVERSE_PAD_SWELL, GLASS_SHIMMER ...
            item.kind = "TEXTURE";
            item.label = body.toUpperCase();
            if (! tableChords.empty()) item.notes = tableChords.front().second;
            if (item.notes.empty()) { card.warnings.add ("no chord to build " + name + " from"); continue; }
            if (item.label.contains ("GLASS") || item.label.contains ("SHIMMER") || item.label.contains ("BELL"))
                for (auto& n : item.notes) n = juce::jmin (115, n + 12);
        }
        card.items.push_back (item);
    }
    // chords in the table that the file list doesn't mention, or no file list at all
    for (auto& [chordName, notes] : tableChords)
        if (usedChords.count (chordName) == 0)
        {
            CardItem item; item.kind = "CH"; item.label = safeLabel (chordName.replace ("/", "")); item.notes = notes;
            card.items.push_back (item);
        }
    if (wavNames.isEmpty())
    {
        std::set<int> pitches;
        for (auto& m : card.melody) pitches.insert (m.note);
        for (auto p : pitches)
        {
            CardItem item; item.kind = "LEAD"; item.notes = { p }; item.label = fileNoteName (p, card.mpcNotation, card.prefersFlats);
            card.items.push_back (item);
        }
    }
    for (auto& m : midNames)
    {
        if (m.containsIgnoreCase ("CHORD")) card.chordsMidiName = m;
        else if (m.containsIgnoreCase ("MELOD")) card.melodyMidiName = m;
    }
}

//==============================================================================
static void finishCard (RecipeCard& card, int number)
{
    if (card.name.isEmpty()) card.name = "Recipe " + juce::String (number);
    if (card.code.isEmpty())
    {
        juce::String initials;
        for (auto& w : words (card.name)) if (juce::CharacterFunctions::isLetter (w[0])) initials += juce::String::charToString (juce::CharacterFunctions::toUpperCase (w[0]));
        card.code = "RC_" + (initials.isNotEmpty() ? initials : juce::String (number));
    }
    const double beat = 60.0 / card.tempo;
    for (auto& it : card.items)
    {
        if (it.fileName.isEmpty()) it.fileName = card.code + "_" + safeLabel (it.label) + (it.kind == "TEXTURE" ? juce::String() : "_" + it.kind);
        if (it.holdSec < 0) it.holdSec = -it.holdSec * 4.0 * beat;                         // bars
        if (it.holdSec <= 0)
            it.holdSec = it.kind == "CH" ? juce::jmin (4.0, 4.0 * beat)                     // a one-bar gate
                       : it.kind == "TEXTURE" ? 4.0 : juce::jlimit (0.35, 1.2, beat);
        if (it.kind == "TEXTURE")
        {
            if (it.attackMs <= 0 && (it.label.contains ("SWELL") || it.label.contains ("PAD"))) it.attackMs = 800;
            it.reverse = it.label.contains ("REVERSE");
            it.shimmer = it.label.contains ("GLASS") || it.label.contains ("SHIMMER") || it.label.contains ("BELL");
            it.engine = card.textureEngine;   // glass / shimmer: the pad an octave up + a shimmer delay (bells would add off-key partials)
        }
        else it.engine = it.kind == "CH" ? card.chordEngine : card.leadEngine;
    }
    if (card.chordsMidiName.isEmpty() && ! card.chordSequence.empty()) card.chordsMidiName = card.code + "_CHORDS";
    if (card.melodyMidiName.isEmpty() && ! card.melody.empty()) card.melodyMidiName = card.code + "_MELODY";
}

std::vector<RecipeCard> parseRecipeCards (const juce::String& textIn, bool mpcNotation)
{
    std::vector<RecipeCard> cards;
    auto text = normalise (textIn);
    auto lines = juce::StringArray::fromLines (text);

    // 1. FORGE CARD blocks
    for (int i = 0; i < lines.size(); ++i)
        if (lines[i].containsIgnoreCase ("FORGE CARD"))
        {
            RecipeCard card;
            card.mpcNotation = mpcNotation;
            for (++i; i < lines.size() && ! (lines[i].contains ("===") && lines[i].containsIgnoreCase ("END")); ++i)
                readForgeLine (card, lines[i]);
            finishCard (card, (int) cards.size() + 1);
            cards.push_back (card);
        }
    if (! cards.empty()) return cards;

    // 2. the book's text: numbered recipes ("1. Blue Hour Convertible")
    std::vector<std::pair<juce::String, juce::StringArray>> sections;
    for (auto& line : lines)
    {
        auto t = line.trim();
        auto first = t.upToFirstOccurrenceOf (".", false, false);
        const bool heading = first.isNotEmpty() && first.length() <= 2 && first.containsOnly ("0123456789")
                             && t.length() > first.length() + 3 && t[first.length() + 1] == ' '
                             && juce::CharacterFunctions::isUpperCase (t[first.length() + 2]) && ! t.contains (":");
        if (heading) sections.push_back ({ t.fromFirstOccurrenceOf (".", false, false).trim(), {} });
        else if (! sections.empty()) sections.back().second.add (line);
    }
    if (sections.empty()) sections.push_back ({ {}, lines });
    for (auto& [name, body] : sections)
    {
        RecipeCard card;
        card.mpcNotation = mpcNotation;
        card.name = name;
        readBookText (card, body);
        if (card.items.empty() && card.melody.empty()) continue;
        finishCard (card, (int) cards.size() + 1);
        cards.push_back (card);
    }
    return cards;
}

juce::String RecipeCard::describe() const
{
    juce::String t;
    t << name << "   (" << code << ")   key " << (key.isNotEmpty() ? key : juce::String ("?")) << ", " << juce::roundToInt (tempo) << " BPM, notes read as "
      << (mpcNotation ? "MPC names (C3 = middle C)" : "standard names (C4 = middle C)") << "\n";
    t << "Sound: chords " << chordEngine << ", notes " << leadEngine << ", textures " << textureEngine;
    if (fx.isArray())
    {
        juce::StringArray f;
        for (auto& e : *fx.getArray()) f.add (e["type"].toString());
        t << "   FX: " << f.joinIntoString (", ");
    }
    t << "\n\n";
    for (auto& it : items)
    {
        juce::StringArray n, mpc;
        for (auto x : it.notes) { n.add (prefersFlats ? fileNoteName (x, mpcNotation, true) : fileNoteName (x, mpcNotation).replace ("s", "#")); mpc.add (midiNoteName (x)); }
        t << it.fileName.paddedRight (' ', 30) << " " << n.joinIntoString (" ");
        if (! mpcNotation) t << "   (MPC: " << mpc.joinIntoString (" ") << ")";
        if (it.velocity != 100) t << "  vel " << it.velocity;
        if (it.kind == "TEXTURE") t << "  attack " << juce::roundToInt (it.attackMs) << " ms" << (it.reverse ? ", reversed" : "");
        t << "\n";
    }
    if (! chordSequence.empty()) t << chordsMidiName << ".mid   " << (int) chordSequence.size() << " chords, one per bar\n";
    if (! melody.empty()) t << melodyMidiName << ".mid   " << (int) melody.size() << " melody notes\n";
    for (auto& w : warnings) t << "! " << w << "\n";
    return t;
}

//==============================================================================
std::vector<SampleSpec> planCard (const RecipeCard& card, const Style& style, const PlanOptions& opt, juce::String& error)
{
    std::vector<SampleSpec> out;
    if (card.items.empty()) { error = "the card has nothing to render"; return out; }
    juce::Random rng (opt.seed);
    const auto variation = rng.nextInt64();
    int index = 0;
    for (auto& it : card.items)
    {
        SampleSpec s;
        s.index = ++index;
        s.kind = it.kind == "CH" ? "Chord" : (it.notes.size() > 1 ? "Chord" : "Key");
        s.label = it.label;
        s.voicing = it.kind == "CH" ? juce::String ("card") : it.kind.toLowerCase();
        s.fileStem = safeFileName (it.fileName);
        s.stemTail = s.fileStem;
        s.engine = opt.engine.isNotEmpty() && opt.engine != "mixed" ? opt.engine : it.engine;
        s.variation = variation;
        s.styleId = style.id; s.tag = card.code; s.shortTag = card.code;
        s.processing = style.processing;
        if (card.fx.isArray() && ! opt.cardUseRecipeFx)
        {
            s.processing.fx = card.fx;           // the card's own effects instead of the recipe's
            s.processing.maxLength = 0;
            s.processing.ampDecayMs = 0;
            s.processing.pitchShift = 0;
            s.processing.fxTailMs = 400;
        }
        s.processing.tempo = card.tempo;
        if (it.kind == "TEXTURE")
        {
            s.processing.attackMs = it.attackMs;
            s.processing.maxLength = 0;
            if (it.shimmer)
            {
                juce::Array<juce::var> fx;
                if (s.processing.fx.isArray()) fx = *s.processing.fx.getArray();
                auto* hp = new juce::DynamicObject();
                hp->setProperty ("type", "highpass"); hp->setProperty ("freq", 400); hp->setProperty ("poles", 2);
                auto* sh = new juce::DynamicObject();
                sh->setProperty ("type", "shimmer"); sh->setProperty ("sync", "1/8"); sh->setProperty ("feedback", 0.45); sh->setProperty ("mix", 0.35);
                fx.insert (0, juce::var (hp));
                fx.add (juce::var (sh));
                s.processing.fx = juce::var (fx);
                s.processing.fxTailMs = 1200;
            }
            if (it.reverse)
            {
                juce::Array<juce::var> fx;
                if (s.processing.fx.isArray()) fx = *s.processing.fx.getArray();
                auto* r = new juce::DynamicObject();
                r->setProperty ("type", "reverse");
                fx.add (juce::var (r));
                s.processing.fx = juce::var (fx);
            }
        }
        // root: the chord's own root if its name starts with one, else the lowest note
        s.rootNote = *std::min_element (it.notes.begin(), it.notes.end());
        auto rootPc = parsePitchClass (it.label.substring (0, it.label.length() > 1 && (it.label[1] == 'b' || it.label[1] == 's' || it.label[1] == '#') ? 2 : 1)
                                           .replace ("s", "#"));
        if (it.kind == "CH" && rootPc >= 0)
            for (auto n : it.notes) if (n % 12 == rootPc) { s.rootNote = n; break; }
        for (size_t k = 0; k < it.notes.size(); ++k)
        {
            NoteEvent e;
            e.note = juce::jlimit (0, 127, it.notes[k] - s.processing.pitchShift);
            e.velocity = it.velocity;
            e.startSec = it.kind == "CH" ? 0.004 * (double) k : 0.0;       // a hint of a roll, like a player
            e.lengthSec = it.holdSec;
            s.notes.push_back (e);
        }
        out.push_back (s);
    }
    return out;
}

juce::StringArray writeCardMidi (const RecipeCard& card, const juce::File& folder)
{
    juce::StringArray written;
    const int ppq = 960;
    auto save = [&] (const juce::String& name, const juce::MidiMessageSequence& seq)
    {
        juce::MidiFile mf;
        mf.setTicksPerQuarterNote (ppq);
        juce::MidiMessageSequence meta;
        meta.addEvent (juce::MidiMessage::tempoMetaEvent ((int) (60000000.0 / card.tempo)), 0);
        meta.addEvent (juce::MidiMessage::timeSignatureMetaEvent (4, 4), 0);
        mf.addTrack (meta);
        mf.addTrack (seq);
        auto file = folder.getChildFile (safeFileName (name) + ".mid");
        juce::FileOutputStream stream (file);
        if (stream.openedOk()) { stream.setPosition (0); stream.truncate(); if (mf.writeTo (stream)) written.add (file.getFileName()); }
    };
    if (! card.chordSequence.empty())
    {
        juce::MidiMessageSequence seq;
        double bar = 0;
        for (auto& [name, notes] : card.chordSequence)
        {
            for (auto n : notes)
            {
                seq.addEvent (juce::MidiMessage::noteOn (1, n, (juce::uint8) 90), bar * 4 * ppq);
                seq.addEvent (juce::MidiMessage::noteOff (1, n), (bar + 1) * 4 * ppq - 10);
            }
            bar += 1;
        }
        seq.updateMatchedPairs();
        save (card.chordsMidiName, seq);
    }
    if (! card.melody.empty())
    {
        juce::MidiMessageSequence seq;
        for (auto& m : card.melody)
        {
            seq.addEvent (juce::MidiMessage::noteOn (1, m.note, (juce::uint8) 100), m.startBeats * ppq);
            seq.addEvent (juce::MidiMessage::noteOff (1, m.note), (m.startBeats + m.lengthBeats) * ppq - 10);
        }
        seq.updateMatchedPairs();
        save (card.melodyMidiName, seq);
    }
    return written;
}

juce::StringArray writeCardMidiForPlan (const PlanOptions& plan, const juce::File& folder)
{
    if (! plan.useCard) return {};
    auto cards = parseRecipeCards (plan.cardText, plan.cardMpcNotation);
    if (! juce::isPositiveAndBelow (plan.cardIndex, (int) cards.size())) return {};
    folder.createDirectory();
    return writeCardMidi (cards[(size_t) plan.cardIndex], folder);
}

juce::String forgeCardPrompt()
{
    return "At the end of every recipe, add a FORGE CARD block exactly in this format (plain text, no table, no bullets).\n"
           "Note names are standard (C4 = middle C), sharps as #. One line per item:\n\n"
           "=== FORGE CARD ===\n"
           "name: Blue Hour Convertible\n"
           "code: DL_BHC\n"
           "key: D major\n"
           "tempo: 92\n"
           "sound: soft electric piano + slow analog pad\n"
           "fx: chorus 15, delay 1/8d 10, plate 20, highpass 120\n"
           "CH Dmaj9 = D3 F#3 A3 C#4 E4\n"
           "CH Bm7 = B2 D3 F#3 A3\n"
           "LEAD = D5 E5 F#5 A5 B5 C#6\n"
           "SOFT = D5 F#5 A5 | velocity 60\n"
           "TEXTURE WARM_PAD_SWELL = D3 A3 F#4 | attack 800 | hold 4\n"
           "MELODY = F#5/q A5/e C#6/e B5/q A5/q | F#5/q E5/e F#5/e A5/q F#5/q\n"
           "=== END ===\n\n"
           "CH = a chord hit (exact notes, low to high). LEAD / SOFT = single notes (SOFT = soft velocity).\n"
           "TEXTURE NAME = a pad / swell / shimmer built on those notes (REVERSE in the name plays it backwards).\n"
           "MELODY = the melody, bars separated by |, each note NOTE/length with q = quarter, e = eighth, h = half, s = sixteenth.\n";
}
}
