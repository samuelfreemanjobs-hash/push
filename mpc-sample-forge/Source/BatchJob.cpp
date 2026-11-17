#include "BatchJob.h"
#include "ProgressionLibrary.h"
#include "BuiltInSynth.h"
#include "Analysis.h"
#include <thread>

namespace forge
{
juce::String describeSpec (const SampleSpec& s)
{
    juce::StringArray notes;
    for (auto& n : s.notes) notes.add (midiNoteName (n.note));
    return s.fileStem + "  [" + notes.joinIntoString (" ") + "]  " + juce::String (s.holdSec(), 2) + "s";
}

BatchJob::BatchJob (juce::AudioProcessor& p, BatchRequest r, std::vector<SampleSpec> s)
    : juce::Thread ("MPC Sample Forge render"), proc (p), request (std::move (r)), specs (std::move (s))
{
}

BatchJob::~BatchJob()
{
    stopThread (30000);
}

void BatchJob::post (int done, int total, const juce::String& msg)
{
    if (auto cb = onProgress)
        juce::MessageManager::callAsync ([cb, done, total, msg] { cb (done, total, msg); });
}

juce::String BatchJob::mpcSetupSheet() const
{
    juce::String t;
    t << "MPC SAMPLE FORGE - pad setup sheet\n"
      << "==================================\n\n"
      << "Instrument: " << request.instrumentName << "\n"
      << "Style:      " << request.style.name << "\n\n"
      << "For every pad (drum program):\n"
      << "  * Playback: One-Shot. The file already starts on the transient and ends at the\n"
      << "    recipe's end point, so no Trim (Shift + Pad 13) is needed.\n"
      << "  * 16 Levels: Tune. Pick the pad, enable 16 Levels and choose Tune (Pitch).\n"
      << "  * Note names use the MPC convention (MIDI 60 = C3). The root of each file is\n"
      << "    also stored inside the WAV.\n\n";

    juce::StringArray seen;
    for (auto& spec : specs)
    {
        if (seen.contains (spec.styleId)) continue;
        seen.add (spec.styleId);

        const Style* st = findBuiltInStyle (spec.styleId);
        if (st == nullptr && spec.styleId == request.style.id) st = &request.style;
        if (st == nullptr) continue;

        auto& P = st->processing;
        t << "--- " << st->name << " ---\n";
        if (st->description.isNotEmpty()) t << st->description << "\n";
        t << "  Root (as rendered): " << midiNoteName (spec.rootNote) << " (MIDI " << spec.rootNote << ")\n";
        if (P.maxLength > 0)  t << "  End point:  " << juce::roundToInt (P.maxLength * 1000) << " ms (already trimmed)\n";
        if (P.ampDecayMs > 0) t << "  Amp decay:  " << juce::roundToInt (P.ampDecayMs) << " ms exponential (baked in; leave pad Decay open,\n"
                                << "              or set Decay Mode to " << juce::roundToInt (P.ampDecayMs) << " ms to tighten further)\n";
        if (P.attackMs > 0)   t << "  Fade-in:    " << juce::roundToInt (P.attackMs) << " ms (baked in)\n";
        if (P.pitchShift != 0) t << "  Varispeed:  " << P.pitchShift << " semitones (baked in)\n";
        if (st->pad.tune != 0) t << "  Pad Tune:   " << (st->pad.tune > 0 ? "+" : "") << st->pad.tune << " semitones\n";
        if (st->pad.muteGroup > 0) t << "  Mute Group: " << st->pad.muteGroup << " (monophonic choke for clean runs)\n";
        if (st->pad.fullLevel) t << "  Velocity:   Full Level on (Shift + Pad 1)\n";
        if (st->pad.phrasing.isNotEmpty()) t << "  16 Levels phrasing: " << st->pad.phrasing << "\n";

        juce::StringArray files;
        for (auto& other : specs)
            if (other.styleId == spec.styleId)
                files.add ((request.splitBanks ? "Bank_" + juce::String::charToString ((juce::juce_wchar) ('A' + ((other.index - 1) / 16) % 26)) + "/" : juce::String())
                           + other.fileStem + ".wav");
        t << "  Files (" << files.size() << "):\n    " << files.joinIntoString ("\n    ") << "\n\n";
    }

    if (! specs.empty() && specs.front().pad > 0)
    {
        t << "SCALE BANKS - every Bank_ folder is ONE sound, laid out across the pads in key.\n"
          << "Load a bank into a drum program in order: file 01 on Pad 1 ... file 16 on Pad 16.\n"
          << "No 16 Levels needed - every pad is already the right note / chord.\n\n";
        for (size_t b = 0; b * 16 < specs.size(); ++b)
        {
            const auto& first = specs[b * 16];
            auto* st = findBuiltInStyle (first.styleId);
            t << "Bank_" << juce::String::charToString ((juce::juce_wchar) ('A' + (int) b % 26)) << ": "
              << (st != nullptr ? st->name : first.tag) << "  (" << first.bankInfo << ")\n";
            // the MPC's 4x4 grid: pads 13-16 on top, 1-4 at the bottom
            for (int row = 3; row >= 0; --row)
            {
                t << "   ";
                for (int col = 0; col < 4; ++col)
                {
                    auto i = b * 16 + (size_t) (row * 4 + col);
                    auto cell = i < specs.size() ? juce::String (row * 4 + col + 1).paddedLeft ('0', 2) + " " + specs[i].label : juce::String();
                    t << cell.paddedRight (' ', 18);
                }
                t << "\n";
            }
            // progression banks: which pads play which progression
            juce::StringArray rowsText;
            for (size_t i = b * 16; i < specs.size() && i < b * 16 + 16;)
            {
                auto j = i;
                while (j + 1 < specs.size() && j + 1 < b * 16 + 16 && specs[j + 1].group == specs[i].group) ++j;
                if (specs[i].group.isNotEmpty())
                    rowsText.add ("   pads " + juce::String (specs[i].pad) + (j > i ? "-" + juce::String (specs[j].pad) : juce::String())
                                  + ": " + specs[i].group);
                i = j + 1;
            }
            if (first.progressionSet > 0)
                if (auto* lib = st != nullptr ? &progressionLibrary (st->progressionLibrary) : nullptr)
                    for (auto& set : *lib)
                        if (set.number == first.progressionSet && set.tip.isNotEmpty())
                            rowsText.add ("   Tip: " + set.tip);
            if (! rowsText.isEmpty()) t << rowsText.joinIntoString ("\n") << "\n";
            t << "\n";
        }
    }

    if (! request.registers.empty())
    {
        t << "Register sets: every sample is also in ";
        juce::StringArray names;
        for (auto r : request.registers) if (r != 0) names.add ((r > 0 ? "+" : "") + juce::String (r) + " st");
        t << names.joinIntoString (", ") << " (folders Octave_Down / Octave_Up ...).\n"
          << "  With 16 Levels > Tune, load the register closest to the notes you play: big\n"
          << "  re-pitches make one-shots grainy / short (up) or dull / long (down).\n\n";
    }
    if (request.roundRobins > 1)
        t << "Round robins: Round_Robin/Take_2" << (request.roundRobins > 2 ? " ... Take_" + juce::String (request.roundRobins) : juce::String())
          << " hold other takes of every sample\n"
          << "  (same pad numbers, names end _rr2 ...): each played again a little differently - velocity,\n"
          << "  timing and length - so repeated hits don't sound machine-gunned. Put take 1-" << request.roundRobins
          << " on the layers of one pad\n  and set the layers to cycle / random if your MPC has pad layers, or swap takes between passes.\n\n";
    if (request.render.loudnessMatch)
        t << "Loudness matched: pads play at even volume, so you can leave pad levels alone.\n\n";

    t << "Workflow tips from the recipe guide:\n"
      << "  * Put melodic pads in a Mute Group so each new note chokes the last one.\n"
      << "  * Assign a Q-Link to Filter Cutoff or Pitch Bend while recording 16 Levels patterns.\n"
      << "  * For Chopped & Screwed: resample a 16 Levels phrase, then Half Speed (Shift + Pad 6)\n"
      << "    or drop it -12 semitones.\n"
      << "  * Puzzle-piece layering: e.g. Rhodes Bass on Pad 1, EP Bell on Pad 2, Pluck on Pad 3,\n"
      << "    switching 16 Levels between them.\n";
    return t;
}

static juce::String csv (const juce::String& s)
{
    return "\"" + s.replace ("\"", "\"\"") + "\"";
}

static juce::String manifestHeader()
{
    return "file,recipe,type,name,voicing,engine,root_note,root_midi,midi_notes,velocities,hold_seconds,length_seconds,score,"
           "register_semitones,loudness_lufs,gain_db,sounds_like,sounds_like_db,source\n";
}

static juce::String manifestRow (const juce::File& batchFolder, const SampleSpec& spec, const juce::File& file, double lengthSec,
                                 int score, int shift, float loudness, float gainDb, bool builtIn, const juce::String& similar, float similarDb,
                                 const juce::File& source = {})
{
    juce::StringArray notes, vels;
    for (auto& nn : spec.notes) { notes.add (juce::String (nn.note)); vels.add (juce::String (nn.velocity)); }
    juce::String r;
    r << csv (file.getRelativePathFrom (batchFolder).replaceCharacter ('\\', '/')) << "," << spec.tag << "," << spec.kind << ","
      << csv (spec.label) << "," << spec.voicing << "," << (builtIn ? spec.engine : juce::String ("-")) << ","
      << midiNoteName (spec.rootNote) << "," << spec.rootNote << "," << csv (notes.joinIntoString (" ")) << ","
      << csv (vels.joinIntoString (" ")) << "," << juce::String (spec.holdSec(), 3) << ","
      << juce::String (lengthSec, 3) << "," << (score >= 0 ? juce::String (score) : juce::String()) << ","
      << shift << "," << juce::String (loudness, 1) << "," << juce::String (gainDb, 1) << ","
      << similar << "," << (similar.isNotEmpty() ? juce::String (similarDb, 2) : juce::String()) << ","
      << (source == juce::File() ? juce::String() : csv (source.getRelativePathFrom (batchFolder).replaceCharacter ('\\', '/'))) << "\n";
    return r;
}

static bool applyGainToWav (const juce::File& file, float gainDb, int rootNote, juce::String& error)
{
    juce::WavAudioFormat wav;
    std::unique_ptr<juce::AudioFormatReader> reader (wav.createReaderFor (file.createInputStream().release(), true));
    if (reader == nullptr) { error = "cannot read " + file.getFileName(); return false; }
    juce::AudioBuffer<float> audio ((int) reader->numChannels, (int) reader->lengthInSamples);
    reader->read (&audio, 0, audio.getNumSamples(), 0, true, true);
    auto sr = reader->sampleRate;
    auto bits = (int) reader->bitsPerSample;
    reader.reset();
    audio.applyGain (juce::Decibels::decibelsToGain (gainDb));
    return OfflineRenderer::writeWav (file, audio, sr, bits, rootNote, error);
}

juce::File BatchJob::bankFolder (int index, const juce::File& folder) const
{
    if (! request.splitBanks) return folder;
    auto bank = index / 16;
    return folder.getChildFile ("Bank_" + juce::String::charToString ((juce::juce_wchar) ('A' + bank % 26))
                                + (bank >= 26 ? juce::String (bank / 26 + 1) : juce::String()));
}

bool planBatch (BatchRequest& req, bool smartPick, std::vector<SampleSpec>& specs, juce::StringArray& notes)
{
    juce::String error;
    const int wanted = req.plan.count;
    const bool exact = req.plan.usePalette || req.plan.useCard;   // exactly what was asked for
    const bool banks = ! exact && (req.plan.scaleBank || req.style.scaleBankOnly);
    req.smartPickTarget = 0;
    if (req.plan.useCard)
    {
        smartPick = false;
        req.variety = 0;
        req.plan.scaleBank = false;
        req.splitBanks = false;
    }
    if (req.plan.usePalette)
    {
        // exactly the chords you picked: nothing chosen for you, nothing dropped as "too similar"
        if (smartPick) notes.add ("Chord palette: rendering exactly your " + juce::String ((int) req.plan.palette.size()) + " picks (Smart Pick is skipped).");
        smartPick = false;
        req.variety = 0;
        req.plan.scaleBank = false;
    }
    if (banks)
    {
        req.splitBanks = true; // one sound per 16-pad bank
        if (smartPick || ! req.registers.empty())
            notes.add ("Scale banks: Smart Pick and Registers are skipped - each bank is a complete 16-pad set across the scale.");
        req.registers.clear();
    }
    if (! banks && smartPick)
    {
        req.plan.count = juce::jmin (2000, (int) std::ceil (wanted * 1.5));
        req.smartPickTarget = wanted;
    }
    specs = planSamples (req.style, req.plan, error);
    if (req.smartPickTarget > 0)
    {
        if ((int) specs.size() <= wanted)
        {
            notes.add ("Smart Pick skipped: only " + juce::String ((int) specs.size()) + " distinct samples are possible with these settings.");
            req.smartPickTarget = 0;
        }
        error = {};
    }
    req.plan.count = wanted;
    if (error.isNotEmpty()) notes.add ("WARNING: " + error);
    return ! specs.empty();
}

juce::var specToVar (const SampleSpec& s)
{
    auto* o = new juce::DynamicObject();
    o->setProperty ("index", s.index);         o->setProperty ("pad", s.pad);
    o->setProperty ("fileStem", s.fileStem);   o->setProperty ("stemHead", s.stemHead);
    o->setProperty ("stemTail", s.stemTail);   o->setProperty ("kind", s.kind);
    o->setProperty ("label", s.label);         o->setProperty ("voicing", s.voicing);
    o->setProperty ("rootNote", s.rootNote);   o->setProperty ("engine", s.engine);
    o->setProperty ("variation", juce::String (s.variation));
    o->setProperty ("styleId", s.styleId);     o->setProperty ("tag", s.tag);
    o->setProperty ("shortTag", s.shortTag);   o->setProperty ("bankInfo", s.bankInfo);
    o->setProperty ("group", s.group);         o->setProperty ("progressionSet", s.progressionSet);
    juce::Array<juce::var> notes;
    for (auto& n : s.notes)
    {
        auto* e = new juce::DynamicObject();
        e->setProperty ("note", n.note); e->setProperty ("velocity", n.velocity);
        e->setProperty ("start", n.startSec); e->setProperty ("length", n.lengthSec);
        notes.add (juce::var (e));
    }
    o->setProperty ("notes", notes);
    auto* p = new juce::DynamicObject();
    p->setProperty ("maxLength", s.processing.maxLength);   p->setProperty ("attackMs", s.processing.attackMs);
    p->setProperty ("ampDecayMs", s.processing.ampDecayMs); p->setProperty ("fxTailMs", s.processing.fxTailMs);
    p->setProperty ("pitchShift", s.processing.pitchShift); p->setProperty ("tempo", s.processing.tempo);
    p->setProperty ("fx", s.processing.fx);
    o->setProperty ("processing", juce::var (p));
    return juce::var (o);
}

bool specFromVar (const juce::var& v, SampleSpec& s)
{
    if (! v.isObject() || ! v["notes"].isArray()) return false;
    s = SampleSpec();
    s.index = v["index"]; s.pad = v["pad"];
    s.fileStem = v["fileStem"]; s.stemHead = v["stemHead"]; s.stemTail = v["stemTail"];
    s.kind = v["kind"]; s.label = v["label"]; s.voicing = v["voicing"];
    s.rootNote = v["rootNote"]; s.engine = v["engine"]; s.variation = v["variation"].toString().getLargeIntValue();
    s.styleId = v["styleId"]; s.tag = v["tag"]; s.shortTag = v["shortTag"]; s.bankInfo = v["bankInfo"];
    s.group = v["group"]; s.progressionSet = v["progressionSet"];
    for (auto& e : *v["notes"].getArray())
        s.notes.push_back ({ (int) e["note"], (int) e["velocity"], (double) e["start"], (double) e["length"] });
    auto p = v["processing"];
    s.processing.maxLength = p["maxLength"]; s.processing.attackMs = p["attackMs"];
    s.processing.ampDecayMs = p["ampDecayMs"]; s.processing.fxTailMs = p["fxTailMs"];
    s.processing.pitchShift = p["pitchShift"]; s.processing.tempo = p.getProperty ("tempo", 90.0);
    s.processing.fx = p["fx"];
    return true;
}

static juce::String registerFolderName (int shift)
{
    auto octaves = std::abs (shift) / 12;
    return (shift % 12 == 0 ? (octaves == 1 ? juce::String() : juce::String (octaves) + "_") + (octaves == 1 ? "Octave" : "Octaves")
                            : juce::String (std::abs (shift)) + "_Semitones")
           + (shift < 0 ? "_Down" : "_Up");
}

void BatchJob::renderParallel (int n, const Task& task)
{
    if (n <= 0) return;
    std::atomic<int> next { 0 };
    auto settings = request.render;

    auto work = [&] (juce::AudioProcessor* instrument)
    {
        auto s = settings;
        if (dynamic_cast<BuiltInSynth*> (instrument) != nullptr) s.warmUpSec = 0; // nothing to load
        OfflineRenderer renderer (*instrument, s);
        renderer.warmUp();
        for (;;)
        {
            auto i = next.fetch_add (1);
            if (i >= n || threadShouldExit()) break;
            task (i, renderer, *instrument);
        }
    };

    auto used = (size_t) juce::jmin ((int) workers.size(), n);
    std::vector<std::thread> threads;
    for (size_t k = 1; k < used; ++k) threads.emplace_back (work, workers[k]);
    work (workers[0]);
    for (auto& t : threads) t.join();
}

void BatchJob::run()
{
    auto finish = [this] (bool ok, const juce::String& summary)
    {
        if (auto cb = onFinished)
            juce::MessageManager::callAsync ([cb, ok, summary] { cb (ok, summary); });
    };
    const auto started = juce::Time::getMillisecondCounterHiRes();

    const auto& folder = request.outputFolder;
    if (! folder.createDirectory())
    {
        finish (false, "Could not create output folder " + folder.getFullPathName());
        return;
    }

    // ---- every instance of the instrument renders in parallel
    workers = { &proc };
    auto* builtIn = dynamic_cast<BuiltInSynth*> (&proc);
    if (builtIn != nullptr)
    {
        auto cores = juce::jlimit (1, 8, juce::SystemStats::getNumCpus());
        for (int k = 1; k < cores; ++k)
        {
            ownedWorkers.push_back (std::make_unique<BuiltInSynth>());
            workers.push_back (ownedWorkers.back().get());
        }
    }
    else
        for (auto* p : request.extraInstances) if (p != nullptr) workers.push_back (p);

    auto setUp = [] (juce::AudioProcessor& instrument, const SampleSpec& spec)
    {
        if (auto* b = dynamic_cast<BuiltInSynth*> (&instrument))
        {
            b->reset();  // no leftover (inaudible) tails from whatever this worker rendered before
            Engine e;
            if (parseEngine (spec.engine, e)) b->setEngine (e);
            b->setVariation (spec.variation);
        }
    };
    auto fail = [this] (const juce::String& msg) { const juce::ScopedLock sl (failureLock); failures.add (msg); };

    if (request.registersOnly)
    {
        runRegistersOnly();
        elapsed = (juce::Time::getMillisecondCounterHiRes() - started) / 1000.0;
        return;
    }

    const int total = (int) specs.size();
    const bool picking = request.smartPickTarget > 0 && request.smartPickTarget < total;
    const bool banks = ! specs.empty() && specs.front().pad > 0;
    post (0, total, "Preparing " + request.instrumentName + (workers.size() > 1 ? " (" + juce::String ((int) workers.size()) + " in parallel)" : juce::String()) + " ...");

    // With Smart Pick every candidate is rendered into a scratch folder first.
    auto workDir = picking ? folder.getChildFile (".candidates") : folder;
    if (picking) { workDir.deleteRecursively(); workDir.createDirectory(); }

    struct Rendered { bool ok = false; juce::File file; double lengthSec = 0; float loudness = -100; SampleFeatures features; };
    std::vector<Rendered> rendered ((size_t) total);

    renderParallel (total, [&] (int i, OfflineRenderer& renderer, juce::AudioProcessor& instrument)
    {
        auto& spec = specs[(size_t) i];
        post (progressDone++, total, (picking ? juce::String ("Rendering candidate ") : juce::String ("Rendering ")) + spec.fileStem);
        setUp (instrument, spec);

        juce::AudioBuffer<float> audio;
        juce::String error;
        auto file = picking ? workDir.getChildFile ("cand_" + juce::String (i).paddedLeft ('0', 4) + ".wav")
                            : bankFolder (i).getChildFile (spec.fileStem + ".wav");
        if (request.appendOnly) file = file.getNonexistentSibling (false); // never overwrite an earlier export
        file.getParentDirectory().createDirectory();

        if (! renderer.render (spec, audio, error)
            || ! OfflineRenderer::writeWav (file, audio, request.render.sampleRate, request.render.bitDepth, spec.rootNote, error))
        {
            fail (spec.fileStem + ": " + error);
            return;
        }

        auto& r = rendered[(size_t) i];
        r.ok = true;
        r.file = file;
        r.lengthSec = audio.getNumSamples() / request.render.sampleRate;
        r.loudness = momentaryLoudness (audio, request.render.sampleRate);
        r.features = analyseSample (audio, request.render.sampleRate, spec.rootNote, spec.kind == "Key" || spec.kind == "Octave");
        audioMicros += (juce::int64) (r.lengthSec * 1.0e6);
    });

    // variety thresholds: how close (in dB of fingerprint difference) counts as "the same sound"
    const float dupDb = request.appendOnly ? 0.0f : request.variety == 2 ? 1.5f : request.variety == 1 ? 0.6f : 0.0f;

    // ------------------------------------------------------------ Smart Pick
    std::vector<int> origin;          // final position -> candidate index
    std::vector<PickVerdict> verdicts;
    juce::String pickSummary, varietySummary;

    if (picking && ! threadShouldExit())
    {
        post (total, total, "Smart Pick: scoring " + juce::String (total) + " candidates ...");

        std::vector<PickCandidate> cands ((size_t) total);
        for (int i = 0; i < total; ++i)
        {
            auto& spec = specs[(size_t) i];
            auto& c = cands[(size_t) i];
            c.valid = rendered[(size_t) i].ok;
            c.features = rendered[(size_t) i].features;
            c.rootNote = spec.rootNote;
            c.group = spec.styleId + ((spec.kind == "Chord" || spec.kind == "Stab") ? "/chords" : "/keys");
            std::vector<int> sounding;
            for (auto& nn : spec.notes) sounding.push_back (nn.note + spec.processing.pitchShift);
            c.lowIntervalViolations = lowIntervalViolations (sounding, &c.lowIntervalDetail);
        }
        PickSettings ps;
        ps.duplicateDb = juce::jmax (0.6f, dupDb);
        ps.strictVariety = request.variety == 2;
        verdicts = smartPick (cands, request.smartPickTarget, ps);

        std::vector<SampleSpec> finalSpecs;
        for (int i = 0; i < total; ++i)
            if (verdicts[(size_t) i].kept) { origin.push_back (i); finalSpecs.push_back (specs[(size_t) i]); }
        renumberSamples (finalSpecs);

        for (size_t j = 0; j < finalSpecs.size(); ++j)
        {
            auto& r = rendered[(size_t) origin[j]];
            auto dest = bankFolder ((int) j).getChildFile (finalSpecs[j].fileStem + ".wav");
            dest.getParentDirectory().createDirectory();
            dest.deleteFile();
            if (! r.file.moveFileTo (dest)) fail ("could not move " + r.file.getFileName() + " to " + dest.getFullPathName());
            r.file = dest;
        }

        // report every decision, so you can see why something was left out
        juce::String report = "candidate,kept,final_file,score,reasons,similar_to_candidate,nearest_db,harmonicity,mud,harsh,sub,crest_db,centroid_hz\n";
        int dupes = 0, flawed = 0, spares = 0;
        double scoreSum = 0;
        for (int i = 0; i < total; ++i)
        {
            auto& v = verdicts[(size_t) i];
            auto& f = rendered[(size_t) i].features;
            juce::String finalName;
            for (size_t j = 0; j < origin.size(); ++j)
                if (origin[j] == i) finalName = rendered[(size_t) i].file.getRelativePathFrom (folder).replaceCharacter ('\\', '/');
            if (v.kept) scoreSum += v.score;
            else if (v.similarTo >= 0) ++dupes;
            else if (! v.reasons.isEmpty()) ++flawed;
            else ++spares;

            report << (i + 1) << "," << (v.kept ? "yes" : "no") << "," << csv (finalName) << "," << juce::roundToInt (v.score) << ","
                   << csv (v.reasons.joinIntoString ("; ")) << "," << (v.similarTo >= 0 ? juce::String (v.similarTo + 1) : juce::String()) << ","
                   << juce::String (v.nearest > 1e8f ? 0.0f : v.nearest, 2) << "," << juce::String (f.harmonicity, 2) << ","
                   << juce::String (f.mudShare, 3) << "," << juce::String (f.harshShare, 3) << "," << juce::String (f.subShare, 3) << ","
                   << juce::String (f.crestDb, 1) << "," << juce::roundToInt (f.centroidHz) << "\n";
        }
        folder.getChildFile ("_smart_pick.csv").replaceWithText (report);
        workDir.deleteRecursively();

        pickSummary << "Smart Pick: rendered " << total << " candidates, kept the best " << (int) finalSpecs.size()
                    << " (average score " << juce::roundToInt (scoreSum / juce::jmax<size_t> (1, finalSpecs.size())) << "/100).\n"
                    << "  Left out: " << dupes << " near-duplicates, " << flawed << " with problems (muddy / harsh / unclear pitch ...), "
                    << spares << " good spares. Details: _smart_pick.csv";
        specs = std::move (finalSpecs);
    }
    else
    {
        for (int i = 0; i < total; ++i) origin.push_back (i);
        if (picking) workDir.deleteRecursively();

        // No Smart Pick: still drop samples that sound (nearly) the same as an earlier one.
        // Scale banks are left alone - same sound on every pad is the point of a bank.
        if (dupDb > 0 && ! banks && ! threadShouldExit())
        {
            std::vector<int> keptIdx;
            int removed = 0;
            for (int i = 0; i < total; ++i)
            {
                auto& r = rendered[(size_t) i];
                if (! r.ok) continue;
                bool dup = false;
                for (auto k : keptIdx)
                    if (fingerprintDistance (r.features, rendered[(size_t) k].features) < dupDb) { dup = true; break; }
                if (dup) { r.file.deleteFile(); r.ok = false; ++removed; }
                else keptIdx.push_back (i);
            }
            if (removed > 0)
            {
                // close the gaps: renumber the survivors and refill the pad banks in order
                std::vector<SampleSpec> kept;
                std::vector<int> keptOrigin;
                for (auto i : keptIdx) { kept.push_back (specs[(size_t) i]); keptOrigin.push_back (i); }
                renumberSamples (kept);
                for (size_t j = 0; j < kept.size(); ++j)
                {
                    auto& r = rendered[(size_t) keptOrigin[j]];
                    auto dest = bankFolder ((int) j).getChildFile (kept[j].fileStem + ".wav");
                    if (dest == r.file) continue;
                    dest.getParentDirectory().createDirectory();
                    if (r.file.moveFileTo (dest)) r.file = dest;
                }
                for (auto& sub : folder.findChildFiles (juce::File::findDirectories, false, "Bank_*"))
                    if (sub.getNumberOfChildFiles (juce::File::findFiles) == 0) sub.deleteRecursively();
                specs.clear();
                origin.clear();
                for (size_t j = 0; j < kept.size(); ++j) { specs.push_back (kept[j]); origin.push_back (keptOrigin[j]); }
                varietySummary << "Variety: removed " << removed << " near-duplicate" << (removed == 1 ? "" : "s")
                               << (request.variety == 2 ? " (strict)" : "") << " - the batch is a little smaller but every file sounds different";
            }
        }
    }

    // ------------------------------------------------------------ the final list of files
    struct Row { SampleSpec spec; juce::File file; double lengthSec = 0; int score = -1; int shift = 0; float loudness = -100; float gainDb = 0;
                 SampleFeatures features; int similarTo = -1; float similarDb = 0; juce::File source; };
    std::vector<Row> rows;
    for (size_t j = 0; j < specs.size() && j < origin.size(); ++j)
    {
        auto& r = rendered[(size_t) origin[j]];
        if (! r.ok) continue;
        Row row;
        row.spec = specs[j]; row.file = r.file; row.lengthSec = r.lengthSec; row.loudness = r.loudness; row.features = r.features;
        row.score = verdicts.empty() ? -1 : juce::roundToInt (verdicts[(size_t) origin[j]].score);
        rows.push_back (row);
    }
    const auto originals = rows.size();

    // "sounds like": each sample's closest match in the batch (shown in Review)
    if (! banks)
        for (size_t a = 0; a < originals; ++a)
        {
            float best = 1e9f; int who = -1;
            for (size_t b = 0; b < originals; ++b)
                if (a != b)
                    if (auto d = fingerprintDistance (rows[a].features, rows[b].features); d < best) { best = d; who = (int) b; }
            rows[a].similarTo = who; rows[a].similarDb = best;
        }

    // ------------------------------------------------------------ register sets (for every sample)
    std::vector<int> shifts;
    std::atomic<int> outOfRange { 0 };
    for (auto s : request.registers) if (s != 0 && std::find (shifts.begin(), shifts.end(), s) == shifts.end()) shifts.push_back (s);

    if (! shifts.empty() && ! threadShouldExit())
    {
        const int jobs = (int) (shifts.size() * originals);
        std::vector<Row> extra ((size_t) jobs);
        std::vector<bool> made ((size_t) jobs, false);
        progressDone = 0;
        renderParallel (jobs, [&] (int t, OfflineRenderer& renderer, juce::AudioProcessor& instrument)
        {
            auto shift = shifts[(size_t) t / originals];
            auto j = (size_t) t % originals;
            auto name = registerFolderName (shift);
            post (progressDone++, jobs, "Register " + name + ": " + rows[j].spec.fileStem);
            SampleSpec moved;
            if (! transposeSpec (rows[j].spec, shift, moved)) { ++outOfRange; return; } // e.g. a bass an octave lower
            setUp (instrument, moved);
            juce::AudioBuffer<float> audio;
            juce::String error;
            auto file = folder.getChildFile (name).getChildFile (rows[j].file.getParentDirectory().getRelativePathFrom (folder))
                              .getChildFile (moved.fileStem + ".wav");
            if (rows[j].file.getParentDirectory() == folder) file = folder.getChildFile (name).getChildFile (moved.fileStem + ".wav");
            file.getParentDirectory().createDirectory();
            if (! renderer.render (moved, audio, error)
                || ! OfflineRenderer::writeWav (file, audio, request.render.sampleRate, request.render.bitDepth, moved.rootNote, error))
            {
                fail (moved.fileStem + " (" + name + "): " + error);
                return;
            }
            auto& row = extra[(size_t) t];
            row.spec = moved; row.file = file; row.lengthSec = audio.getNumSamples() / request.render.sampleRate;
            row.score = rows[j].score; row.shift = shift; row.loudness = momentaryLoudness (audio, request.render.sampleRate);
            row.source = rows[j].file;
            audioMicros += (juce::int64) (row.lengthSec * 1.0e6);
            made[(size_t) t] = true;
        });
        for (size_t t = 0; t < extra.size(); ++t) if (made[t]) rows.push_back (extra[t]);
    }

    // ------------------------------------------------------------ round robins (for every sample)
    const int takes = juce::jlimit (1, 4, request.roundRobins);
    int takesMade = 0;
    if (takes > 1 && ! threadShouldExit())
    {
        const int jobs = (int) ((takes - 1) * originals);
        std::vector<Row> extra ((size_t) jobs);
        std::vector<bool> made ((size_t) jobs, false);
        progressDone = 0;
        renderParallel (jobs, [&] (int t, OfflineRenderer& renderer, juce::AudioProcessor& instrument)
        {
            const int take = 2 + t / (int) originals;
            const auto j = (size_t) t % originals;
            auto spec = roundRobinTake (rows[j].spec, take);
            auto name = "Round_Robin/Take_" + juce::String (take);
            post (progressDone++, jobs, "Take " + juce::String (take) + ": " + spec.fileStem);
            setUp (instrument, spec);
            juce::AudioBuffer<float> audio;
            juce::String error;
            auto rel = rows[j].file.getParentDirectory().getRelativePathFrom (folder);
            auto dir = rows[j].file.getParentDirectory() == folder ? folder.getChildFile (name) : folder.getChildFile (name).getChildFile (rel);
            auto file = dir.getChildFile (spec.fileStem + ".wav");
            dir.createDirectory();
            if (! renderer.render (spec, audio, error)
                || ! OfflineRenderer::writeWav (file, audio, request.render.sampleRate, request.render.bitDepth, spec.rootNote, error))
            {
                fail (spec.fileStem + ": " + error);
                return;
            }
            auto& row = extra[(size_t) t];
            row.spec = spec; row.file = file; row.lengthSec = audio.getNumSamples() / request.render.sampleRate;
            row.score = rows[j].score; row.loudness = momentaryLoudness (audio, request.render.sampleRate);
            row.source = rows[j].file;
            audioMicros += (juce::int64) (row.lengthSec * 1.0e6);
            made[(size_t) t] = true;
        });
        for (size_t t = 0; t < extra.size(); ++t) if (made[t]) { rows.push_back (extra[t]); ++takesMade; }
    }

    // ------------------------------------------------------------ loudness matching
    // Every file was peak-normalised; now bring them to the same *perceived* loudness.
    // Gains only go down (never clip): the target is set so ~80% reach it exactly and
    // the quietest few stay at full level.
    juce::String loudnessSummary;
    if (request.render.loudnessMatch && rows.size() > 1 && ! threadShouldExit())
    {
        post ((int) rows.size(), (int) rows.size(), "Matching loudness ...");
        std::vector<float> levels;
        for (auto& r : rows) levels.push_back (r.loudness);
        std::sort (levels.begin(), levels.end());
        auto target = levels[(size_t) (0.2 * (double) (levels.size() - 1))];

        int adjusted = 0, belowTarget = 0;
        for (auto& r : rows)
        {
            if (threadShouldExit()) break;
            auto gainDb = juce::jmin (0.0f, target - r.loudness);
            if (r.loudness < target - 0.5f) ++belowTarget;
            if (gainDb > -0.1f) continue;
            juce::String error;
            if (! applyGainToWav (r.file, gainDb, r.spec.rootNote, error)) fail ("loudness: " + error);
            else { r.gainDb = gainDb; r.loudness += gainDb; ++adjusted; }
        }
        loudnessSummary << "Loudness matched to " << juce::String (target, 1) << " LUFS (" << adjusted << " files turned down"
                        << (belowTarget > 0 ? ", " + juce::String (belowTarget) + " naturally quieter ones left at full level" : juce::String()) << ")";
    }

    // ------------------------------------------------------------ manifest + specs
    int written = 0;
    juce::String manifest = manifestHeader();
    juce::Array<juce::var> specList;
    for (auto& row : rows)
    {
        ++written;
        auto similar = row.similarTo >= 0 && row.similarDb < 2.0f
                     ? "#" + juce::String (rows[(size_t) row.similarTo].spec.pad > 0 ? rows[(size_t) row.similarTo].spec.pad : rows[(size_t) row.similarTo].spec.index)
                     : juce::String();
        manifest << manifestRow (folder, row.spec, row.file, row.lengthSec, row.score, row.shift, row.loudness, row.gainDb,
                                 builtIn != nullptr, similar, row.similarDb, row.source);
        auto entry = specToVar (row.spec);
        entry.getDynamicObject()->setProperty ("file", row.file.getRelativePathFrom (folder).replaceCharacter ('\\', '/'));
        entry.getDynamicObject()->setProperty ("register", row.shift);
        specList.add (entry);
    }

    if (request.appendOnly)
    {
        auto mf = folder.getChildFile ("_manifest.csv");
        if (mf.existsAsFile()) mf.appendText (manifest.fromFirstOccurrenceOf ("\n", false, false));
        else mf.replaceWithText (manifest);
        auto old = juce::JSON::parse (folder.getChildFile ("_specs.json"));
        juce::Array<juce::var> merged;
        if (old.isArray()) merged.addArray (*old.getArray());
        merged.addArray (specList);
        folder.getChildFile ("_specs.json").replaceWithText (juce::JSON::toString (juce::var (merged), true));
    }
    else
    {
        folder.getChildFile ("_manifest.csv").replaceWithText (manifest);
        folder.getChildFile ("_specs.json").replaceWithText (juce::JSON::toString (juce::var (specList), true));
        folder.getChildFile ("_MPC_Setup.txt").replaceWithText (mpcSetupSheet());
    }

    // Everything needed to reproduce the batch.
    auto* info = new juce::DynamicObject();
    info->setProperty ("instrument", request.instrumentName);
    info->setProperty ("style", request.style.toVar());
    info->setProperty ("key", request.plan.key);
    info->setProperty ("scale", request.plan.scale.isNotEmpty() ? request.plan.scale : request.style.scale);
    info->setProperty ("seed", request.plan.seed);
    info->setProperty ("count", picking ? request.smartPickTarget : request.plan.count);
    info->setProperty ("smartPickCandidates", picking ? total : 0);
    juce::Array<juce::var> regs;
    for (auto r : request.registers) regs.add (r);
    info->setProperty ("registers", regs);
    info->setProperty ("loudnessMatch", request.render.loudnessMatch);
    info->setProperty ("roundRobins", takes);
    info->setProperty ("variety", request.variety);
    info->setProperty ("sampleRate", request.render.sampleRate);
    info->setProperty ("bitDepth", request.render.bitDepth);
    info->setProperty ("fixedRoot", request.plan.fixedRoot >= 0 ? midiNoteName (request.plan.fixedRoot) : juce::String ("free"));
    info->setProperty ("engine", request.plan.engine.isNotEmpty() ? request.plan.engine : request.style.engine);
    info->setProperty ("maxLength", request.render.trimToSec);
    info->setProperty ("created", juce::Time::getCurrentTime().toISO8601 (true));
    if (! request.appendOnly || ! folder.getChildFile ("_batch.json").existsAsFile())
        folder.getChildFile ("_batch.json").replaceWithText (juce::JSON::toString (juce::var (info)));

    elapsed = (juce::Time::getMillisecondCounterHiRes() - started) / 1000.0;

    if (threadShouldExit())
    {
        finish (false, "Cancelled after " + juce::String (written) + " samples.");
        return;
    }

    post (total, total, "Done");

    juce::String summary;
    summary << "Wrote " << (int) originals << " of " << (picking ? request.smartPickTarget : total) << " samples"
            << (rows.size() - (size_t) takesMade > originals
                    ? " (+" + juce::String ((int) (rows.size() - (size_t) takesMade - originals)) + " in other registers"
                      + (outOfRange > 0 ? ", " + juce::String (outOfRange.load()) + " skipped as too low / high to be useful" : juce::String()) + ")"
                    : juce::String())
            << (takesMade > 0 ? " (+" + juce::String (takesMade) + " round-robin takes in Round_Robin)" : juce::String())
            << " in " << juce::String (elapsed, 1) << " s to " << folder.getFullPathName();
    if (pickSummary.isNotEmpty()) summary << "\n" << pickSummary;
    if (varietySummary.isNotEmpty()) summary << "\n" << varietySummary;
    if (loudnessSummary.isNotEmpty()) summary << "\n" << loudnessSummary;
    if (! failures.isEmpty())
        summary << "\n" << failures.size() << " failed:\n  " << failures.joinIntoString ("\n  ");

    finish (failures.isEmpty(), summary);
}

void BatchJob::runRegistersOnly()
{
    auto finish = [this] (bool ok, const juce::String& summary)
    {
        if (auto cb = onFinished)
            juce::MessageManager::callAsync ([cb, ok, summary] { cb (ok, summary); });
    };
    auto fail = [this] (const juce::String& msg) { const juce::ScopedLock sl (failureLock); failures.add (msg); };
    const auto& folder = request.outputFolder;

    std::vector<int> shifts;
    for (auto s : request.registers) if (s != 0 && std::find (shifts.begin(), shifts.end(), s) == shifts.end()) shifts.push_back (s);
    const auto n = specs.size();
    const int jobs = (int) (shifts.size() * n);
    juce::StringArray manifestLines;
    juce::CriticalSection lines;
    std::atomic<int> made { 0 }, skipped { 0 };

    renderParallel (jobs, [&] (int t, OfflineRenderer& renderer, juce::AudioProcessor& instrument)
    {
        auto shift = shifts[(size_t) t / n];
        auto j = (size_t) t % n;
        auto& original = request.existingFiles[j];
        auto name = registerFolderName (shift);
        post (progressDone++, jobs, "Octave copy " + name + ": " + specs[j].fileStem);

        SampleSpec moved;
        if (! transposeSpec (specs[j], shift, moved)) { ++skipped; return; }
        if (auto* b = dynamic_cast<BuiltInSynth*> (&instrument))
        {
            b->reset();
            Engine e;
            if (parseEngine (moved.engine, e)) b->setEngine (e);
            b->setVariation (moved.variation);
        }
        juce::AudioBuffer<float> audio;
        juce::String error;
        auto rel = original.getParentDirectory() == folder ? juce::String() : original.getParentDirectory().getRelativePathFrom (folder);
        auto file = folder.getChildFile (name).getChildFile (rel).getChildFile (moved.fileStem + ".wav");
        file.getParentDirectory().createDirectory();
        if (! renderer.render (moved, audio, error)
            || ! OfflineRenderer::writeWav (file, audio, request.render.sampleRate, request.render.bitDepth, moved.rootNote, error))
        {
            fail (moved.fileStem + " (" + name + "): " + error);
            return;
        }

        // match the copy's loudness to the original it belongs to (never louder)
        float loud = momentaryLoudness (audio, request.render.sampleRate), gain = 0;
        if (request.render.loudnessMatch)
        {
            juce::AudioFormatManager fm;
            fm.registerBasicFormats();
            if (std::unique_ptr<juce::AudioFormatReader> reader { fm.createReaderFor (original) })
            {
                juce::AudioBuffer<float> o ((int) reader->numChannels, (int) reader->lengthInSamples);
                reader->read (&o, 0, o.getNumSamples(), 0, true, true);
                gain = juce::jmin (0.0f, momentaryLoudness (o, reader->sampleRate) - loud);
                if (gain < -0.1f && applyGainToWav (file, gain, moved.rootNote, error)) loud += gain;
            }
        }
        audioMicros += (juce::int64) (audio.getNumSamples() / request.render.sampleRate * 1.0e6);
        const juce::ScopedLock sl (lines);
        manifestLines.add (manifestRow (folder, moved, file, audio.getNumSamples() / request.render.sampleRate, -1, shift, loud, gain,
                                        dynamic_cast<BuiltInSynth*> (&proc) != nullptr, {}, 0, original));
        ++made;
    });

    // append to the batch's manifest so Export Keepers / your notes see the copies
    auto manifest = folder.getChildFile ("_manifest.csv");
    if (manifest.existsAsFile()) manifest.appendText (manifestLines.joinIntoString (""));

    juce::String summary;
    summary << "Made " << made.load() << " octave copies of " << (int) n << " keepers"
            << (skipped > 0 ? " (" + juce::String (skipped.load()) + " skipped as too low / high)" : juce::String())
            << " in " << folder.getFullPathName();
    if (! failures.isEmpty()) summary << "\n" << failures.size() << " failed:\n  " << failures.joinIntoString ("\n  ");
    finish (failures.isEmpty(), summary);
}
}
