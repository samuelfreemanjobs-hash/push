#include "SamplePlanner.h"
#include "BuiltInSynth.h"
#include <set>
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

std::vector<SampleSpec> planSamples (const Style& style, const PlanOptions& opt, juce::String& error)
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

    const bool fixedRoot = juce::isPositiveAndBelow (opt.fixedRoot, 128);
    if (fixedRoot)
    {
        // 16 Levels: the pads transpose from one root, so the key is irrelevant.
        c.keyLocked = false;
        prefix = safeFileName (opt.prefix.isNotEmpty() ? opt.prefix : style.name) + "_" + midiNoteName (opt.fixedRoot);
    }

    auto recipeName = (opt.recipe.isNotEmpty() ? opt.recipe : style.recipe).toLowerCase();
    const bool mixedRecipe = recipeName == "mixed";
    Recipe fixedRecipe = Recipe::EPiano;
    if (! mixedRecipe && recipeName.isNotEmpty() && ! parseRecipe (recipeName, fixedRecipe))
    {
        error = "unknown recipe '" + recipeName + "'";
        return result;
    }
    auto allRecipes = recipeIds();
    auto nextRecipe = [&]
    {
        return mixedRecipe ? allRecipes[rng.nextInt (allRecipes.size())] : recipeId (fixedRecipe);
    };

    // Unique-sample bookkeeping. In 16 Levels mode the same chord/note is allowed
    // again when it sounds different: a new synth variation, or (with a VST3)
    // a different velocity layer / length.
    auto uniqueKey = [&] (const juce::String& base, const juce::String& recipe, int velocity, double hold, bool relaxed)
    {
        if (! fixedRoot && ! relaxed) return base;
        if (opt.builtIn) return base + recipe + juce::String (rng.nextInt64());
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
            rootPc = fixedRoot ? opt.fixedRoot % 12 : rng.nextInt (12);
            quality = findQuality (pickWeighted (style.chords, rng));
        }

        if (quality == nullptr) continue;

        auto voicing = pickWeighted (style.voicings, rng);
        auto isStab = rng.nextFloat() < style.stabChance;

        auto hold = isStab ? style.stabHold : randomDouble (rng, style.holdLow, style.holdHigh);
        auto recipe = nextRecipe();
        auto velocity = c.velocity();

        // Allow the same chord twice only as a stab vs. a held chord.
        auto key = uniqueKey (juce::String (rootPc) + quality->id + voicingId (voicing) + (isStab ? "S" : ""),
                              recipe, velocity, hold, attempts > numChords * 100);
        if (used.count (key)) continue;
        used.insert (key);

        auto root = fixedRoot ? opt.fixedRoot : placePitchClass (rootPc, style.chordRootLow);
        if (! fixedRoot && root > style.chordRootHigh) root -= 12;

        auto notes = buildVoicing (root, quality->intervals, voicing);
        if (fixedRoot && ! notes.empty() && notes.front() < root)
        {
            // keep the pad's root as the lowest note so 16 Levels tuning is unambiguous
            for (auto& n : notes) if (n < root) n += 12;
            std::sort (notes.begin(), notes.end());
            notes.erase (std::unique (notes.begin(), notes.end()), notes.end());
        }

        SampleSpec s;
        s.kind = isStab ? "Stab" : "Chord";
        s.label = pitchClassName (rootPc) + quality->suffix;
        s.voicing = voicingId (voicing);
        s.rootNote = root;
        s.recipe = recipe;
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
        auto note = fixedRoot ? opt.fixedRoot : randomInt (rng, style.keyLow, style.keyHigh);
        if (! c.inKey (note)) continue;

        std::vector<int> notes { note };
        auto hold = randomDouble (rng, style.holdLow, style.holdHigh) * 0.8;
        juce::String label = midiNoteName (note);

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
            label = pitchClassName (note % 12) + q->suffix + "_" + midiNoteName (note);
        }

        auto recipe = nextRecipe();
        auto velocity = c.velocity();
        auto key = uniqueKey (type + juce::String (note) + label, recipe, velocity, hold, attempts > numKeys * 100);
        if (used.count (key)) continue;
        used.insert (key);

        SampleSpec s;
        s.kind = type == "Stab" ? "KeyStab" : type;
        s.label = label;
        s.voicing = type == "Stab" ? "close" : "-";
        s.rootNote = note;
        s.recipe = recipe;
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

    // ------------------------------------------------------------ file names
    for (size_t i = 0; i < result.size(); ++i)
    {
        auto& s = result[i];
        s.index = (int) i + 1;
        auto stem = prefix + "_" + juce::String (s.index).paddedLeft ('0', 3) + "_";
        Recipe r;
        if (opt.builtIn && mixedRecipe && parseRecipe (s.recipe, r))
            stem += recipeDisplayName (r) + "_";
        stem += s.kind + "_" + s.label;
        if (s.kind == "Chord" || s.kind == "Stab")
            stem += "_" + s.voicing;
        s.fileStem = safeFileName (stem);
    }

    return result;
}
}
