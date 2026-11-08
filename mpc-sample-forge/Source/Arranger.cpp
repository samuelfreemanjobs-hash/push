#include "Arranger.h"
#include "BuiltInSynth.h"
#include "OfflineRenderer.h"
#include <map>
#include <set>

namespace forge
{
//==============================================================================
// Genre presets (vintage R&B first). Every part has a recipe, so each genre covers keys,
// bass, pad / strings, lead and drums.

const std::vector<SongPreset>& songPresets()
{
    static const std::vector<SongPreset> presets = []
    {
        std::vector<SongPreset> p;
        auto add = [&p] (const char* id, const char* name, const char* desc, const char* scale, const char* keys,
                         double t0, double t1, double swing, std::initializer_list<const char*> progs,
                         const char* keysR, const char* bassR, const char* padR, const char* leadR,
                         const char* keysPat, const char* bassPat, const char* drumPat, int density)
        {
            SongPreset s;
            s.id = id; s.name = name; s.description = desc; s.scale = scale;
            s.keys = juce::StringArray::fromTokens (keys, " ", "");
            s.tempoLow = t0; s.tempoHigh = t1; s.swing = swing;
            for (auto* x : progs) s.progressions.add (x);
            s.keysRecipe = keysR; s.bassRecipe = bassR; s.padRecipe = padR; s.leadRecipe = leadR;
            s.keysPattern = keysPat; s.bassPattern = bassPat; s.drumPattern = drumPat; s.leadDensity = density;
            p.push_back (s);
        };

        add ("philly-soul", "70s Philly Soul",
             "Sweet 70s soul (Philadelphia International, Delfonics, Stylistics): Wurlitzer comping, fingered soul bass, lush strings, a horn line, straight-8ths groove.",
             "major", "C F Bb Eb G", 90, 104, 0.1,
             { "I:maj7 vi:m7 ii:m7 V:7sus4", "IV:maj7 iii:m7 ii:m7 I:maj7", "I:maj7 I:7 IV:maj7 iv:m6", "ii:m9 V:13 I:maj9 vi:m9",
               "IV:maj7 V:9sus4 iii:m7 vi:m7", "I:maj9 III:7 vi:m7 II:9" },
             "wurli-keys", "soul-bass", "philly-strings", "soul-horns", "pushes", "soul", "soul70", 1);
        add ("stax-soul", "Memphis / Stax Soul",
             "Gritty 60s-70s Memphis soul (Stax, Hi Records): organ stabs on 2 and 4, a busy bass, horn section lines, strings underneath.",
             "mixolydian", "C F G Bb D", 96, 112, 0.15,
             { "I:7 IV:7 I:7 V:7", "I:9 bVII:maj7 IV:9 I:9", "I:7 vi:m7 ii:m7 V:7", "IV:9 I:7 V:7s9 I:7" },
             "organ-stabs", "soul-bass", "philly-strings", "soul-horns", "stabs", "soul", "soul70", 1);
        add ("quiet-storm", "80s Quiet Storm",
             "Slow-jam R&B (Luther, Anita Baker, Freddie Jackson): DX-style electric piano held chords, a smooth synth bass, warm pad, sax answers, half-time drums.",
             "major", "Db Eb Ab Bb F", 64, 78, 0.0,
             { "ii:m9 V:9sus4 I:maj9 vi:m9", "IV:maj9 V:9sus4 iii:m7 vi:m9", "I:maj9 iii:m7 IV:maj9 V:9sus4", "vi:m9 ii:m9 V:13 I:maj9",
               "IV:maj7 iii:m7 ii:m9 I:maj9" },
             "rnb-80s-chords", "quiet-storm-bass", "smooth-jazz-pad", "smooth-jazz-sax", "whole", "quiet", "quietstorm", 0);
        add ("sade", "Sade / Minor Quiet Storm",
             "Moody minor quiet storm (Sade, Maxwell): Rhodes chords, a dark warm pad, a round bass and a soft sax, slow and sparse.",
             "dorian", "A D E F# C#", 70, 86, 0.05,
             { "i:m9 bVII:maj7 i:m9 bVII:maj7", "i:m9 iv:m11 bVII:maj9 bIII:maj7", "i:m11 bVI:maj7 bVII:6 i:m9", "iv:m9 v:m7 i:m9 i:m9" },
             "wiz-ep", "quiet-storm-bass", "sade-pad", "smooth-jazz-sax", "whole", "quiet", "quietstorm", 0);
        add ("new-jack", "New Jack Swing",
             "Late-80s / early-90s New Jack Swing (Teddy Riley, Bobby Brown, Guy): swung 16ths, synth stabs, a bouncy synth bass, choir pad, synth lead.",
             "minor", "C D E F A", 100, 112, 0.55,
             { "i:m7 iv:m7 bVII:9 bIII:maj7", "i:m9 bVI:maj7 bVII:7 i:m9", "iv:m9 v:m7 i:m9 i:m9", "i:m7 bVII:9 bVI:maj7 V:7s9" },
             "njs-stabs", "njs-bass", "choir", "lead-synth", "njs", "njs", "njs", 2);
        add ("neo-soul", "90s Neo-Soul",
             "Neo-soul (D'Angelo, Erykah Badu, Soulquarians): laid-back Rhodes, a deep lazy bass, a warm pad, a flute line, swung behind-the-beat drums.",
             "dorian", "E F G A C", 80, 92, 0.55,
             { "i:m9 iv:m9 bVII:9 bIII:maj9", "i:m11 bVI:maj7 bVII:13 i:m9", "i:m9 IV:9 i:m9 IV:9", "bIII:maj7 bVI:maj7 ii:m7 V:7s9",
               "iv:m9 bVII:13 bIII:maj9 bVI:maj7" },
             "neo-soul", "neo-soul-bass", "smooth-jazz-pad", "mellotron-flute", "pushes", "neo", "neosoul", 1);
        add ("gospel-rnb", "90s Gospel R&B",
             "90s gospel-flavoured R&B (Jodeci, Boyz II Men, K-Ci & JoJo): rich Rhodes voicings with gospel rhythm, a choir pad, organ lead, a soul bass.",
             "major", "Ab Bb Db Eb F", 68, 84, 0.2,
             { "IV:maj9 V:9sus4 iii:m7 vi:m9", "ii:m9 V:13 I:maj9 IV:maj9", "I:maj9 III:7s9 vi:m9 V:9sus4", "IV:maj7 iv:m6 iii:m7 vi:m9" },
             "gospel-rnb-90s", "bassline", "choir", "zaytoven-organ", "gospel", "soul", "neosoul", 1);
        add ("smooth-jazz", "Smooth Jazz",
             "Smooth jazz / jazz-soul (Bob James, Grover Washington Jr.): Rhodes comping, a walking upright bass, a soft pad, alto sax melody, light swing.",
             "major", "F Bb Eb G C", 84, 98, 0.35,
             { "ii:m9 V:13 I:maj9 vi:m9", "IV:maj7s11 iii:m7 ii:m9 V:9sus4", "I:maj9 vi:m9 ii:m9 V:13", "iii:m7 VI:7s9 ii:m9 V:13" },
             "smooth-jazz-chords", "upright-bass", "smooth-jazz-pad", "smooth-jazz-sax", "pushes", "walk", "lightswing", 1);
        add ("g-funk", "G-Funk",
             "West Coast G-funk (Dr. Dre, Warren G, DJ Quik): Rhodes stabs, a Moog bass, strings, the whistle lead, a laid-back groove.",
             "dorian", "A E D G C", 88, 98, 0.25,
             { "i:m7 IV:9 i:m7 IV:9", "i:m9 bVII:9 bVI:maj7 V:7s9", "i:m7 iv:m7 i:m7 V:7s9", "i:m9 bIII:maj7 IV:9 i:m9" },
             "gfunk-ep", "moog-bass", "philly-strings", "gfunk-lead", "stabs", "gfunk", "gfunk", 1);
        add ("screw-rnb", "DJ Screw Chopped R&B",
             "Houston chopped & screwed R&B: slow, heavy, dreamy chords, an 808 bass, a dark choir, a talkbox line, half-time drums.",
             "minor", "C D E F G", 60, 70, 0.0,
             { "i:m9 bVI:maj7 iv:m9 V:7s9", "i:m7 bVII:9 bVI:maj7 bVII:9", "i:m9 iv:m9 bVII:9 bIII:maj7" },
             "screw-rnb", "808-bass", "screwed-choir", "talkbox-lead", "whole", "808", "screw", 0);
        add ("luxury-soul", "Luxury Soul (Maybach)",
             "Luxury soul / Maybach Music (Rick Ross, J.U.S.T.I.C.E. League): Rhodes, 808 bass, lush jazz strings, jazz brass lines, trap-soul drums.",
             "dorian", "C# D# F G A", 68, 80, 0.0,
             { "i:m9 bVI:maj9 bIII:maj9 bVII:69", "i:m11 iv:m9 bVII:13 bIII:maj9", "bVI:maj9 bVII:9sus4 i:m9 i:m9" },
             "wiz-ep", "808-bass", "ross-lux-strings", "ross-lux-brass", "pushes", "808", "trapsoul", 0);
        return p;
    }();
    return presets;
}

const SongPreset* findSongPreset (const juce::String& id)
{
    for (auto& p : songPresets()) if (p.id.equalsIgnoreCase (id) || p.name.equalsIgnoreCase (id)) return &p;
    return nullptr;
}

//==============================================================================
std::vector<int> ChordBlock::intervals() const
{
    if (auto* q = findQuality (quality)) return q->intervals;
    return { 0, 4, 7 };
}

juce::String ChordBlock::name() const
{
    auto* q = findQuality (quality);
    auto suffix = q != nullptr ? q->suffix : quality;
    if (suffix == "power") suffix = "5";
    return pitchClassName (rootPc) + suffix;
}

juce::String ChordBlock::roman (int tonicPc) const
{
    static const char* names[] = { "I", "bII", "II", "bIII", "III", "IV", "#IV", "V", "bVI", "VI", "bVII", "VII" };
    auto iv = intervals();
    const bool minor = iv.size() > 1 && iv[1] == 3;
    juce::String r = names[((rootPc - tonicPc) % 12 + 12) % 12];
    if (minor) r = r.toLowerCase();
    auto* q = findQuality (quality);
    auto suffix = q != nullptr ? q->suffix : quality;
    if (minor && suffix.startsWith ("m") && ! suffix.startsWith ("maj")) suffix = suffix.substring (1);
    return r + suffix;
}

double Arrangement::totalBeats() const
{
    double t = 0;
    for (auto& b : blocks) t += b.beats;
    return t;
}

juce::String Arrangement::progressionText() const
{
    juce::StringArray t;
    for (auto& b : blocks) t.add (b.name() + (std::abs (b.beats - 4.0) > 1e-6 ? "*" + juce::String (b.beats) : juce::String()));
    return t.joinIntoString (" ");
}

//==============================================================================
// Reading progressions

static bool parseRoman (const juce::String& textIn, int& offset, int& used, bool& lower)
{
    auto text = textIn;
    int accidental = 0, i = 0;
    while (i < text.length() && (text[i] == 'b' || text[i] == '#'))
    {
        // a lone "b" before the numeral is a flat; "bVII"
        accidental += text[i] == '#' ? 1 : -1;
        ++i;
    }
    static const std::pair<const char*, int> numerals[] = { { "VII", 11 }, { "VI", 9 }, { "IV", 5 }, { "V", 7 }, { "III", 4 }, { "II", 2 }, { "I", 0 } };
    auto rest = text.substring (i);
    for (auto& [n, semis] : numerals)
        if (rest.toUpperCase().startsWith (n))
        {
            lower = juce::CharacterFunctions::isLowerCase (rest[0]);
            offset = ((semis + accidental) % 12 + 12) % 12;
            used = i + (int) strlen (n);
            return true;
        }
    return false;
}

bool parseProgression (const juce::String& text, int tonicPc, std::vector<ChordBlock>& out, juce::String& error)
{
    out.clear();
    for (auto tokenIn : juce::StringArray::fromTokens (text, " ,|-\t\n", ""))
    {
        auto token = tokenIn.trim();
        if (token.isEmpty()) continue;
        ChordBlock b;
        if (token.contains ("*")) { b.beats = juce::jlimit (0.5, 32.0, token.fromLastOccurrenceOf ("*", false, false).getDoubleValue()); token = token.upToLastOccurrenceOf ("*", false, false); }

        int offset = 0, used = 0;
        bool lower = false;
        if (parseRoman (token, offset, used, lower))
        {
            b.rootPc = (tonicPc + offset) % 12;
            auto q = token.substring (used).trimCharactersAtStart (":");
            if (q.isEmpty()) b.quality = lower ? "min" : "maj";
            else
            {
                b.quality = {};
                if (findQuality (q) != nullptr) b.quality = q;
                else
                    for (auto& cq : allQualities())
                        if (cq.suffix == q || (lower && cq.suffix == "m" + q)) { b.quality = cq.id; break; }
                if (b.quality.isEmpty()) { error = "unknown chord type '" + q + "' in " + tokenIn; return false; }
            }
        }
        else
        {
            PaletteChord pc;
            if (! PaletteChord::fromString (token, pc) || pc.isNote()) { error = "can't read '" + tokenIn + "' (e.g. ii:m9  V:13  Imaj9  or  Dm9 G13 Cmaj9)"; return false; }
            b.rootPc = pc.rootPc;
            b.quality = pc.quality;
        }
        out.push_back (b);
    }
    if (out.empty()) { error = "no chords"; return false; }
    return true;
}

std::vector<ChordBlock> generateProgression (const SongPreset& preset, int tonicPc, int bars, juce::int64 seed)
{
    juce::Random rng (seed);
    auto pick = [&] (int avoid) { int k; int guard = 0; do { k = rng.nextInt (preset.progressions.size()); } while (k == avoid && preset.progressions.size() > 1 && ++guard < 20); return k; };
    auto blocksOf = [&] (int k)
    {
        std::vector<ChordBlock> b;
        juce::String err;
        parseProgression (preset.progressions[k], tonicPc, b, err);
        return b;
    };
    // a turnaround on the last chord: V13 in major, bVII9 / V7#9 in minor
    auto turnaround = [&] (std::vector<ChordBlock> b)
    {
        if (b.empty()) return b;
        juce::String t = preset.scale == "major" || preset.scale == "mixolydian" ? (rng.nextBool() ? "V:13" : "V:9sus4")
                                                                                  : (rng.nextBool() ? "bVII:9" : "V:7s9");
        std::vector<ChordBlock> last;
        juce::String err;
        if (parseProgression (t, tonicPc, last, err)) b.back() = last.front();
        return b;
    };
    const int a = pick (-1), bIndex = pick (a);
    auto A = blocksOf (a), B = blocksOf (bIndex);
    std::vector<ChordBlock> out;
    auto append = [&out] (const std::vector<ChordBlock>& x) { out.insert (out.end(), x.begin(), x.end()); };
    if (bars <= 4) append (A);
    else if (bars <= 8) { append (A); append (rng.nextBool() ? turnaround (A) : turnaround (B)); }
    else { append (A); append (B); append (A); append (turnaround (B)); }
    return out;
}

//==============================================================================
// Parts

namespace
{
    std::vector<int> scaleOf (const juce::String& name)
    {
        std::vector<int> s;
        if (! getScale (name, s)) getScale ("major", s);
        return s;
    }

    /** The voicing of `intervals` on rootPc, within [low, high], that moves least from `previous`. */
    std::vector<int> voiceLed (const std::vector<int>& previous, int rootPc, std::vector<int> intervals, int low, int high)
    {
        std::vector<int> best;
        double bestCost = 1e9;
        for (int base = low - 12; base <= high; ++base)
        {
            if (((base % 12) + 12) % 12 != rootPc) continue;
            for (int inv = 0; inv < (int) intervals.size(); ++inv)
            {
                auto notes = invertChord (base, intervals, inv);
                if (notes.front() < low || notes.back() > high + 7) continue;
                double cost = 0;
                if (previous.empty()) cost = std::abs ((notes.front() + notes.back()) * 0.5 - (low + high) * 0.5);
                else
                    for (auto n : notes)
                    {
                        int d = 99;
                        for (auto p : previous) d = std::min (d, std::abs (n - p));
                        cost += d;
                    }
                cost += 8.0 * lowIntervalViolations (notes);
                if (cost < bestCost) { bestCost = cost; best = notes; }
            }
        }
        if (best.empty()) best = invertChord (rootPc + 12 * (low / 12), intervals, 0);
        return best;
    }

    int nearestWithPc (int pc, int target, int low, int high)
    {
        int best = -1;
        for (int n = low; n <= high; ++n)
            if (n % 12 == pc && (best < 0 || std::abs (n - target) < std::abs (best - target))) best = n;
        return best >= 0 ? best : juce::jlimit (low, high, pc + 12 * (target / 12));
    }

    struct Hit { double start, length; float vel; };

    std::vector<Hit> keysPattern (const juce::String& id)
    {
        if (id == "whole")  return { { 0, 3.9, 1.0f } };
        if (id == "stabs")  return { { 0, 0.9, 1.0f }, { 1.5, 0.4, 0.8f }, { 3.0, 0.4, 0.85f } };
        if (id == "gospel") return { { 0, 1.0, 1.0f }, { 1.75, 0.25, 0.7f }, { 2.0, 0.5, 0.85f }, { 2.75, 0.25, 0.7f }, { 3.0, 1.0, 0.9f } };
        if (id == "njs")    return { { 0, 0.25, 1.0f }, { 0.75, 0.25, 0.8f }, { 1.5, 0.25, 0.85f }, { 2.5, 0.25, 0.8f }, { 3.0, 0.5, 0.9f } };
        return { { 0, 1.5, 1.0f }, { 2.5, 1.2, 0.85f } };   // "pushes"
    }

    /** Bass hits: degree 'R' root, '5' fifth, '8' octave, '3' third, '7' seventh, 'A' approach to the next root. */
    struct BassHit { double start, length; char degree; float vel; };
    std::vector<BassHit> bassPattern (const juce::String& id)
    {
        if (id == "quiet") return { { 0, 2.75, 'R', 1.0f }, { 3, 0.5, '8', 0.8f }, { 3.5, 0.5, 'A', 0.75f } };
        if (id == "walk")  return { { 0, 0.95, 'R', 1.0f }, { 1, 0.95, '3', 0.85f }, { 2, 0.95, '5', 0.9f }, { 3, 0.95, 'A', 0.85f } };
        if (id == "gfunk") return { { 0, 1.5, 'R', 1.0f }, { 1.75, 0.25, 'R', 0.7f }, { 2.5, 0.5, '8', 0.85f }, { 3, 0.5, '5', 0.8f }, { 3.5, 0.5, 'A', 0.8f } };
        if (id == "808")   return { { 0, 2.4, 'R', 1.0f }, { 2.75, 0.75, 'R', 0.85f }, { 3.5, 0.5, '5', 0.7f } };
        if (id == "njs")   return { { 0, 0.4, 'R', 1.0f }, { 0.75, 0.25, 'R', 0.75f }, { 1.5, 0.4, 'R', 0.85f }, { 2.25, 0.25, '8', 0.8f },
                                    { 2.5, 0.4, 'R', 0.85f }, { 3.25, 0.25, '5', 0.75f }, { 3.5, 0.4, 'A', 0.8f } };
        if (id == "neo")   return { { 0, 1.5, 'R', 1.0f }, { 1.75, 0.25, '5', 0.6f }, { 2.5, 0.75, 'R', 0.85f }, { 3.25, 0.25, '7', 0.65f }, { 3.5, 0.5, 'A', 0.75f } };
        return { { 0, 1.25, 'R', 1.0f }, { 1.5, 0.5, 'R', 0.75f }, { 2, 0.75, '5', 0.9f }, { 3, 0.5, '8', 0.8f }, { 3.5, 0.5, 'A', 0.75f } }; // "soul"
    }

    /** 16 steps per bar. x = hit, X = accent, g = ghost. */
    struct DrumBar { const char *kick, *snare, *hat, *open, *rim, *clap; };
    DrumBar drumPattern (const juce::String& id)
    {
        if (id == "quietstorm") return { "x.......x.x.....", "........X.......", "x.x.x.x.x.x.x.x.", "................", "....g.......g...", "................" };
        if (id == "neosoul")    return { "x......x..x.....", "....X.......X...", "x.xgx.x.x.xgx.x.", "..............x.", "................", "................" };
        if (id == "njs")        return { "x..x..x...x..x..", "....X.......X...", "xxxxxxxxxxxxxxxx", "................", "................", "....x.......x..." };
        if (id == "gfunk")      return { "x......x.x......", "....X.......X...", "x.x.x.x.x.x.x.x.", "......x.........", "................", "....x.......x..." };
        if (id == "screw")      return { "x.........x.....", "........X.......", "x...x...x...x...", "................", "................", "........x......." };
        if (id == "trapsoul")   return { "x......x..x.....", "........X.......", "x.x.x.x.xxx.x.x.", "................", "................", "........x......." };
        if (id == "lightswing") return { "x.......x.......", "....g.......g...", "x..xx..xx..xx..x", "................", "....x.......x...", "................" };
        return { "x.....x...x.....", "....X.......X...", "x.x.x.x.x.x.x.x.", "..............x.", "................", "................" }; // "soul70"
    }

    double swingShift (double beat, double swing)
    {
        const double frac = beat - std::floor (beat);
        if (std::abs (frac - 0.5) < 1e-6) return swing / 6.0;                                       // 8ths
        if (std::abs (frac - 0.25) < 1e-6 || std::abs (frac - 0.75) < 1e-6) return swing / 12.0;    // 16ths
        return 0;
    }

    const Style& recipeOr (const juce::String& id)
    {
        if (auto* s = findBuiltInStyle (id)) return *s;
        return builtInStyles().getReference (0);
    }
}

void generateParts (Arrangement& arr, const SongPreset& preset)
{
    // keep the user's choices of recipe / on-off per part
    std::map<juce::String, Part> previous;
    for (auto& p : arr.parts) previous[p.id] = p;
    auto makePart = [&] (const char* id, const char* name, const juce::String& recipe, int channel)
    {
        Part p;
        p.id = id; p.name = name; p.channel = channel;
        p.recipeId = previous.count (id) ? previous[id].recipeId : recipe;
        p.enabled = previous.count (id) ? previous[id].enabled : true;
        if (previous.count (id)) { p.pluginPath = previous[id].pluginPath; p.pluginName = previous[id].pluginName; }
        return p;
    };
    Part keys = makePart ("keys", "Keys", preset.keysRecipe, 1);
    Part bass = makePart ("bass", "Bass", preset.bassRecipe, 2);
    Part pad  = makePart ("pad", "Pad / Strings", preset.padRecipe, 3);
    Part lead = makePart ("lead", "Lead", preset.leadRecipe, 4);
    Part drums = makePart ("drums", "Drums", "", 10);

    juce::Random rng (arr.seed * 7919 + 13);
    const auto scale = scaleOf (arr.scale);
    auto inScale = [&] (int note) { return std::find (scale.begin(), scale.end(), ((note - arr.tonic) % 12 + 12) % 12) != scale.end(); };

    const auto& keysR = recipeOr (keys.recipeId);
    const auto& bassR = recipeOr (bass.recipeId);
    const auto& padR  = recipeOr (pad.recipeId);
    const auto& leadR = recipeOr (lead.recipeId);
    auto vel = [] (const Style& s, float scale) { return juce::jlimit (1, 127, juce::roundToInt (((s.velocityLow + s.velocityHigh) * 0.5) * scale)); };

    std::vector<int> prevKeys, prevPad;
    int prevBass = juce::jlimit (28, 60, bassR.keyLow + 7);
    double t = 0;
    for (size_t i = 0; i < arr.blocks.size(); ++i)
    {
        const auto& b = arr.blocks[i];
        const auto& next = arr.blocks[(i + 1) % arr.blocks.size()];
        auto iv = b.intervals();

        // ---- keys: the full chord (fifth dropped from big chords), voice-led in the recipe's register
        auto keysIv = iv;
        if (keysIv.size() >= 5) keysIv.erase (std::remove (keysIv.begin(), keysIv.end(), 7), keysIv.end());
        auto chord = voiceLed (prevKeys, b.rootPc, keysIv, keysR.chordRootLow, keysR.chordRootHigh + 12);
        prevKeys = chord;
        for (double bar = 0; bar < b.beats - 1e-6; bar += 4)
            for (auto& h : keysPattern (preset.keysPattern))
            {
                if (bar + h.start >= b.beats - 1e-6) continue;
                auto len = std::min (h.length, b.beats - bar - h.start);
                for (size_t k = 0; k < chord.size(); ++k)
                    keys.notes.push_back ({ chord[k], t + bar + h.start, len, vel (keysR, h.vel * (k + 1 == chord.size() ? 1.05f : 1.0f)) });
            }

        // ---- pad: the upper structure (no root), long and soft, a little higher
        std::vector<int> padIv;
        for (auto x : iv) if (x % 12 != 0) padIv.push_back (x);
        if (padIv.size() > 3) padIv.erase (std::remove (padIv.begin(), padIv.end(), 7), padIv.end());
        if (padIv.empty()) padIv = { 4, 7 };
        {
            // voice the upper notes relative to the chord root, then lead them smoothly
            std::vector<int> best;
            double bestCost = 1e9;
            const int low = juce::jmax (55, padR.keyLow), high = low + 19;
            for (int base = low - 24; base <= high; ++base)
            {
                if (((base % 12) + 12) % 12 != b.rootPc) continue;
                for (int rot = 0; rot < (int) padIv.size(); ++rot)
                {
                    std::vector<int> notes;
                    for (size_t k = 0; k < padIv.size(); ++k) notes.push_back (base + padIv[k] + (k < (size_t) rot ? 12 : 0));
                    std::sort (notes.begin(), notes.end());
                    if (notes.front() < low || notes.back() > high) continue;
                    double cost = 0;
                    for (auto n : notes) { int d = 99; for (auto p : prevPad) d = std::min (d, std::abs (n - p)); cost += prevPad.empty() ? 0 : d; }
                    if (prevPad.empty()) cost = std::abs (notes.front() - low - 5);
                    if (cost < bestCost) { bestCost = cost; best = notes; }
                }
            }
            if (! best.empty())
            {
                prevPad = best;
                for (auto n : best) pad.notes.push_back ({ n, t, b.beats - 0.05, vel (padR, 0.8f) });
            }
        }

        // ---- bass: the preset's pattern on this chord, approaching the next root
        const int bassLow = bassR.keyLow, bassHigh = juce::jmax (bassR.keyLow + 17, bassR.keyHigh - 7);
        const int root = nearestWithPc (b.rootPc, prevBass, bassLow, bassLow + 11);
        const int nextRoot = nearestWithPc (next.rootPc, root, bassLow, bassLow + 11);
        prevBass = root;
        auto degreeNote = [&] (char d)
        {
            switch (d)
            {
                case '5': return root + 7;
                case '8': return root + 12 <= bassHigh ? root + 12 : root;
                case '3': return root + (std::find (iv.begin(), iv.end(), 3) != iv.end() ? 3 : 4);
                case '7': return root + (std::find (iv.begin(), iv.end(), 11) != iv.end() ? 11 : 10);
                case 'A':
                {
                    // a step into the next root: from below by a half step, or from the scale step above
                    int a = nextRoot - 1;
                    if (! inScale (a) && inScale (nextRoot + 2)) a = nextRoot + 2;
                    return juce::jlimit (bassLow, bassHigh, a);
                }
                default: return root;
            }
        };
        for (double bar = 0; bar < b.beats - 1e-6; bar += 4)
            for (auto& h : bassPattern (preset.bassPattern))
            {
                if (bar + h.start >= b.beats - 1e-6) continue;
                if (h.degree == 'A' && bar + 4 < b.beats - 1e-6) continue;   // only approach at the end of the chord
                auto len = std::min (h.length, b.beats - bar - h.start);
                bass.notes.push_back ({ degreeNote (h.degree), t + bar + h.start, len, vel (bassR, h.vel) });
            }

        // ---- drums: one bar of the groove for every 4 beats
        auto dp = drumPattern (preset.drumPattern);
        for (double bar = 0; bar < b.beats - 1e-6; bar += 4)
        {
            const std::pair<const char*, int> lanes[] = { { dp.kick, 36 }, { dp.snare, 38 }, { dp.hat, 42 }, { dp.open, 46 }, { dp.rim, 37 }, { dp.clap, 39 } };
            for (auto& [pattern, note] : lanes)
                for (int step = 0; step < 16; ++step)
                {
                    const double at = bar + step * 0.25;
                    if (at >= b.beats - 1e-6) break;
                    const char c = pattern[step];
                    if (c == '.') continue;
                    int v = c == 'X' ? 120 : c == 'g' ? 50 : (note == 42 ? (step % 4 == 0 ? 92 : 72) : 100);
                    drums.notes.push_back ({ note, t + at, 0.2, v });
                }
        }
        t += b.beats;
    }

    // ---- lead: a motif from chord tones on strong beats and scale steps between, phrased in 2-bar
    // questions and answers, in the lead recipe's register
    {
        // above the keys: start where the keys' top notes end, an octave and a half of room
        int keysTop = 60;
        for (auto& n : keys.notes) keysTop = std::max (keysTop, n.note);
        const int low = juce::jlimit (55, 84, std::max (leadR.keyLow, keysTop - 2));
        const int high = juce::jmin (std::max (leadR.keyHigh, low + 12), low + 19);
        static const std::vector<std::vector<std::pair<double, double>>> motifs[] = {
            { { { 0, 1.5 }, { 2, 0.5 }, { 2.5, 1.5 } }, { { 0.5, 1.0 }, { 1.5, 0.5 }, { 2, 2.0 } }, { { 0, 3.0 } } },                          // sparse
            { { { 0, 0.5 }, { 0.5, 0.5 }, { 1, 1.0 }, { 2.5, 0.5 }, { 3, 1.0 } }, { { 0.5, 0.5 }, { 1, 0.5 }, { 1.5, 1.0 }, { 3, 1.0 } },
              { { 0, 1.0 }, { 1.5, 0.5 }, { 2, 1.5 } } },                                                                                     // medium
            { { { 0, 0.5 }, { 0.5, 0.25 }, { 0.75, 0.25 }, { 1, 0.5 }, { 1.5, 0.5 }, { 2.5, 0.5 }, { 3, 0.5 }, { 3.5, 0.5 } },
              { { 0, 0.25 }, { 0.25, 0.25 }, { 0.5, 0.5 }, { 1.5, 0.5 }, { 2, 1.0 }, { 3.5, 0.5 } } }                                         // busy
        };
        const auto& bank = motifs[juce::jlimit (0, 2, preset.leadDensity)];
        auto blockAt = [&] (double beat) -> const ChordBlock&
        {
            double acc = 0;
            for (auto& bl : arr.blocks) { if (beat < acc + bl.beats) return bl; acc += bl.beats; }
            return arr.blocks.back();
        };
        const double total = arr.totalBeats();
        int pitch = nearestWithPc (arr.tonic, (low + high) / 2 + 4, low, high);
        const int motifA = rng.nextInt ((int) bank.size()), motifB = rng.nextInt ((int) bank.size());
        for (double bar = 0; bar < total - 1e-6; bar += 4)
        {
            const int barIndex = (int) (bar / 4);
            const bool answer = barIndex % 2 == 1;
            const auto& motif = bank[(size_t) (answer ? motifB : motifA)];
            if (preset.leadDensity == 0 && answer && rng.nextFloat() < 0.5f) continue;   // room to breathe
            for (size_t k = 0; k < motif.size(); ++k)
            {
                const double at = bar + motif[k].first;
                if (at >= total - 1e-6) break;
                const auto& bl = blockAt (at);
                std::vector<int> chordTones;
                for (auto x : bl.intervals()) chordTones.push_back ((bl.rootPc + x) % 12);
                const bool strong = std::abs (motif[k].first - std::round (motif[k].first)) < 1e-6 && ((int) motif[k].first % 2 == 0);
                int target = pitch;
                if (k == 0 && ! answer)
                {
                    // a phrase starts with a leap to a chord tone somewhere in the upper half
                    const int aim = (low + high) / 2 + rng.nextInt (juce::Range<int> (-2, 7));
                    int best = pitch; int bestD = 99;
                    for (int n = low; n <= high; ++n)
                        if (std::find (chordTones.begin(), chordTones.end(), n % 12) != chordTones.end() && std::abs (n - aim) < bestD) { bestD = std::abs (n - aim); best = n; }
                    target = best;
                }
                else if (strong || k == 0)
                {
                    // nearest chord tone, preferring a small step from where the line is
                    int best = pitch; int bestD = 99;
                    for (int n = low; n <= high; ++n)
                        if (std::find (chordTones.begin(), chordTones.end(), n % 12) != chordTones.end())
                        {
                            int d = std::abs (n - pitch) + (n == pitch ? 2 : 0);
                            if (d < bestD) { bestD = d; best = n; }
                        }
                    target = best;
                }
                else
                {
                    int dir = rng.nextBool() ? 1 : -1;
                    if (pitch >= high - 2) dir = -1;
                    if (pitch <= low + 2) dir = 1;
                    target = pitch + dir;
                    while (! inScale (target) && target > low && target < high) target += dir;
                }
                pitch = juce::jlimit (low, high, target);
                auto len = std::min (motif[k].second, total - at);
                lead.notes.push_back ({ pitch, at, len * 0.95, vel (leadR, strong ? 1.0f : 0.85f) });
            }
        }
    }

    // ---- swing everything that sits on an off-beat
    if (arr.swing > 0)
        for (auto* p : { &keys, &bass, &lead, &drums })
            for (auto& n : p->notes) n.start += swingShift (n.start, arr.swing);

    arr.parts = { keys, bass, pad, lead, drums };
}

//==============================================================================
// MIDI

static int gmProgram (const juce::String& partId, const juce::String& engine)
{
    if (partId == "bass") return engine.contains ("moog") ? 38 : engine == "808" ? 38 : 33;
    if (engine == "epiano" || engine == "epbell") return 4;
    if (engine == "wurli") return 4;
    if (engine == "organ" || engine == "comboorgan") return 16;
    if (engine == "strings") return 48;
    if (engine == "pad") return 89;
    if (engine == "choir") return 52;
    if (engine == "brass") return 61;
    if (engine == "sax") return 65;
    if (engine == "flute") return 73;
    if (engine == "whistle" || engine == "triangle") return 80;
    if (engine == "lead" || engine == "supersaw") return 81;
    if (engine == "talkbox") return 85;
    if (engine == "bell" || engine == "musicbox") return 10;
    return 0;
}

juce::StringArray writeArrangementMidi (const Arrangement& arr, const juce::File& folder, const juce::String& name)
{
    juce::StringArray written;
    folder.createDirectory();
    const int ppq = 960;
    auto metaTrack = [&] (const juce::String& title)
    {
        juce::MidiMessageSequence meta;
        meta.addEvent (juce::MidiMessage::textMetaEvent (3, title), 0);
        meta.addEvent (juce::MidiMessage::tempoMetaEvent ((int) (60000000.0 / arr.tempo)), 0);
        meta.addEvent (juce::MidiMessage::timeSignatureMetaEvent (4, 4), 0);
        return meta;
    };
    auto partTrack = [&] (const Part& p)
    {
        juce::MidiMessageSequence seq;
        const int ch = juce::jlimit (1, 16, p.channel);
        seq.addEvent (juce::MidiMessage::textMetaEvent (3, p.name + (p.recipeId.isNotEmpty() ? " (" + p.recipeId + ")" : juce::String())), 0);
        if (ch != 10) seq.addEvent (juce::MidiMessage::programChange (ch, gmProgram (p.id, recipeOr (p.recipeId).engine)), 0);
        for (auto& n : p.notes)
        {
            seq.addEvent (juce::MidiMessage::noteOn (ch, juce::jlimit (0, 127, n.note), (juce::uint8) juce::jlimit (1, 127, n.velocity)), n.start * ppq);
            seq.addEvent (juce::MidiMessage::noteOff (ch, juce::jlimit (0, 127, n.note)), juce::jmax (n.start * ppq + 1, (n.start + n.length) * ppq - 1));
        }
        seq.updateMatchedPairs();
        return seq;
    };
    auto save = [&] (const juce::String& fileName, std::vector<juce::MidiMessageSequence> tracks)
    {
        juce::MidiFile mf;
        mf.setTicksPerQuarterNote (ppq);
        for (auto& t : tracks) mf.addTrack (t);
        auto file = folder.getChildFile (safeFileName (fileName) + ".mid");
        file.deleteFile();
        juce::FileOutputStream stream (file);
        if (stream.openedOk() && mf.writeTo (stream)) written.add (file.getFileName());
    };

    // every part on its own track (type 1)
    std::vector<juce::MidiMessageSequence> all { metaTrack (name) };
    for (auto& p : arr.parts) if (p.enabled && ! p.notes.empty()) all.push_back (partTrack (p));
    save (name, all);
    // one file per part
    for (auto& p : arr.parts)
        if (p.enabled && ! p.notes.empty()) save (name + "_" + p.id, { metaTrack (name + " " + p.name), partTrack (p) });
    // the progression as block chords, one per block
    {
        Part chords;
        chords.id = "chords"; chords.name = "Chords"; chords.channel = 1;
        std::vector<int> prev;
        double t = 0;
        for (auto& b : arr.blocks)
        {
            auto notes = voiceLed (prev, b.rootPc, b.intervals(), 48, 64);
            prev = notes;
            for (auto n : notes) chords.notes.push_back ({ n, t, b.beats - 0.02, 90 });
            t += b.beats;
        }
        auto seq = partTrack (chords);
        // chord names as markers, so a DAW shows them
        double at = 0;
        for (auto& b : arr.blocks) { seq.addEvent (juce::MidiMessage::textMetaEvent (6, b.name()), at * ppq); at += b.beats; }
        seq.sort();
        save (name + "_chords", { metaTrack (name + " chords: " + arr.progressionText()), seq });
    }
    return written;
}

//==============================================================================
// Loops

const std::vector<std::pair<juce::String, juce::String>>& vintageFinishes()
{
    static const std::vector<std::pair<juce::String, juce::String>> f {
        { "clean", "Clean (no finish)" }, { "tape70s", "70s Tape" }, { "cassette", "Cassette" }, { "digital80s", "80s Digital" },
        { "sp1200", "90s SP-1200 (crunchy)" }, { "mpc60", "MPC60 / MPC3000" }, { "vinyl", "Vinyl" }, { "dusty", "Dusty Record (vinyl + tape)" } };
    return f;
}

juce::var vintageFinishFx (const juce::String& id)
{
    auto fx = [] (std::initializer_list<std::initializer_list<std::pair<const char*, juce::var>>> list)
    {
        juce::Array<juce::var> a;
        for (auto& e : list)
        {
            auto* o = new juce::DynamicObject();
            for (auto& [k, v] : e) o->setProperty (k, v);
            a.add (juce::var (o));
        }
        return juce::var (a);
    };
    if (id == "tape70s")
        return fx ({ { { "type", "tape" }, { "drive", 0.45 } }, { { "type", "wowflutter" }, { "amount", 0.22 } },
                     { { "type", "lowpass" }, { "freq", 9500 }, { "poles", 2 } }, { { "type", "hiss" }, { "levelDb", -50 } },
                     { { "type", "compressor" }, { "thresholdDb", -14 }, { "ratio", 2 }, { "attackMs", 10 }, { "releaseMs", 150 } } });
    if (id == "cassette")
        return fx ({ { { "type", "tape" }, { "drive", 0.55 } }, { { "type", "wowflutter" }, { "amount", 0.4 } },
                     { { "type", "lowpass" }, { "freq", 7000 }, { "poles", 2 } }, { { "type", "highpass" }, { "freq", 60 }, { "poles", 2 } },
                     { { "type", "hiss" }, { "levelDb", -42 } } });
    if (id == "digital80s")
        return fx ({ { { "type", "peak" }, { "freq", 9000 }, { "gainDb", 3 }, { "q", 0.7 } }, { { "type", "peak" }, { "freq", 250 }, { "gainDb", -2 }, { "q", 0.8 } },
                     { { "type", "compressor" }, { "thresholdDb", -16 }, { "ratio", 3 }, { "attackMs", 5 }, { "releaseMs", 100 } } });
    if (id == "sp1200")
        return fx ({ { { "type", "bitcrush" }, { "bits", 12 }, { "downsample", 2 } }, { { "type", "lowpass" }, { "freq", 11000 }, { "poles", 2 } },
                     { { "type", "tube" }, { "drive", 0.25 } } });
    if (id == "mpc60")
        return fx ({ { { "type", "bitcrush" }, { "bits", 12 }, { "downsample", 1 } }, { { "type", "lowpass" }, { "freq", 15000 }, { "poles", 2 } },
                     { { "type", "tube" }, { "drive", 0.2 } },
                     { { "type", "compressor" }, { "thresholdDb", -12 }, { "ratio", 2.5 }, { "attackMs", 3 }, { "releaseMs", 80 } } });
    if (id == "vinyl")
        return fx ({ { { "type", "highpass" }, { "freq", 35 }, { "poles", 2 } }, { { "type", "lowpass" }, { "freq", 11000 }, { "poles", 2 } },
                     { { "type", "vinyl" }, { "amount", 0.5 } } });
    if (id == "dusty")
        return fx ({ { { "type", "tape" }, { "drive", 0.4 } }, { { "type", "wowflutter" }, { "amount", 0.2 } },
                     { { "type", "lowpass" }, { "freq", 8500 }, { "poles", 2 } }, { { "type", "vinyl" }, { "amount", 0.7 } } });
    return {};
}

/** Plays a part through an instrument over the loop's exact timeline (plus room for its tail). */
static juce::AudioBuffer<float> renderPart (juce::AudioProcessor& proc, const Arrangement& arr, const Part& part, double sr,
                                            int loopLen, int tailLen, bool isPlugin)
{
    const int bs = 512;
    proc.setNonRealtime (true);
    proc.prepareToPlay (sr, bs);
    const int chans = juce::jmax (2, proc.getTotalNumInputChannels(), proc.getTotalNumOutputChannels());
    const int outs = juce::jmax (1, proc.getTotalNumOutputChannels());
    juce::AudioBuffer<float> block (chans, bs);
    if (isPlugin)
    {
        // some instruments load samples in the background: let them, and silence anything left over
        auto end = juce::Time::getMillisecondCounterHiRes() + 1200.0;
        juce::MidiBuffer off;
        for (int ch = 1; ch <= 16; ++ch) off.addEvent (juce::MidiMessage::allNotesOff (ch), 0);
        while (juce::Time::getMillisecondCounterHiRes() < end)
        {
            block.clear();
            proc.processBlock (block, off);
            off.clear();
            juce::Thread::sleep (2);
        }
    }
    struct Ev { juce::int64 pos; juce::MidiMessage msg; };
    std::vector<Ev> events;
    for (auto& n : part.notes)
    {
        auto on = (juce::int64) (n.start * 60.0 / arr.tempo * sr);
        auto offPos = (juce::int64) ((n.start + n.length) * 60.0 / arr.tempo * sr);
        events.push_back ({ on, juce::MidiMessage::noteOn (1, juce::jlimit (0, 127, n.note), (juce::uint8) juce::jlimit (1, 127, n.velocity)) });
        events.push_back ({ juce::jmax (on + 1, offPos), juce::MidiMessage::noteOff (1, juce::jlimit (0, 127, n.note)) });
    }
    std::stable_sort (events.begin(), events.end(), [] (auto& a, auto& b) { return a.pos < b.pos; });
    const auto latency = juce::jmax (0, proc.getLatencySamples());

    juce::AudioBuffer<float> audio (2, loopLen + tailLen + bs + latency);
    audio.clear();
    size_t next = 0;
    juce::int64 written = 0;
    for (juce::int64 pos = 0; pos < loopLen + tailLen + latency; pos += bs)
    {
        juce::MidiBuffer midi;
        while (next < events.size() && events[next].pos < pos + bs) { midi.addEvent (events[next].msg, (int) (events[next].pos - pos)); ++next; }
        block.clear();
        proc.processBlock (block, midi);
        for (int i = 0; i < bs; ++i)
        {
            const auto at = pos + i - latency;            // the plugin's reported delay comes off the front
            if (at < 0 || at >= audio.getNumSamples()) continue;
            audio.setSample (0, (int) at, block.getSample (0, i));
            audio.setSample (1, (int) at, block.getSample (juce::jmin (1, outs - 1), i));
            written = at;
        }
    }
    juce::ignoreUnused (written);
    juce::MidiBuffer off;
    for (int ch = 1; ch <= 16; ++ch) off.addEvent (juce::MidiMessage::allNotesOff (ch), 0);
    block.clear();
    proc.processBlock (block, off);
    proc.releaseResources();
    proc.setNonRealtime (false);
    audio.setSize (2, loopLen + tailLen, true);
    return audio;
}

bool renderArrangement (const Arrangement& arr, double sr, juce::AudioBuffer<float>& mix, std::vector<juce::AudioBuffer<float>>& stems,
                        juce::String& error, InstrumentFor instrumentFor, std::function<void (double)> progress)
{
    const int loopLen = juce::roundToInt (arr.lengthSeconds() * sr);
    if (loopLen <= 0) { error = "the progression is empty"; return false; }
    const int tailLen = (int) (sr * 4.0);
    stems.assign (arr.parts.size(), {});
    mix.setSize (2, loopLen);
    mix.clear();
    const std::map<juce::String, float> partGain { { "keys", 1.0f }, { "bass", 0.9f }, { "pad", 0.5f }, { "lead", 0.7f } };

    for (size_t i = 0; i < arr.parts.size(); ++i)
    {
        if (progress) progress ((double) i / (double) arr.parts.size());
        const auto& part = arr.parts[i];
        if (! part.enabled || part.id == "drums" || part.notes.empty()) continue;
        const auto& recipe = recipeOr (part.recipeId);

        juce::AudioBuffer<float> audio;
        auto* plugin = instrumentFor ? instrumentFor (part) : nullptr;
        if (plugin != nullptr)
            audio = renderPart (*plugin, arr, part, sr, loopLen, tailLen, true);     // your instrument, dry
        else
        {
            BuiltInSynth synth;
            Engine engine;
            if (! parseEngine (recipe.engine == "mixed" ? juce::String ("epiano") : recipe.engine, engine)) parseEngine ("epiano", engine);
            synth.setEngine (engine);
            synth.setVariation (arr.seed * 31 + (juce::int64) i);
            audio = renderPart (synth, arr, part, sr, loopLen, tailLen, false);
            // the recipe's sound: its effects over the whole part (not its one-shot trim / varispeed)
            if (recipe.processing.fx.isArray()) applyFxChain (audio, sr, recipe.processing.fx, arr.tempo, arr.seed + (juce::int64) i);
        }

        // fold the tail back onto the start: the loop plays seamlessly
        juce::AudioBuffer<float> loop (2, loopLen);
        loop.clear();
        for (int ch = 0; ch < 2; ++ch)
            for (int s = 0; s < audio.getNumSamples(); ++s)
                loop.addSample (ch, s % loopLen, audio.getSample (ch, s));

        auto peak = loop.getMagnitude (0, loopLen);
        if (peak > 1.0e-5f)
        {
            auto it = partGain.find (part.id);
            loop.applyGain ((it != partGain.end() ? it->second : 0.7f) * 0.5f / peak);
        }
        stems[i] = loop;
        for (int ch = 0; ch < 2; ++ch) mix.addFrom (ch, 0, loop, ch, 0, loopLen);
    }

    auto peak = mix.getMagnitude (0, loopLen);
    if (peak < 1.0e-5f) { error = "nothing to render (all melodic parts are off, or an instrument made no sound)"; return false; }
    mix.applyGain (0.7f / peak);
    for (auto& st : stems) if (st.getNumSamples() > 0) st.applyGain (0.7f / peak);

    // the vintage finish over the mix and each stem (so stems still sound like the mix)
    auto finish = vintageFinishFx (arr.finish);
    if (finish.isArray())
    {
        applyFxChain (mix, sr, finish, arr.tempo, arr.seed + 99);
        for (size_t i = 0; i < stems.size(); ++i)
            if (stems[i].getNumSamples() > 0) applyFxChain (stems[i], sr, finish, arr.tempo, arr.seed + 99 + (juce::int64) i);
    }

    // master: the mix peaks at -1 dBFS; the stems keep the same gain so they add up to the mix
    peak = mix.getMagnitude (0, loopLen);
    if (peak < 1.0e-5f) { error = "the finish silenced the loop"; return false; }
    const float master = 0.891f / peak;
    mix.applyGain (master);
    for (auto& st : stems) if (st.getNumSamples() > 0) st.applyGain (master);
    if (progress) progress (1.0);
    return true;
}

juce::StringArray writeLoopChops (const Arrangement& arr, const juce::AudioBuffer<float>& loop, const juce::File& folder,
                                  const juce::String& name, const juce::String& mode, double sr, int bits)
{
    juce::StringArray written;
    if (mode == "off" || mode.isEmpty() || loop.getNumSamples() == 0) return written;
    struct Cut { double startBeat, beats; juce::String label; int root; };
    std::vector<Cut> cuts;
    const double total = arr.totalBeats();
    if (mode == "chord")
    {
        double t = 0;
        for (auto& b : arr.blocks) { cuts.push_back ({ t, b.beats, safeFileName (b.name()), 48 + b.rootPc }); t += b.beats; }
    }
    else if (mode == "bar")
    {
        for (double t = 0; t < total - 1e-6; t += 4) cuts.push_back ({ t, std::min (4.0, total - t), "bar" + juce::String ((int) (t / 4) + 1).paddedLeft ('0', 2), 48 + arr.tonic });
    }
    else
    {
        for (int k = 0; k < 16; ++k) cuts.push_back ({ total * k / 16.0, total / 16.0, "slice" + juce::String (k + 1).paddedLeft ('0', 2), 48 + arr.tonic });
    }
    auto chopsFolder = folder.getChildFile (safeFileName (name) + "_Chops");
    chopsFolder.deleteRecursively();
    juce::String map;
    map << "PAD CHOPS - " << name << "\n" << arr.progressionText() << "   (" << juce::String (arr.tempo, 0) << " BPM)\n"
        << "Load each Bank folder into a drum program in order: file 01 on Pad 1 ... 16 on Pad 16.\n"
        << "Play the pads in order to play the progression; set the pads to One Shot so each chop plays out.\n\n";
    const double secPerBeat = 60.0 / arr.tempo;
    for (size_t k = 0; k < cuts.size(); ++k)
    {
        const auto& c = cuts[k];
        const int from = juce::jlimit (0, loop.getNumSamples() - 1, juce::roundToInt (c.startBeat * secPerBeat * sr));
        const int to = juce::jlimit (from + 1, loop.getNumSamples(), juce::roundToInt ((c.startBeat + c.beats) * secPerBeat * sr));
        juce::AudioBuffer<float> chop (2, to - from);
        for (int ch = 0; ch < 2; ++ch) chop.copyFrom (ch, 0, loop, juce::jmin (ch, loop.getNumChannels() - 1), from, to - from);
        const int fadeIn = juce::jmin (chop.getNumSamples() / 4, (int) (sr * 0.002)), fadeOut = juce::jmin (chop.getNumSamples() / 4, (int) (sr * 0.006));
        chop.applyGainRamp (0, fadeIn, 0.0f, 1.0f);
        chop.applyGainRamp (chop.getNumSamples() - fadeOut, fadeOut, 1.0f, 0.0f);
        const int pad = (int) (k % 16) + 1;
        const auto bank = juce::String::charToString ((juce::juce_wchar) ('A' + (int) (k / 16) % 26));
        auto file = chopsFolder.getChildFile ("Bank_" + bank).getChildFile (juce::String (pad).paddedLeft ('0', 2) + "_" + c.label + ".wav");
        file.getParentDirectory().createDirectory();
        juce::String e;
        if (OfflineRenderer::writeWav (file, chop, sr, bits, c.root, e))
            written.add (file.getRelativePathFrom (folder));
        map << "Bank " << bank << "  Pad " << juce::String (pad).paddedLeft (' ', 2) << ":  " << c.label.paddedRight (' ', 12)
            << " beats " << juce::String (c.startBeat, 2) << " - " << juce::String (c.startBeat + c.beats, 2) << "\n";
    }
    chopsFolder.getChildFile ("_Pad_Map.txt").replaceWithText (map);
    return written;
}

juce::StringArray writeArrangementLoops (const Arrangement& arr, const juce::File& folder, const juce::String& name, double sr, int bits,
                                         juce::String& error, const juce::String& chopMode, InstrumentFor instrumentFor,
                                         std::function<void (double)> progress)
{
    juce::StringArray written;
    juce::AudioBuffer<float> mix;
    std::vector<juce::AudioBuffer<float>> stems;
    if (! renderArrangement (arr, sr, mix, stems, error, instrumentFor, progress)) return written;
    folder.createDirectory();
    const int root = 48 + arr.tonic;
    auto write = [&] (const juce::String& fileName, const juce::AudioBuffer<float>& audio)
    {
        juce::String e;
        auto file = folder.getChildFile (safeFileName (fileName) + ".wav");
        if (OfflineRenderer::writeWav (file, audio, sr, bits, root, e)) written.add (file.getFileName());
        else error = e;
    };
    write (name + "_loop", mix);
    for (size_t i = 0; i < arr.parts.size(); ++i)
        if (stems[i].getNumSamples() > 0) write (name + "_stem_" + arr.parts[i].id, stems[i]);
    written.addArray (writeLoopChops (arr, mix, folder, name, chopMode, sr, bits));
    return written;
}

//==============================================================================
// Saved progressions

juce::var arrangementToVar (const Arrangement& a, const SongFileExtras& x)
{
    auto* o = new juce::DynamicObject();
    o->setProperty ("format", "MPCSampleForgeSong");
    o->setProperty ("version", 1);
    o->setProperty ("name", x.name);
    o->setProperty ("favourite", x.favourite);
    o->setProperty ("created", x.created.isNotEmpty() ? x.created : juce::Time::getCurrentTime().toISO8601 (true));
    o->setProperty ("preset", a.presetId);
    o->setProperty ("key", pitchClassName (a.tonic));
    o->setProperty ("scale", a.scale);
    o->setProperty ("tempo", a.tempo);
    o->setProperty ("swing", a.swing);
    o->setProperty ("seed", juce::String (a.seed));
    o->setProperty ("finish", a.finish);
    o->setProperty ("chords", a.progressionText());
    juce::Array<juce::var> blocks;
    for (auto& b : a.blocks)
    {
        auto* bo = new juce::DynamicObject();
        bo->setProperty ("root", pitchClassName (b.rootPc));
        bo->setProperty ("quality", b.quality);
        bo->setProperty ("beats", b.beats);
        blocks.add (juce::var (bo));
    }
    o->setProperty ("blocks", blocks);
    juce::Array<juce::var> parts;
    for (auto& p : a.parts)
    {
        auto* po = new juce::DynamicObject();
        po->setProperty ("id", p.id);
        po->setProperty ("recipe", p.recipeId);
        po->setProperty ("enabled", p.enabled);
        if (p.pluginPath.isNotEmpty())
        {
            po->setProperty ("plugin", p.pluginPath);
            po->setProperty ("pluginName", p.pluginName);
            auto it = x.pluginStates.find (p.id);
            if (it != x.pluginStates.end()) po->setProperty ("pluginState", it->second.toBase64Encoding());
        }
        parts.add (juce::var (po));
    }
    o->setProperty ("parts", parts);
    return juce::var (o);
}

bool arrangementFromVar (const juce::var& v, Arrangement& a, SongFileExtras& x, juce::String& error)
{
    if (v["format"].toString() != "MPCSampleForgeSong") { error = "not an MPC Sample Forge progression"; return false; }
    a = Arrangement();
    x = SongFileExtras();
    x.name = v["name"].toString();
    x.favourite = (bool) v["favourite"];
    x.created = v["created"].toString();
    a.presetId = v["preset"].toString();
    a.tonic = juce::jmax (0, parsePitchClass (v["key"].toString()));
    a.scale = v.getProperty ("scale", "major").toString();
    a.tempo = juce::jlimit (40.0, 220.0, (double) v.getProperty ("tempo", 90.0));
    a.swing = juce::jlimit (0.0, 1.0, (double) v.getProperty ("swing", 0.0));
    a.seed = v["seed"].toString().getLargeIntValue();
    a.finish = v.getProperty ("finish", "clean").toString();
    if (auto* blocks = v["blocks"].getArray())
        for (auto& bv : *blocks)
        {
            ChordBlock b;
            b.rootPc = juce::jmax (0, parsePitchClass (bv["root"].toString()));
            b.quality = bv["quality"].toString();
            b.beats = juce::jlimit (0.5, 32.0, (double) bv.getProperty ("beats", 4.0));
            if (findQuality (b.quality) == nullptr) { error = "unknown chord type " + b.quality; return false; }
            a.blocks.push_back (b);
        }
    if (auto* parts = v["parts"].getArray())
        for (auto& pv : *parts)
        {
            Part p;
            p.id = pv["id"].toString();
            p.recipeId = pv["recipe"].toString();
            p.enabled = (bool) pv.getProperty ("enabled", true);
            p.pluginPath = pv["plugin"].toString();
            p.pluginName = pv["pluginName"].toString();
            if (pv.hasProperty ("pluginState"))
            {
                juce::MemoryBlock state;
                if (state.fromBase64Encoding (pv["pluginState"].toString())) x.pluginStates[p.id] = state;
            }
            a.parts.push_back (p);
        }
    if (a.blocks.empty()) { error = "the progression has no chords"; return false; }
    return true;
}

bool saveSongFile (const juce::File& f, const Arrangement& a, const SongFileExtras& x)
{
    f.getParentDirectory().createDirectory();
    return f.replaceWithText (juce::JSON::toString (arrangementToVar (a, x), false));
}

bool loadSongFile (const juce::File& f, Arrangement& a, SongFileExtras& x, juce::String& error)
{
    if (! f.existsAsFile()) { error = "file not found: " + f.getFullPathName(); return false; }
    return arrangementFromVar (juce::JSON::parse (f), a, x, error);
}

juce::File songLibraryFolder()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("MPCSampleForge").getChildFile ("Progressions");
}

//==============================================================================
// "More like this"

std::vector<ChordBlock> varyProgression (const std::vector<ChordBlock>& blocks, int tonic, const juce::String& scaleName,
                                         juce::int64 seed, int strength)
{
    std::vector<ChordBlock> out = blocks;
    if (strength <= 0 || blocks.empty()) return out;
    juce::Random rng (seed);
    std::vector<int> scale;
    if (! getScale (scaleName, scale)) getScale ("major", scale);
    auto inKey = [&] (const ChordBlock& b)
    {
        for (auto i : b.intervals())
            if (std::find (scale.begin(), scale.end(), ((b.rootPc + i - tonic) % 12 + 12) % 12) == scale.end()) return false;
        return true;
    };
    // richer / simpler colours of the same chord
    static const std::map<juce::String, std::vector<const char*>> colours {
        { "maj7", { "maj9", "69", "maj7s11" } }, { "maj9", { "maj7", "69", "maj13" } }, { "69", { "maj9", "maj7" } },
        { "m7", { "m9", "m11" } }, { "m9", { "m7", "m11" } }, { "m11", { "m9", "m7" } },
        { "7", { "9", "13", "7sus4" } }, { "9", { "13", "9sus4", "7" } }, { "13", { "9", "7s9", "13b9" } },
        { "7sus4", { "9sus4", "7" } }, { "9sus4", { "7sus4", "13" } }, { "7s9", { "7b9", "13" } },
        { "maj", { "add9", "maj7" } }, { "min", { "m7", "madd9" } } };
    const double chance = juce::jlimit (0.15, 0.7, 0.2 + 0.15 * strength);
    for (size_t i = 0; i < out.size(); ++i)
    {
        if (rng.nextDouble() > chance) continue;
        auto& b = out[i];
        const auto& next = blocks[(i + 1) % blocks.size()];
        const int degree = ((b.rootPc - tonic) % 12 + 12) % 12;
        const int pick = rng.nextInt (10);
        if (pick < 4)
        {
            auto it = colours.find (b.quality);
            if (it != colours.end()) b.quality = it->second[(size_t) rng.nextInt ((int) it->second.size())];
        }
        else if (pick < 7)
        {
            // a chord with the same job: I <-> iii / vi, IV <-> ii, vi <-> IV, i <-> bIII / bVI ...
            struct Sub { int from, to; const char* quality; };
            static const Sub subs[] = { { 0, 9, "m9" }, { 0, 4, "m7" }, { 5, 2, "m9" }, { 2, 5, "maj9" }, { 9, 5, "maj9" }, { 9, 0, "maj9" },
                                        { 4, 7, "9sus4" }, { 3, 8, "maj7" }, { 8, 3, "maj9" }, { 10, 5, "9" }, { 5, 10, "9" } };
            std::vector<const Sub*> options;
            for (auto& sb : subs) if (sb.from == degree) options.push_back (&sb);
            if (! options.empty())
            {
                auto* sb = options[(size_t) rng.nextInt ((int) options.size())];
                ChordBlock c = b;
                c.rootPc = (tonic + sb->to) % 12;
                c.quality = sb->quality;
                if (inKey (c) || strength > 2) b = c;
            }
        }
        else if (pick < 9 && i + 1 < out.size())
        {
            // a secondary dominant into the next chord (V7 of it)
            b.rootPc = (next.rootPc + 7) % 12;
            b.quality = rng.nextBool() ? "7s9" : "13";
        }
        else if (b.quality == "7" || b.quality == "9" || b.quality == "13" || b.quality == "7s9")
        {
            // tritone substitution for a dominant
            b.rootPc = (b.rootPc + 6) % 12;
            b.quality = "9";
        }
    }
    return out;
}
}
