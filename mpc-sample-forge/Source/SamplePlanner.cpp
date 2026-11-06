#include "SamplePlanner.h"
#include "BuiltInSynth.h"
#include <set>
#include <map>
#include <numeric>
#include <algorithm>

namespace forge
{
double SampleSpec::holdSec() const
{
    double end = 0;
    for (auto& n : notes) end = std::max (end, n.startSec + n.lengthSec);
    return end;
}

juce::String safeFileName (const juce::String& s)
{
    auto r = s.replace ("#", "s").replace ("/", "-").replace (" ", "");
    return r.retainCharacters ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-+.");
}

template <typename T>
static const T& pickWeighted (const std::vector<Weighted<T>>& items, juce::Random& rng)
{
    float total = 0;
    for (auto& i : items) total += i.weight;
    auto r = rng.nextFloat() * total;
    for (auto& i : items)
    {
        if (r < i.weight) return i.value;
        r -= i.weight;
    }
    return items.back().value;
}

static int randomInt (juce::Random& rng, int lo, int hi) { return lo + rng.nextInt (hi - lo + 1); }
static double randomDouble (juce::Random& rng, double lo, double hi) { return lo + rng.nextDouble() * (hi - lo); }

/** First MIDI note >= low with the given pitch class. */
static int placePitchClass (int pc, int low)
{
    return low + (((pc - low) % 12) + 12) % 12;
}

namespace
{
    struct Context
    {
        const Style& style;
        juce::Random& rng;
        bool keyLocked;
        int tonic;
        std::vector<int> scale;

        bool inKey (int midi) const
        {
            if (! keyLocked) return true;
            auto rel = ((midi - tonic) % 12 + 12) % 12;
            return std::find (scale.begin(), scale.end(), rel) != scale.end();
        }

        int velocity() const { return randomInt (rng, style.velocityLow, style.velocityHigh); }
    };

    /** Velocity per chord note, with a slightly louder top voice like a real player. */
    std::vector<NoteEvent> makeChordNotes (const Context& c, const std::vector<int>& notes, double hold, bool strum, int base)
    {
        std::vector<NoteEvent> out;
        auto spread = strum ? randomDouble (c.rng, 0.3, 1.0) * c.style.strumMs / 1000.0 : 0.0;
        auto perNote = notes.size() > 1 ? spread / (double) (notes.size() - 1) : 0.0;

        for (size_t i = 0; i < notes.size(); ++i)
        {
            NoteEvent e;
            e.note = notes[i];
            auto accent = (i + 1 == notes.size()) ? 6 : 0;
            e.velocity = juce::jlimit (1, 127, base + accent + randomInt (c.rng, -5, 5));
            e.startSec = perNote * (double) i + (strum ? randomDouble (c.rng, 0.0, 0.004) : 0.0);
            e.lengthSec = juce::jmax (0.02, hold - e.startSec);
            out.push_back (e);
        }

        auto minStart = std::min_element (out.begin(), out.end(),
                                          [] (auto& a, auto& b) { return a.startSec < b.startSec; })->startSec;
        for (auto& e : out) e.startSec -= minStart;
        return out;
    }
}

static juce::String shortVoicing (const juce::String& v)
{
    static const std::map<juce::String, juce::String> m { { "close", "cl" }, { "inv1", "i1" }, { "inv2", "i2" }, { "drop2", "d2" },
                                                          { "open", "op" }, { "rootless", "rl" }, { "shell", "sh" }, { "spread", "sp" },
                                                          { "root", "rt" }, { "inv3", "i3" }, { "inv4", "i4" } };
    auto it = m.find (v);
    return it != m.end() ? it->second : v;
}

static juce::String shortKind (const juce::String& kind)
{
    if (kind == "Octave") return "oct";
    if (kind == "Fifth") return "5th";
    if (kind == "Stab" || kind == "KeyStab") return "stab";
    return {};
}

static void nameSamples (std::vector<SampleSpec>& specs, const juce::String& prefix, bool withTag, bool withEngine,
                         bool shortNames = false, const juce::String& shortTag = {})
{
    for (size_t i = 0; i < specs.size(); ++i)
    {
        auto& s = specs[i];
        if (shortNames)
        {
            // number first so the MPC's alphabetical list matches pad order
            juce::String tail;
            Engine r;
            if (withTag && s.shortTag.isNotEmpty())        tail += s.shortTag + "_";
            else if (shortTag.isNotEmpty())                 tail += shortTag + "_";
            if (withEngine && parseEngine (s.engine, r))    tail += engineDisplayName (r).substring (0, 6) + "_";
            tail += s.kind == "KeyStab" ? s.label.upToFirstOccurrenceOf ("_", false, false) : s.label;
            auto extra = (s.kind == "Chord" || s.kind == "Stab") ? shortVoicing (s.voicing) : juce::String();
            if (s.kind == "Stab" || s.kind == "KeyStab") extra = (extra.isNotEmpty() ? extra + "_" : juce::String()) + "stab";
            else if (shortKind (s.kind).isNotEmpty())      extra = shortKind (s.kind);
            if (extra.isNotEmpty()) tail += "_" + extra;
            s.stemHead = {};
            s.stemTail = tail;
            continue;
        }

        juce::String tail;
        if (withTag && s.tag.isNotEmpty())
            tail += s.tag + "_";
        Engine r;
        if (withEngine && parseEngine (s.engine, r))
            tail += engineDisplayName (r) + "_";
        tail += s.kind + "_" + s.label;
        if (s.kind == "Chord" || s.kind == "Stab")
            tail += "_" + s.voicing;
        s.stemHead = prefix;
        s.stemTail = tail;
    }
    renumberSamples (specs);
}

void renumberSamples (std::vector<SampleSpec>& specs)
{
    for (size_t i = 0; i < specs.size(); ++i)
    {
        auto& s = specs[i];
        s.index = (int) i + 1;
        auto number = s.pad > 0 ? juce::String (s.pad).paddedLeft ('0', 2) : juce::String (s.index).paddedLeft ('0', 3);
        s.fileStem = safeFileName (s.stemHead.isEmpty() ? number + "_" + s.stemTail
                                                         : s.stemHead + "_" + number + "_" + s.stemTail);
    }
}

bool transposeSpec (const SampleSpec& in, int semitones, SampleSpec& out)
{
    out = in;
    if (semitones == 0) return true;
    for (auto& n : out.notes)
    {
        n.note += semitones;
        if (n.note < 21 || n.note > 108) return false; // stay on the piano: below that it is rumble, above it whistle
    }
    out.rootNote = in.rootNote + semitones;
    if (out.rootNote < 24 || out.rootNote > 108) return false;

    auto oldRoot = midiNoteName (in.rootNote), newRoot = midiNoteName (out.rootNote);
    if (in.kind == "Key" || in.kind == "Octave" || in.kind == "Fifth")
        out.label = newRoot;
    else if (in.kind == "KeyStab")
        out.label = in.label.upToLastOccurrenceOf ("_", true, false) + newRoot;

    // octave numbers in the name follow the new register (short stab names carry none)
    if (out.label != in.label)
        out.stemTail = in.stemTail.replaceFirstOccurrenceOf (in.label, out.label);
    if (in.stemHead.endsWith ("_" + oldRoot))
        out.stemHead = in.stemHead.dropLastCharacters (oldRoot.length()) + newRoot;

    auto number = out.pad > 0 ? juce::String (out.pad).paddedLeft ('0', 2) : juce::String (out.index).paddedLeft ('0', 3);
    out.fileStem = safeFileName (out.stemHead.isEmpty() ? number + "_" + out.stemTail : out.stemHead + "_" + number + "_" + out.stemTail);
    return true;
}

static std::vector<SampleSpec> planSingle (const Style& style, const PlanOptions& opt, juce::String& error)
{
    std::vector<SampleSpec> result;
    juce::Random rng (opt.seed);

    Context c { style, rng, false, 0, {} };

    auto scaleName = opt.scale.isNotEmpty() ? opt.scale : style.scale;
    if (! getScale (scaleName, c.scale))
    {
        error = "unknown scale '" + scaleName + "'";
        return result;
    }

    if (opt.key.isNotEmpty() && ! opt.key.equalsIgnoreCase ("random"))
    {
        c.tonic = parsePitchClass (opt.key);
        if (c.tonic < 0) { error = "unknown key '" + opt.key + "'"; return result; }
        c.keyLocked = true;
    }

    auto count = juce::jlimit (1, 2000, opt.count);
    auto numChords = (int) std::round ((float) count * style.chordRatio);
    auto numKeys = count - numChords;

    auto prefix = safeFileName (opt.prefix.isNotEmpty() ? opt.prefix : style.name);
    if (c.keyLocked)
        prefix += "_" + pitchClassName (c.tonic) + (scaleName.containsIgnoreCase ("minor") || scaleName == "dorian"
                                                       || scaleName == "phrygian" ? "min" : "maj");

    // Tape-style varispeed after rendering moves the sounding pitch: names and the
    // WAV root note describe what you hear, not what was played.
    const int ps = style.processing.pitchShift;
    if (c.keyLocked) // plan in the played key so the varispeeded result lands in the requested key
        c.tonic = ((c.tonic - ps) % 12 + 12) % 12;

    const int rootMidi = opt.fixedRoot == PlanOptions::styleRoot ? (style.root >= 0 ? style.root : 60) : opt.fixedRoot;
    const bool fixedRoot = juce::isPositiveAndBelow (rootMidi, 128);
    if (fixedRoot)
    {
        // 16 Levels: the pads transpose from one root, so the key is irrelevant.
        c.keyLocked = false;
        prefix = safeFileName (opt.prefix.isNotEmpty() ? opt.prefix : style.name) + "_" + midiNoteName (rootMidi + ps);
    }

    auto engineName = (opt.engine.isNotEmpty() ? opt.engine : style.engine).toLowerCase();
    const bool mixedEngine = engineName == "mixed";
    Engine fixedEngine = Engine::EPiano;
    if (! mixedEngine && engineName.isNotEmpty() && ! parseEngine (engineName, fixedEngine))
    {
        error = "unknown engine '" + engineName + "'";
        return result;
    }
    auto allRecipes = engineIds();
    auto nextEngine = [&]
    {
        return mixedEngine ? allRecipes[rng.nextInt (allRecipes.size())] : engineId (fixedEngine);
    };

    // Unique-sample bookkeeping. In 16 Levels mode the same chord/note is allowed
    // again when it sounds different: a new synth variation, or (with a VST3)
    // a different velocity layer / length.
    auto uniqueKey = [&] (const juce::String& base, const juce::String& engine, int velocity, double hold, bool relaxed)
    {
        if (! fixedRoot && ! relaxed) return base;
        if (opt.builtIn) return base + engine + juce::String (rng.nextInt64());
        return base + "v" + juce::String (velocity / 12) + "h" + juce::String ((int) (hold * 4));
    };

    std::set<juce::String> used;

    // For a locked key: the scale degrees and which of the style's chords fit on each.
    std::vector<std::pair<int, std::vector<Weighted<juce::String>>>> diatonic;
    if (c.keyLocked)
    {
        for (auto degree : c.scale)
        {
            auto rootPc = (c.tonic + degree) % 12;
            std::vector<Weighted<juce::String>> fitting;
            for (auto& w : style.chords)
                if (auto* q = findQuality (w.value); q != nullptr && chordFitsScale (rootPc, q->intervals, c.tonic, c.scale))
                    fitting.push_back (w);
            if (! fitting.empty())
                diatonic.push_back ({ rootPc, fitting });
        }

        if (diatonic.empty()) // none of the style's chords fit this scale: use plain diatonic triads
            for (auto degree : c.scale)
            {
                auto rootPc = (c.tonic + degree) % 12;
                for (auto id : { "min", "maj", "dim", "sus2", "5" })
                    if (chordFitsScale (rootPc, findQuality (id)->intervals, c.tonic, c.scale))
                    {
                        diatonic.push_back ({ rootPc, { { id, 1.0f } } });
                        break;
                    }
            }
    }

    // ---------------------------------------------------------------- chords
    for (int attempts = 0; (int) result.size() < numChords && attempts < numChords * 300; ++attempts)
    {
        int rootPc;
        const ChordQuality* quality = nullptr;

        if (c.keyLocked)
        {
            auto& [degreeRoot, fitting] = diatonic[(size_t) rng.nextInt ((int) diatonic.size())];
            rootPc = degreeRoot;
            quality = findQuality (pickWeighted (fitting, rng));
        }
        else
        {
            rootPc = fixedRoot ? rootMidi % 12 : rng.nextInt (12);
            quality = findQuality (pickWeighted (style.chords, rng));
        }

        if (quality == nullptr) continue;

        auto voicing = pickWeighted (style.voicings, rng);
        auto isStab = rng.nextFloat() < style.stabChance;

        auto hold = isStab ? style.stabHold : randomDouble (rng, style.holdLow, style.holdHigh);
        auto engine = nextEngine();
        auto velocity = c.velocity();

        // Allow the same chord twice only as a stab vs. a held chord.
        auto key = uniqueKey (juce::String (rootPc) + quality->id + voicingId (voicing) + (isStab ? "S" : ""),
                              engine, velocity, hold, attempts > numChords * 100);
        if (used.count (key)) continue;
        used.insert (key);

        auto root = fixedRoot ? rootMidi : placePitchClass (rootPc, style.chordRootLow);
        if (! fixedRoot && root > style.chordRootHigh) root -= 12;

        auto notes = buildVoicing (root, quality->intervals, voicing);

        // Skip muddy low voicings (low interval limits) while there are other options.
        if (attempts < numChords * 150 && lowIntervalViolations (notes) > 0)
        {
            used.erase (key);
            continue;
        }
        if (fixedRoot && ! notes.empty() && notes.front() < root)
        {
            // keep the pad's root as the lowest note so 16 Levels tuning is unambiguous
            for (auto& n : notes) if (n < root) n += 12;
            std::sort (notes.begin(), notes.end());
            notes.erase (std::unique (notes.begin(), notes.end()), notes.end());
        }

        SampleSpec s;
        s.kind = isStab ? "Stab" : "Chord";
        s.label = pitchClassName (rootPc + ps) + quality->suffix;
        s.voicing = voicingId (voicing);
        s.rootNote = juce::jlimit (0, 127, root + ps);
        s.engine = engine;
        s.variation = rng.nextInt64();
        s.notes = makeChordNotes (c, notes, hold, ! isStab, velocity);
        result.push_back (s);
    }

    // ------------------------------------------------------ single-key shots
    std::vector<Weighted<juce::String>> keyTypes;
    if (style.keyNote > 0)   keyTypes.push_back ({ "Key", style.keyNote });
    if (style.keyOctave > 0) keyTypes.push_back ({ "Octave", style.keyOctave });
    if (style.keyFifth > 0)  keyTypes.push_back ({ "Fifth", style.keyFifth });
    if (style.keyStab > 0)   keyTypes.push_back ({ "Stab", style.keyStab });

    auto keysEnd = (int) result.size() + numKeys;
    for (int attempts = 0; (int) result.size() < keysEnd && attempts < numKeys * 300; ++attempts)
    {
        auto type = pickWeighted (keyTypes, rng);
        auto note = fixedRoot ? rootMidi : randomInt (rng, style.keyLow, style.keyHigh);
        if (! c.inKey (note)) continue;

        std::vector<int> notes { note };
        auto hold = randomDouble (rng, style.holdLow, style.holdHigh) * 0.8;
        juce::String label = midiNoteName (note + ps);

        if (type == "Octave")
        {
            notes.push_back (note + 12);
        }
        else if (type == "Fifth")
        {
            notes.push_back (note + 7);
            notes.push_back (note + 12);
            if (! c.inKey (note + 7)) continue;
        }
        else if (type == "Stab")
        {
            // short triad stab, diatonic when a key is chosen
            const ChordQuality* q = nullptr;
            for (auto id : { rng.nextBool() ? "min" : "maj", "min", "maj", "sus2", "5" })
                if (auto* cand = findQuality (id); ! c.keyLocked || chordFitsScale (note % 12, cand->intervals, c.tonic, c.scale))
                {
                    q = cand;
                    break;
                }
            if (q == nullptr) continue;
            notes = buildVoicing (note, q->intervals, rng.nextBool() ? Voicing::Close : Voicing::Inversion1);
            hold = style.stabHold;
            label = pitchClassName (note + ps) + q->suffix + "_" + midiNoteName (note + ps);
        }

        auto engine = nextEngine();
        auto velocity = c.velocity();
        auto key = uniqueKey (type + juce::String (note) + label, engine, velocity, hold, attempts > numKeys * 100);
        if (used.count (key)) continue;
        used.insert (key);

        SampleSpec s;
        s.kind = type == "Stab" ? "KeyStab" : type;
        s.label = label;
        s.voicing = type == "Stab" ? "close" : "-";
        s.rootNote = juce::jlimit (0, 127, note + ps);
        s.engine = engine;
        s.variation = rng.nextInt64();
        s.notes = makeChordNotes (c, notes, hold, false, velocity);
        result.push_back (s);
    }

    if ((int) result.size() < count && fixedRoot && ! opt.builtIn)
        error = "only " + juce::String ((int) result.size()) + " distinct samples: with a VST3 on one root, variety comes only "
                "from velocity, length and voicing. Lower the count, or render several patches into separate batches";
    else if ((int) result.size() < count)
        error = "only " + juce::String ((int) result.size()) + " unique samples are possible with these settings "
                "(widen the style's ranges, add chord types, or use key=random)";

    if (fixedRoot)
    {
        // Interleave chords and single notes so every 16-pad bank gets a mix.
        std::vector<SampleSpec> keys (result.begin() + (std::ptrdiff_t) std::min ((size_t) numChords, result.size()), result.end());
        result.resize (std::min ((size_t) numChords, result.size()));
        std::vector<SampleSpec> mixed;
        size_t ci = 0, ki = 0;
        while (ci < result.size() || ki < keys.size())
        {
            auto chordTurn = ki >= keys.size()
                             || (ci < result.size() && (double) ci / (double) result.size() <= (double) ki / (double) keys.size());
            mixed.push_back (chordTurn ? result[ci++] : keys[ki++]);
        }
        result = std::move (mixed);
    }

    for (auto& sp : result)
    {
        sp.processing = style.processing;
        sp.styleId = style.id;
        sp.tag = style.tag;
        sp.shortTag = style.shortTag;
    }

    nameSamples (result, prefix, false, opt.builtIn && mixedEngine, opt.shortNames, style.shortTag);
    return result;
}



//==============================================================================
std::vector<int> invertChord (int rootMidi, const std::vector<int>& intervals, int inversion)
{
    std::vector<int> notes;
    for (auto i : intervals) notes.push_back (rootMidi + i);
    std::sort (notes.begin(), notes.end());
    notes.erase (std::unique (notes.begin(), notes.end()), notes.end());
    for (int k = 0; k < inversion && notes.size() > 1; ++k)
    {
        notes.front() += 12;
        std::sort (notes.begin(), notes.end());
    }
    return notes;
}

static const char* romanFor (int degree, bool minorChord)
{
    static const char* upper[] = { "I", "II", "III", "IV", "V", "VI", "VII" };
    static const char* lower[] = { "i", "ii", "iii", "iv", "v", "vi", "vii" };
    return minorChord ? lower[degree % 7] : upper[degree % 7];
}

std::vector<DiatonicChord> diatonicChords (const Style& style, int tonic, const std::vector<int>& scale, juce::int64 variety)
{
    std::vector<DiatonicChord> out;
    juce::Random rng (variety);
    for (int d = 0; d < (int) scale.size(); ++d)
    {
        auto rootPc = (tonic + scale[(size_t) d]) % 12;
        std::vector<Weighted<juce::String>> fitting;
        for (auto& w : style.chords)
            if (auto* q = findQuality (w.value); q != nullptr && chordFitsScale (rootPc, q->intervals, tonic, scale))
                fitting.push_back (w);
        if (fitting.empty())
            for (auto id : { "m7", "maj7", "7", "m7b5", "min", "maj", "dim", "sus2", "5" })
                if (auto* q = findQuality (id); chordFitsScale (rootPc, q->intervals, tonic, scale)) { fitting.push_back ({ id, 1.0f }); break; }
        if (fitting.empty()) fitting.push_back ({ "5", 1.0f });

        juce::String id;
        if (variety != 0) id = pickWeighted (fitting, rng);
        else
        {
            auto best = fitting.front();
            for (auto& w : fitting) if (w.weight > best.weight) best = w;
            id = best.value;
        }
        auto* q = findQuality (id);
        DiatonicChord c;
        c.degree = d;
        c.rootPc = rootPc;
        c.intervals = q->intervals;
        c.roman = romanFor (d, q->intervals.size() > 1 && q->intervals[1] == 3);
        c.name = pitchClassName (rootPc) + q->suffix;
        out.push_back (c);
    }
    return out;
}

std::vector<std::pair<juce::String, std::vector<int>>> progressionsFor (const std::vector<int>& scale)
{
    std::vector<std::pair<juce::String, std::vector<int>>> p;
    if (scale.size() < 7) // pentatonic & co: simple shapes over the available degrees
        return { { "1-4-5-1", { 0, 3, 4, 0 } }, { "1-3-4-1", { 0, 2, 3, 0 } }, { "1-5-4-3", { 0, 4, 3, 2 } }, { "1-2-4-5", { 0, 1, 3, 4 } } };

    const bool major = scale[2] == 4;
    const bool phrygian = scale[1] == 1;
    const bool dorian = ! major && scale[5] == 9;
    if (major)
        p = { { "ii-V-I-vi", { 1, 4, 0, 5 } }, { "I-vi-IV-V", { 0, 5, 3, 4 } }, { "IV-iii-ii-I", { 3, 2, 1, 0 } },
              { "I-V-vi-IV", { 0, 4, 5, 3 } }, { "vi-IV-I-V", { 5, 3, 0, 4 } }, { "IV-V-iii-vi", { 3, 4, 2, 5 } },
              { "I-IV-ii-V", { 0, 3, 1, 4 } }, { "iii-vi-ii-V", { 2, 5, 1, 4 } } };
    else
        p = { { "i-VI-III-VII", { 0, 5, 2, 6 } }, { "i-iv-VII-III", { 0, 3, 6, 2 } }, { "i-VII-VI-VII", { 0, 6, 5, 6 } },
              { "i-iv-v-i", { 0, 3, 4, 0 } }, { "VI-VII-i-i", { 5, 6, 0, 0 } }, { "i-III-VII-VI", { 0, 2, 6, 5 } },
              { "ii-v-i-i", { 1, 4, 0, 0 } }, { "iv-i-v-VI", { 3, 0, 4, 5 } } };
    if (dorian)   p.insert (p.begin(), { { "i-IV-i-IV", { 0, 3, 0, 3 } }, { "i-ii-IV-i", { 0, 1, 3, 0 } } });
    if (phrygian) p.insert (p.begin(), { { "i-II-i-VII", { 0, 1, 0, 6 } }, { "i-II-VII-i", { 0, 1, 6, 0 } } });
    return p;
}

/** Picks the inversion / octave of `intervals` on `rootPc` that moves least from `previous`. */
static std::vector<int> voiceLead (const std::vector<int>& previous, int rootPc, const std::vector<int>& intervals,
                                   int low, int high, int& inversionOut)
{
    std::vector<int> best;
    double bestCost = 1e9;
    for (int base = low - 12; base <= high + 12; ++base)
    {
        if (((base % 12) + 12) % 12 != rootPc) continue;
        for (int inv = 0; inv < (int) intervals.size(); ++inv)
        {
            auto notes = invertChord (base, intervals, inv);
            if (notes.front() < low - 5 || notes.back() > high + 24) continue;
            double cost = 0;
            for (size_t i = 0; i < notes.size(); ++i)
            {
                // distance to the nearest note of the previous chord: smooth voice leading
                int d = 99;
                for (auto p : previous) d = std::min (d, std::abs (notes[i] - p));
                cost += d;
            }
            cost += 6.0 * lowIntervalViolations (notes); // never muddy
            cost += 0.15 * std::abs ((notes.front() + notes.back()) / 2 - (low + high) / 2 + 6); // stay near the register
            if (cost < bestCost) { bestCost = cost; best = notes; inversionOut = inv; }
        }
    }
    return best;
}

//==============================================================================
// Scale banks: each bank of 16 pads is ONE sound, climbing the scale from pad 1
// (single notes) or playing the diatonic chord of each degree (chords), so the
// pads are in tune without 16 Levels > Tune.
static const char* romanNumeral (int degree, bool minorish)
{
    static const char* upper[] = { "I", "II", "III", "IV", "V", "VI", "VII" };
    static const char* lower[] = { "i", "ii", "iii", "iv", "v", "vi", "vii" };
    return minorish ? lower[degree % 7] : upper[degree % 7];
}

static std::vector<SampleSpec> planScaleBanks (const Style& kitOrStyle, const PlanOptions& opt, juce::String& error)
{
    std::vector<SampleSpec> result;
    juce::Random rng (opt.seed);

    // members: a kit spreads its banks over its recipes
    std::vector<const Style*> members;
    if (kitOrStyle.isKit())
    {
        for (auto& m : kitOrStyle.kit)
            if (auto* st = findBuiltInStyle (m.value); st != nullptr && ! st->isKit()) members.push_back (st);
    }
    else members.push_back (&kitOrStyle);
    if (members.empty()) { error = "no styles to build banks from"; return result; }

    int tonic = parsePitchClass (opt.key);
    if (opt.key.isEmpty() || opt.key.equalsIgnoreCase ("random") || tonic < 0)
        tonic = rng.nextInt (12); // one key for every bank, so they all play together

    const int banks = juce::jlimit (1, 26 * 4, (juce::jlimit (1, 2000, opt.count) + 15) / 16);
    auto engineList = engineIds();

    for (int b = 0; b < banks; ++b)
    {
        const Style& style = *members[(size_t) b % members.size()];
        auto scaleName = opt.scale.isNotEmpty() ? opt.scale : style.scale;
        std::vector<int> scale;
        if (! getScale (scaleName, scale)) { error = "unknown scale '" + scaleName + "'"; return {}; }

        const int ps = style.processing.pitchShift;
        const int playedTonic = ((tonic - ps) % 12 + 12) % 12; // so the varispeeded result lands in the key
        const auto type = opt.bankType.isNotEmpty() ? opt.bankType : style.bankType();
        const auto info = pitchClassName (tonic) + " " + scaleName + " - " + type;

        auto engineName = (opt.engine.isNotEmpty() ? opt.engine : style.engine).toLowerCase();
        if (engineName == "mixed" || engineName.isEmpty()) engineName = engineList[rng.nextInt (engineList.size())];
        const auto variation = rng.nextInt64();               // one timbre per bank
        const auto velocity = (style.velocityLow + style.velocityHigh) / 2 + rng.nextInt (5) - 2;
        const auto hold = (style.holdLow + style.holdHigh) * 0.5;

        auto makeSpec = [&] (int pad, const std::vector<int>& notes, int root)
        {
            SampleSpec s;
            s.pad = pad;
            s.rootNote = juce::jlimit (0, 127, root + ps);
            s.engine = engineName;
            s.variation = variation;
            s.processing = style.processing;
            s.styleId = style.id;
            s.tag = style.tag;
            s.shortTag = style.shortTag;
            s.bankInfo = info;
            for (size_t i = 0; i < notes.size(); ++i)
            {
                NoteEvent e;
                e.note = notes[i];
                e.velocity = juce::jlimit (1, 127, velocity + (i + 1 == notes.size() ? 4 : 0));
                e.startSec = 0;
                e.lengthSec = hold;
                s.notes.push_back (e);
            }
            return s;
        };

        if (type == "notes")
        {
            // 16 consecutive scale notes, centred in the style's register
            auto noteAt = [&] (int base, int i) { return base + scale[(size_t) i % scale.size()] + 12 * (i / (int) scale.size()); };
            auto base = placePitchClass (playedTonic, juce::jmax (24, style.keyLow - 12));
            auto centre = (style.keyLow + style.keyHigh) / 2;
            auto best = base;
            for (int cand = base; cand <= 96; cand += 12)
            {
                if (noteAt (cand, 15) > 108) break;
                if (std::abs ((cand + noteAt (cand, 15)) / 2 - centre) < std::abs ((best + noteAt (best, 15)) / 2 - centre)) best = cand;
            }
            for (int pad = 1; pad <= 16; ++pad)
            {
                auto note = noteAt (best, pad - 1);
                auto s = makeSpec (pad, { note }, note);
                s.kind = "Key";
                s.label = midiNoteName (note + ps);
                s.voicing = "-";
                result.push_back (s);
            }
        }
        else if (type == "progressions")
        {
            // 4 progressions x 4 chords, one per row of pads, voice-led for smooth movement
            auto chords = diatonicChords (style, playedTonic, scale, variation | 1);
            auto all = progressionsFor (scale);
            std::vector<size_t> order (all.size());
            std::iota (order.begin(), order.end(), 0);
            for (size_t i = order.size(); i > 1; --i) std::swap (order[i - 1], order[(size_t) rng.nextInt ((int) i)]);

            for (int row = 0; row < 4; ++row)
            {
                auto& prog = all[order[(size_t) row % order.size()]];
                std::vector<int> previous;
                for (int step = 0; step < 4; ++step)
                {
                    auto& c = chords[(size_t) prog.second[(size_t) step] % chords.size()];
                    int inversion = 0;
                    std::vector<int> notes;
                    if (previous.empty())
                    {
                        auto root = placePitchClass (c.rootPc, style.chordRootLow);
                        notes = invertChord (root, c.intervals, 0);
                        if (lowIntervalViolations (notes) > 0) { notes = invertChord (root + 12, c.intervals, 0); }
                    }
                    else
                        notes = voiceLead (previous, c.rootPc, c.intervals, style.chordRootLow, style.chordRootHigh, inversion);
                    previous = notes;

                    auto pad = row * 4 + step + 1;
                    auto rootMidi = notes.front();
                    for (auto n : notes) if (((n - c.rootPc) % 12 + 12) % 12 == 0) { rootMidi = n; break; }
                    auto s = makeSpec (pad, notes, rootMidi);
                    s.kind = "Chord";
                    s.label = c.roman + "-" + pitchClassName (c.rootPc + ps) + c.name.substring (pitchClassName (c.rootPc).length());
                    s.voicing = inversion == 0 ? "root" : "inv" + juce::String (inversion);
                    s.group = prog.first;
                    result.push_back (s);
                }
            }
        }
        else
        {
            // the diatonic chord on each degree; each pass up the scale uses another voicing
            std::vector<Voicing> passVoicings;
            std::vector<Weighted<Voicing>> pool = style.voicings;
            for (int pass = 0; pass < 3; ++pass)
            {
                if (pool.empty()) pool = style.voicings;
                auto v = pickWeighted (pool, rng);
                passVoicings.push_back (v);
                pool.erase (std::remove_if (pool.begin(), pool.end(), [v] (auto& w) { return w.value == v; }), pool.end());
            }

            const int degrees = (int) scale.size();
            std::vector<const ChordQuality*> qualities ((size_t) degrees, nullptr);
            for (int d = 0; d < degrees; ++d)
            {
                auto rootPc = (playedTonic + scale[(size_t) d]) % 12;
                std::vector<Weighted<juce::String>> fitting;
                for (auto& w : style.chords)
                    if (auto* q = findQuality (w.value); q != nullptr && chordFitsScale (rootPc, q->intervals, playedTonic, scale))
                        fitting.push_back (w);
                if (fitting.empty()) // stack the scale's own thirds: 7th chord, else triad
                    for (auto id : { "m7", "maj7", "7", "m7b5", "min", "maj", "dim", "sus2", "5" })
                        if (auto* q = findQuality (id); chordFitsScale (rootPc, q->intervals, playedTonic, scale)) { fitting.push_back ({ id, 1.0f }); break; }
                if (! fitting.empty()) qualities[(size_t) d] = findQuality (pickWeighted (fitting, rng));
            }

            for (int pad = 1; pad <= 16; ++pad)
            {
                auto d = (pad - 1) % degrees;
                auto pass = juce::jmin (2, (pad - 1) / degrees);
                auto* q = qualities[(size_t) d];
                if (q == nullptr) q = findQuality ("5");
                auto rootPc = (playedTonic + scale[(size_t) d]) % 12;
                auto root = placePitchClass (rootPc, style.chordRootLow);
                if (root > style.chordRootHigh) root -= 12;
                if (pass >= 2) root += 12; // pads past the second pass: the same chords an octave up, not repeats

                auto voicing = passVoicings[(size_t) juce::jmin (pass, 1)];
                auto notes = buildVoicing (root, q->intervals, voicing);
                for (auto alt : { Voicing::Close, Voicing::Inversion1, Voicing::Open })
                    if (lowIntervalViolations (notes) > 0) { voicing = alt; notes = buildVoicing (root, q->intervals, alt); }

                auto s = makeSpec (pad, notes, root);
                s.kind = "Chord";
                // roman numeral of the degree (lower case = minor-ish chord) so progressions read off the pads
                auto minorChord = q->intervals.size() > 1 && q->intervals[1] == 3;
                s.label = juce::String (romanNumeral (d, minorChord)) + "-" + pitchClassName (rootPc + ps) + q->suffix;
                s.voicing = voicingId (voicing);
                result.push_back (s);
            }
        }
    }

    // names: <style>_<key>_<pad>_... or, short, <pad>_<tag>_<label>
    for (auto& s : result)
    {
        auto* st = findBuiltInStyle (s.styleId);
        auto shortTag = s.shortTag.isNotEmpty() ? s.shortTag : (st != nullptr ? st->shortTag : juce::String());
        if (opt.shortNames)
        {
            s.stemHead = {};
            s.stemTail = (shortTag.isNotEmpty() ? shortTag + "_" : juce::String()) + s.label
                         + (s.kind == "Chord" ? "_" + shortVoicing (s.voicing) : juce::String());
        }
        else
        {
            s.stemHead = safeFileName ((opt.prefix.isNotEmpty() ? opt.prefix : kitOrStyle.name)) + "_" + pitchClassName (tonic)
                         + "_" + s.bankInfo.fromFirstOccurrenceOf (" ", false, false).upToFirstOccurrenceOf (" ", false, false);
            s.stemTail = (kitOrStyle.isKit() ? s.tag + "_" : juce::String()) + s.kind + "_" + s.label
                         + (s.kind == "Chord" ? "_" + s.voicing : juce::String());
        }
    }
    renumberSamples (result);
    return result;
}

std::vector<SampleSpec> planSamples (const Style& style, const PlanOptions& opt, juce::String& error)
{
    if (opt.scaleBank || style.scaleBankOnly)
        return planScaleBanks (style, opt, error);

    if (! style.isKit())
        return planSingle (style, opt, error);

    // ---- a kit: split the count between member styles by weight
    std::vector<const Style*> members;
    std::vector<float> weights;
    for (auto& m : style.kit)
    {
        auto* member = findBuiltInStyle (m.value);
        if (member == nullptr || member->isKit())
        {
            error = "kit '" + style.id + "' refers to unknown style '" + m.value + "'";
            return {};
        }
        members.push_back (member);
        weights.push_back (m.weight);
    }

    auto count = juce::jlimit (1, 2000, opt.count);
    float total = 0;
    for (auto w : weights) total += w;

    std::vector<int> counts;
    int assigned = 0;
    for (auto w : weights) { counts.push_back ((int) std::floor (count * w / total)); assigned += counts.back(); }
    for (size_t i = 0; assigned < count; i = (i + 1) % counts.size()) { ++counts[i]; ++assigned; }

    std::vector<std::vector<SampleSpec>> parts;
    juce::StringArray warnings;
    for (size_t i = 0; i < members.size(); ++i)
    {
        if (counts[i] <= 0) continue;
        auto o = opt;
        o.count = counts[i];
        o.seed = opt.seed + (juce::int64) (i + 1) * 7919;
        o.prefix = {};
        juce::String err;
        parts.push_back (planSingle (*members[i], o, err));
        if (err.isNotEmpty()) warnings.add (members[i]->name + ": " + err);
    }

    // round-robin so every 16-pad bank holds a bit of each recipe
    std::vector<SampleSpec> result;
    for (size_t k = 0;; ++k)
    {
        bool any = false;
        for (auto& p : parts)
            if (k < p.size()) { result.push_back (p[k]); any = true; }
        if (! any) break;
    }

    auto prefix = safeFileName (opt.prefix.isNotEmpty() ? opt.prefix : style.tag);
    nameSamples (result, prefix, true, false, opt.shortNames);
    error = warnings.joinIntoString ("; ");
    return result;
}
}