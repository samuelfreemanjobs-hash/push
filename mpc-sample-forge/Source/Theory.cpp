#include "Theory.h"
#include <algorithm>
#include <set>

namespace forge
{
const std::vector<ChordQuality>& allQualities()
{
    static const std::vector<ChordQuality> q {
        { "maj",     "",        { 0, 4, 7 } },
        { "min",     "m",       { 0, 3, 7 } },
        { "dim",     "dim",     { 0, 3, 6 } },
        { "aug",     "aug",     { 0, 4, 8 } },
        { "sus2",    "sus2",    { 0, 2, 7 } },
        { "sus4",    "sus4",    { 0, 5, 7 } },
        { "5",       "power",   { 0, 7, 12 } },
        { "add9",    "add9",    { 0, 4, 7, 14 } },
        { "madd9",   "madd9",   { 0, 3, 7, 14 } },
        { "madd11",  "madd11",  { 0, 3, 7, 17 } },
        { "6",       "6",       { 0, 4, 7, 9 } },
        { "m6",      "m6",      { 0, 3, 7, 9 } },
        { "69",      "6-9",     { 0, 4, 7, 9, 14 } },
        { "m69",     "m6-9",    { 0, 3, 7, 9, 14 } },
        { "maj7",    "maj7",    { 0, 4, 7, 11 } },
        { "m7",      "m7",      { 0, 3, 7, 10 } },
        { "7",       "7",       { 0, 4, 7, 10 } },
        { "m7b5",    "m7b5",    { 0, 3, 6, 10 } },
        { "dim7",    "dim7",    { 0, 3, 6, 9 } },
        { "mmaj7",   "mMaj7",   { 0, 3, 7, 11 } },
        { "7sus4",   "7sus4",   { 0, 5, 7, 10 } },
        { "maj9",    "maj9",    { 0, 4, 7, 11, 14 } },
        { "m9",      "m9",      { 0, 3, 7, 10, 14 } },
        { "9",       "9",       { 0, 4, 7, 10, 14 } },
        { "9sus4",   "9sus4",   { 0, 5, 7, 10, 14 } },
        { "mmaj9",   "mMaj9",   { 0, 3, 7, 11, 14 } },
        { "m11",     "m11",     { 0, 3, 7, 10, 14, 17 } },
        { "maj7s11", "maj7#11", { 0, 4, 7, 11, 18 } },
        { "maj9s11", "maj9#11", { 0, 4, 7, 11, 14, 18 } },
        { "13",      "13",      { 0, 4, 7, 10, 14, 21 } },
        { "maj13",   "maj13",   { 0, 4, 7, 11, 14, 21 } },
        { "m13",     "m13",     { 0, 3, 7, 10, 14, 21 } },
        { "7s9",     "7#9",     { 0, 4, 7, 10, 15 } },
        { "7b9",     "7b9",     { 0, 4, 7, 10, 13 } },
        { "7s5",     "7#5",     { 0, 4, 8, 10 } },
        { "13b9",    "13b9",    { 0, 4, 10, 13, 21 } },
        { "quartal", "q4",      { 0, 5, 10, 15 } },
        { "quintal", "q5",      { 0, 7, 14, 21 } },
    };
    return q;
}

const ChordQuality* findQuality (const juce::String& id)
{
    for (auto& q : allQualities())
        if (q.id.equalsIgnoreCase (id))
            return &q;
    return nullptr;
}

static const std::vector<std::pair<Voicing, const char*>>& voicingTable()
{
    static const std::vector<std::pair<Voicing, const char*>> t {
        { Voicing::Close,      "close" },
        { Voicing::Inversion1, "inv1" },
        { Voicing::Inversion2, "inv2" },
        { Voicing::Drop2,      "drop2" },
        { Voicing::Open,       "open" },
        { Voicing::Rootless,   "rootless" },
        { Voicing::BassShell,  "shell" },
        { Voicing::Spread,     "spread" },
    };
    return t;
}

juce::String voicingId (Voicing v)
{
    for (auto& [voicing, name] : voicingTable())
        if (voicing == v)
            return name;
    return "close";
}

bool parseVoicing (const juce::String& id, Voicing& out)
{
    for (auto& [voicing, name] : voicingTable())
        if (id.equalsIgnoreCase (name)) { out = voicing; return true; }
    return false;
}

juce::StringArray allVoicingIds()
{
    juce::StringArray s;
    for (auto& [voicing, name] : voicingTable())
        s.add (name);
    return s;
}

static const std::vector<std::pair<const char*, std::vector<int>>>& scaleTable()
{
    static const std::vector<std::pair<const char*, std::vector<int>>> t {
        { "major",         { 0, 2, 4, 5, 7, 9, 11 } },
        { "minor",         { 0, 2, 3, 5, 7, 8, 10 } },
        { "dorian",        { 0, 2, 3, 5, 7, 9, 10 } },
        { "phrygian",      { 0, 1, 3, 5, 7, 8, 10 } },
        { "lydian",        { 0, 2, 4, 6, 7, 9, 11 } },
        { "mixolydian",    { 0, 2, 4, 5, 7, 9, 10 } },
        { "harmonicminor", { 0, 2, 3, 5, 7, 8, 11 } },
        { "melodicminor",  { 0, 2, 3, 5, 7, 9, 11 } },
    };
    return t;
}

bool getScale (const juce::String& name, std::vector<int>& out)
{
    auto n = name.removeCharacters (" _-").toLowerCase();
    for (auto& [id, notes] : scaleTable())
        if (n == id) { out = notes; return true; }
    return false;
}

juce::StringArray allScaleNames()
{
    juce::StringArray s;
    for (auto& [id, notes] : scaleTable())
        s.add (id);
    return s;
}

juce::String pitchClassName (int pc)
{
    static const char* names[] = { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };
    return names[((pc % 12) + 12) % 12];
}

int parsePitchClass (const juce::String& nameIn)
{
    auto name = nameIn.trim();
    if (name.isEmpty())
        return -1;

    static const int base[] = { 9, 11, 0, 2, 4, 5, 7 }; // A B C D E F G
    auto letter = juce::CharacterFunctions::toUpperCase (name[0]);
    if (letter < 'A' || letter > 'G')
        return -1;

    int pc = base[letter - 'A'];
    auto rest = name.substring (1).toLowerCase();

    if (rest == "#" || rest == "s" || rest == "sharp")  ++pc;
    else if (rest == "b" || rest == "flat")             --pc;
    else if (rest.isNotEmpty())                         return -1;

    return (pc + 12) % 12;
}

juce::String midiNoteName (int midi)
{
    return pitchClassName (midi) + juce::String (midi / 12 - 2);
}

int parseMidiNote (const juce::String& in)
{
    auto s = in.trim();
    if (s.isEmpty()) return -1;
    if (s.containsOnly ("0123456789"))
    {
        auto n = s.getIntValue();
        return juce::isPositiveAndBelow (n, 128) ? n : -1;
    }

    auto split = s.indexOfAnyOf ("-0123456789");
    if (split <= 0) return -1;
    auto pc = parsePitchClass (s.substring (0, split));
    auto octaveText = s.substring (split);
    if (pc < 0 || ! octaveText.trimCharactersAtStart ("-").containsOnly ("0123456789")) return -1;

    auto n = (octaveText.getIntValue() + 2) * 12 + pc;
    return juce::isPositiveAndBelow (n, 128) ? n : -1;
}

static std::vector<int> normalise (std::vector<int> notes)
{
    std::set<int> unique (notes.begin(), notes.end());
    notes.assign (unique.begin(), unique.end());

    if (notes.empty())
        return notes;

    while (notes.back() > 103 && notes.front() - 12 >= 21)
        for (auto& n : notes) n -= 12;

    while (notes.front() < 24)
        for (auto& n : notes) n += 12;

    return notes;
}

std::vector<int> buildVoicing (int root, const std::vector<int>& intervals, Voicing v)
{
    std::vector<int> close;
    for (auto i : intervals)
        close.push_back (root + i);
    std::sort (close.begin(), close.end());

    std::vector<int> upper; // chord without the root, for left-hand-bass voicings
    for (auto i : intervals)
        if (i % 12 != 0)
            upper.push_back (root + i);
    if (upper.empty())
        upper = close;

    std::vector<int> out;

    switch (v)
    {
        case Voicing::Close:
            out = close;
            break;

        case Voicing::Inversion1:
        case Voicing::Inversion2:
        {
            out = close;
            int turns = v == Voicing::Inversion1 ? 1 : 2;
            for (int t = 0; t < turns && out.size() > 2; ++t)
            {
                std::sort (out.begin(), out.end());
                out.front() += 12;
            }
            break;
        }

        case Voicing::Drop2:
            out = close;
            if (out.size() >= 4)
                out[out.size() - 2] -= 12;
            break;

        case Voicing::Open:
            out = close;
            for (size_t i = 1; i < out.size(); i += 2)
                out[i] += 12;
            break;

        case Voicing::Rootless:
            out = upper;
            out.push_back (root - 12);
            break;

        case Voicing::BassShell:
        {
            out.push_back (root - 12);
            for (auto i : intervals)
            {
                auto pc = i % 12;
                if (pc == 3 || pc == 4 || pc == 10 || pc == 11 || pc == 5 || pc == 2)
                    out.push_back (root + pc);
            }
            if (! intervals.empty() && intervals.back() > 12)
                out.push_back (root + intervals.back());
            if (out.size() < 3)
                out.push_back (root + 7);
            break;
        }

        case Voicing::Spread:
        {
            out.push_back (root - 12);
            out.push_back (root + 7 - 12 * (intervals.size() > 4 ? 1 : 0));
            for (size_t i = 0; i < upper.size(); ++i)
                out.push_back (upper[i] + (i % 2 == 1 ? 12 : 0));
            break;
        }
    }

    return normalise (out);
}

bool chordFitsScale (int rootPc, const std::vector<int>& intervals, int tonicPc, const std::vector<int>& scale)
{
    for (auto i : intervals)
    {
        auto rel = ((rootPc + i - tonicPc) % 12 + 12) % 12;
        if (std::find (scale.begin(), scale.end(), rel) == scale.end())
            return false;
    }
    return true;
}
}
