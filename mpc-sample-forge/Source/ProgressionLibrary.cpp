#include "ProgressionLibrary.h"
#include "Theory.h"

namespace forge
{
bool parseChordName (const juce::String& nameIn, NamedChord& out)
{
    auto name = nameIn.trim().replace ("five", "5");
    if (name.isEmpty()) return false;
    auto rootLen = (name.length() > 1 && (name[1] == '#' || name[1] == 'b')) ? 2 : 1;
    auto pc = parsePitchClass (name.substring (0, rootLen));
    if (pc < 0) return false;
    auto rest = name.substring (rootLen);
    out.rootPc = pc;
    if (rest.isEmpty())   out.quality = "maj";
    else if (rest == "m") out.quality = "min";
    else if (rest == "5") out.quality = "5";
    else return false;
    return true;
}

juce::String romanNumeralFor (const NamedChord& c, int tonicPc)
{
    static const char* upper[] = { "I", "bII", "II", "bIII", "III", "IV", "#IV", "V", "bVI", "VI", "bVII", "VII" };
    auto interval = ((c.rootPc - tonicPc) % 12 + 12) % 12;
    juce::String r (upper[interval]);
    if (c.quality == "min") r = r.toLowerCase().replace ("#iv", "#iv").replace ("bii", "bii");
    if (c.quality == "5") r += "5";
    return r;
}

namespace
{
    NamedProgression prog (const char* text)
    {
        NamedProgression p;
        p.text = text;
        for (auto& token : juce::StringArray::fromTokens (text, "-", ""))
        {
            NamedChord c;
            if (parseChordName (token, c)) p.chords.push_back (c);
            else jassertfalse;
        }
        return p;
    }

    ProgressionSet set (int number, const char* key, const char* main, const char* a1, const char* a2, const char* tip)
    {
        ProgressionSet s;
        s.number = number;
        s.keyName = key;
        s.main = prog (main);
        s.alt1 = prog (a1);
        s.alt2 = prog (a2);
        s.tip = tip;
        auto keyText = juce::String (key);
        NamedChord tonic;
        parseChordName (keyText.upToFirstOccurrenceOf (" ", false, false), tonic);
        s.tonicPc = tonic.rootPc;
        s.minor = keyText.containsIgnoreCase ("minor");
        return s;
    }
}

const std::vector<ProgressionSet>& progressionLibrary (const juce::String& id)
{
    // "20+ Trap Chord Progressions For Killer Beats": 20 main progressions, each with two
    // alternates that extend it into a full song. Chord names as written in the article;
    // tips condensed. Roman numerals are worked out from the chords (see romanNumeralFor).
    static const std::vector<ProgressionSet> trap {
        set (1, "C minor", "Cm-Gm-G#-Fm", "Fm-Gm-G#-Cm", "Cm-Eb-D-Gm",
             "Moody and ambiguous (harmonic-minor colour); the chromatic move in the middle adds tension - great under angry, punchy verses. Try it on an 808 with light strings and a solid beat."),
        set (2, "D major", "D-E-B", "Cm-B-Em", "Cm-Em-G",
             "Upbeat and anthemic, bigger-than-life even under tight flows. Put it on strings or a bright synth / organ with a droning bass underneath."),
        set (3, "E minor", "Em-Ebm-B-Em", "Gm-Gbm-Gm-Gbm", "Bm-Cm-Bm-Cm",
             "Spooky: minor with chromatic movement. Works sparse - start with guitar, bass and light strings, bring the drums in later."),
        set (4, "G minor", "Gm-Em-C#m", "Gm-E-Eb-Gbm", "Bm-Cm-Bm-Cm",
             "Night-time, dark-streets mood: mysterious but still epic. 808 bass with an arpeggiated synth and sustained strings."),
        set (5, "C# minor", "C#m-G#m-C#m-G#m", "A-F#m-A-F#m", "A-F-A-F",
             "Simple but very versatile - dark and moody or tough and upbeat depending on the instruments. Guitar or strings with an arpeggiated bassline."),
        set (6, "F minor", "Fm-Eb-Cm", "Fm-Gm-Cm-Eb", "Bb-Cm-G#-Eb",
             "Chilled and melancholic - for lyrics about looking back or regret. Organ or strings over a laid-back half-time beat that leaves room for the vocal."),
        set (7, "A minor", "Am-Em-Am-Em", "Fm-Em-Fm-Em", "Dm-Am-C-B",
             "Moody with a Spanish flavour. Hold each chord for four beats and keep the arrangement sparse: piano or an arpeggiated synth with a moving bassline."),
        set (8, "G minor", "Gm-F-Cm-D", "Em-C-Em-D", "Eb-Em-Eb-Em",
             "Soulful: minor, but with a positive, quirky lift. Arpeggiated synth with delay over a long, held bassline."),
        set (9, "E minor", "Em-Bm-Am-Eb", "Cm-G#-Cm-Eb", "Em-Eb-Em-Eb",
             "Anthemic or dark depending on the sound; wants faster drums and a tight bassline. Guitar, or moody / ambient synths. (The article lists G minor, but these chords sit in E minor.)"),
        set (10, "D minor", "Dm-C-Dm-C", "Bb-C-Bb-C", "Am-C-Am-C",
             "Jazz meets hip hop: accent it with stabs alternating between the two chords, old-school style. Works laid back or upbeat with trap synths, bass and drums."),
        set (11, "F# minor", "F#m-D-C#-D", "Bm-G-Bm-G", "E-D-C#-D",
             "Fast: about 170 BPM, one chord per bar. Laid back: about 120 BPM, holding one chord for 3 beats and the next for 5. Great for arpeggios on soft synths."),
        set (12, "E minor", "Em-Ebm-B-Em-G-C", "D-C#-Gm", "Bm-C-Am",
             "Tension and confidence, with big leaps at the end. Hold chords 1 and 4 for four beats and the rest for two. A distorted 808 on the roots; fits tight bars or melodic flows."),
        set (13, "F# minor", "F#m-A-D", "F#m-E-D", "C#m-D-Bm",
             "Anthemic: bold major moves while staying rooted in minor. A droning bass anchors it; best on the upbeat side with some rave-style sounds."),
        set (14, "F minor", "Fm-C#-Ab-Em-Fm", "Fm-C#-C-C#-Eb", "Fm-C#-Eb-C",
             "Unusual leaps and a chromatic ending. Suits heavier, industrial or rave-leaning trap - or slow it into something ambient."),
        set (15, "E minor", "Em-B-Em-F#", "G#m-F#-Em-F#", "Gm-Em-Gm-F#",
             "Simple and one of the most flexible. 90-140 BPM, 1-2 bars per chord; leaves room for busy hi-hats, gliding synths and the vocal."),
        set (16, "G major", "G5-F#5-Eb5", "Em-Ebm-Em-Ebm", "Dm-Bb-Dm-Bb",
             "Power chords make it bold; the chromatic slide adds darkness. Hold 2 + 2 + 4 beats (or 4 + 2 + 2). Orchestral trap: strings or ambient brass."),
        set (17, "E minor", "E5-B5-G5-F#5", "A5-G5-F#5-G5", "E5-C5-B5-G5",
             "Bold yet eerie power chords. Best on classic trap synths anchored by an 808."),
        set (18, "F# minor", "F#m-Em-A", "C#m-Em-A", "D-E-B",
             "Modern hip-hop feel that sits on almost any trap beat. Brass with an 808 on the roots for the classic sound."),
        set (19, "Bb major", "Bb-A-A-Bb", "Bb-Dm-A-B", "Bb-C-A-Bb",
             "Chill trap: rocking between semitones feels calm. Guitar or strings - guitar gives a lovely lo-fi sound."),
        set (20, "C minor", "C5-D5-Eb5-F5", "C5-G5-G#5-Bb5", "G5-G#5-G5-Bb5",
             "Climbs the C minor scale in power chords: perfect for a slow build from sparse and eerie to huge."),
    };
    static const std::vector<ProgressionSet> none;
    return id.equalsIgnoreCase ("trap") ? trap : none;
}
}
