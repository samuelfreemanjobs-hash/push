#include "BatchJob.h"
#include "BuiltInSynth.h"
#include "Analysis.h"

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

    if (! request.registers.empty())
    {
        t << "Register sets: every sample is also in ";
        juce::StringArray names;
        for (auto r : request.registers) if (r != 0) names.add ((r > 0 ? "+" : "") + juce::String (r) + " st");
        t << names.joinIntoString (", ") << " (folders Octave_Down / Octave_Up ...).\n"
          << "  With 16 Levels > Tune, load the register closest to the notes you play: big\n"
          << "  re-pitches make one-shots grainy / short (up) or dull / long (down).\n\n";
    }
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

juce::File BatchJob::bankFolder (int index, const juce::File& folder) const
{
    if (! request.splitBanks) return folder;
    auto bank = index / 16;
    return folder.getChildFile ("Bank_" + juce::String::charToString ((juce::juce_wchar) ('A' + bank % 26))
                                + (bank >= 26 ? juce::String (bank / 26 + 1) : juce::String()));
}

void BatchJob::run()
{
    auto finish = [this] (bool ok, const juce::String& summary)
    {
        if (auto cb = onFinished)
            juce::MessageManager::callAsync ([cb, ok, summary] { cb (ok, summary); });
    };

    const auto& folder = request.outputFolder;
    if (! folder.createDirectory())
    {
        finish (false, "Could not create output folder " + folder.getFullPathName());
        return;
    }

    const int total = (int) specs.size();
    const bool picking = request.smartPickTarget > 0 && request.smartPickTarget < total;
    post (0, total, "Preparing " + request.instrumentName + " ...");

    // With Smart Pick every candidate is rendered into a scratch folder first.
    auto workDir = picking ? folder.getChildFile (".candidates") : folder;
    if (picking) { workDir.deleteRecursively(); workDir.createDirectory(); }

    struct Rendered { bool ok = false; juce::File file; double lengthSec = 0; float loudness = -100; SampleFeatures features; };
    std::vector<Rendered> rendered ((size_t) total);
    juce::StringArray failures;
    auto* builtIn = dynamic_cast<BuiltInSynth*> (&proc);

    {
        OfflineRenderer renderer (proc, request.render);
        renderer.warmUp();

        for (int i = 0; i < total && ! threadShouldExit(); ++i)
        {
            auto& spec = specs[(size_t) i];
            post (i, total, (picking ? "Rendering candidate " + juce::String (i + 1) + "/" + juce::String (total) + ": " : juce::String ("Rendering "))
                            + spec.fileStem);

            if (builtIn != nullptr)
            {
                Engine r;
                if (parseEngine (spec.engine, r)) builtIn->setEngine (r);
                builtIn->setVariation (spec.variation);
            }

            juce::AudioBuffer<float> audio;
            juce::String error;
            auto file = picking ? workDir.getChildFile ("cand_" + juce::String (i).paddedLeft ('0', 4) + ".wav")
                                : bankFolder (i).getChildFile (spec.fileStem + ".wav");
            file.getParentDirectory().createDirectory();

            if (! renderer.render (spec, audio, error)
                || ! OfflineRenderer::writeWav (file, audio, request.render.sampleRate, request.render.bitDepth, spec.rootNote, error))
            {
                failures.add (spec.fileStem + ": " + error);
                continue;
            }

            auto& r = rendered[(size_t) i];
            r.ok = true;
            r.file = file;
            r.lengthSec = audio.getNumSamples() / request.render.sampleRate;
            r.loudness = momentaryLoudness (audio, request.render.sampleRate);
            if (picking)
                r.features = analyseSample (audio, request.render.sampleRate, spec.rootNote, spec.kind == "Key" || spec.kind == "Octave");
        }
    }

    // ------------------------------------------------------------ Smart Pick
    std::vector<int> origin;          // final position -> candidate index
    std::vector<PickVerdict> verdicts;
    juce::String pickSummary;

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
        verdicts = smartPick (cands, request.smartPickTarget);

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
            if (! r.file.moveFileTo (dest)) failures.add ("could not move " + r.file.getFileName() + " to " + dest.getFullPathName());
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
    }

    // ------------------------------------------------------------ the final list of files
    struct Row { SampleSpec spec; juce::File file; double lengthSec = 0; int score = -1; int shift = 0; float loudness = -100; float gainDb = 0; };
    std::vector<Row> rows;
    for (size_t j = 0; j < specs.size() && j < origin.size(); ++j)
    {
        auto& r = rendered[(size_t) origin[j]];
        if (! r.ok) continue;
        rows.push_back ({ specs[j], r.file, r.lengthSec,
                          verdicts.empty() ? -1 : juce::roundToInt (verdicts[(size_t) origin[j]].score), 0, r.loudness, 0.0f });
    }
    const auto originals = rows.size();

    // ------------------------------------------------------------ register sets
    // The same sounds an octave (or two) away, so 16 Levels > Tune never has to
    // stretch a sample far: tune from the nearest register instead.
    std::vector<int> shifts;
    int outOfRange = 0;
    for (auto s : request.registers) if (s != 0 && std::find (shifts.begin(), shifts.end(), s) == shifts.end()) shifts.push_back (s);

    if (! shifts.empty() && ! threadShouldExit())
    {
        OfflineRenderer renderer (proc, request.render);
        renderer.warmUp();
        const int jobs = (int) (shifts.size() * originals);
        int done = 0;
        outOfRange = 0;
        for (auto shift : shifts)
        {
            auto octaves = std::abs (shift) / 12;
            auto name = (shift % 12 == 0 ? (octaves == 1 ? juce::String() : juce::String (octaves) + "_") + (octaves == 1 ? "Octave" : "Octaves")
                                         : juce::String (std::abs (shift)) + "_Semitones")
                        + (shift < 0 ? "_Down" : "_Up");
            auto base = folder.getChildFile (name);

            for (size_t j = 0; j < originals && ! threadShouldExit(); ++j)
            {
                SampleSpec moved;
                post (done++, jobs, "Register " + name + ": " + rows[j].spec.fileStem);
                if (! transposeSpec (rows[j].spec, shift, moved)) { ++outOfRange; continue; } // e.g. a bass note an octave lower

                if (builtIn != nullptr)
                {
                    Engine e;
                    if (parseEngine (moved.engine, e)) builtIn->setEngine (e);
                    builtIn->setVariation (moved.variation); // same timbre, new register
                }

                juce::AudioBuffer<float> audio;
                juce::String error;
                auto file = bankFolder ((int) j, base).getChildFile (moved.fileStem + ".wav");
                file.getParentDirectory().createDirectory();
                if (! renderer.render (moved, audio, error)
                    || ! OfflineRenderer::writeWav (file, audio, request.render.sampleRate, request.render.bitDepth, moved.rootNote, error))
                {
                    failures.add (moved.fileStem + " (" + name + "): " + error);
                    continue;
                }
                rows.push_back ({ moved, file, audio.getNumSamples() / request.render.sampleRate, rows[j].score, shift,
                                  momentaryLoudness (audio, request.render.sampleRate), 0.0f });
            }
        }
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
        juce::WavAudioFormat wav;
        for (auto& r : rows)
        {
            if (threadShouldExit()) break;
            auto gainDb = juce::jmin (0.0f, target - r.loudness);
            if (r.loudness < target - 0.5f) ++belowTarget;
            if (gainDb > -0.1f) continue;

            std::unique_ptr<juce::AudioFormatReader> reader (wav.createReaderFor (r.file.createInputStream().release(), true));
            if (reader == nullptr) { failures.add ("loudness: cannot read " + r.file.getFileName()); continue; }
            juce::AudioBuffer<float> audio ((int) reader->numChannels, (int) reader->lengthInSamples);
            reader->read (&audio, 0, audio.getNumSamples(), 0, true, true);
            reader.reset();
            audio.applyGain (juce::Decibels::decibelsToGain (gainDb));
            juce::String error;
            if (! OfflineRenderer::writeWav (r.file, audio, request.render.sampleRate, request.render.bitDepth, r.spec.rootNote, error))
                failures.add ("loudness: " + error);
            else { r.gainDb = gainDb; r.loudness += gainDb; ++adjusted; }
        }
        loudnessSummary << "Loudness matched to " << juce::String (target, 1) << " LUFS (" << adjusted << " files turned down"
                        << (belowTarget > 0 ? ", " + juce::String (belowTarget) + " naturally quieter ones left at full level" : juce::String()) << ")";
    }

    // ------------------------------------------------------------ manifest
    int written = 0;
    juce::String manifest = "file,recipe,type,name,voicing,engine,root_note,root_midi,midi_notes,velocities,hold_seconds,length_seconds,score,register_semitones,loudness_lufs,gain_db\n";
    for (auto& row : rows)
    {
        auto& spec = row.spec;
        ++written;
        juce::StringArray notes, vels;
        for (auto& nn : spec.notes) { notes.add (juce::String (nn.note)); vels.add (juce::String (nn.velocity)); }

        manifest << csv (row.file.getRelativePathFrom (folder).replaceCharacter ('\\', '/')) << "," << spec.tag << "," << spec.kind << ","
                 << csv (spec.label) << "," << spec.voicing << "," << (builtIn != nullptr ? spec.engine : juce::String ("-")) << ","
                 << midiNoteName (spec.rootNote) << "," << spec.rootNote << "," << csv (notes.joinIntoString (" ")) << ","
                 << csv (vels.joinIntoString (" ")) << "," << juce::String (spec.holdSec(), 3) << ","
                 << juce::String (row.lengthSec, 3) << "," << (row.score >= 0 ? juce::String (row.score) : juce::String()) << ","
                 << row.shift << "," << juce::String (row.loudness, 1) << "," << juce::String (row.gainDb, 1) << "\n";
    }

    folder.getChildFile ("_manifest.csv").replaceWithText (manifest);
    folder.getChildFile ("_MPC_Setup.txt").replaceWithText (mpcSetupSheet());

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
    info->setProperty ("sampleRate", request.render.sampleRate);
    info->setProperty ("bitDepth", request.render.bitDepth);
    info->setProperty ("fixedRoot", request.plan.fixedRoot >= 0 ? midiNoteName (request.plan.fixedRoot) : juce::String ("free"));
    info->setProperty ("engine", request.plan.engine.isNotEmpty() ? request.plan.engine : request.style.engine);
    info->setProperty ("maxLength", request.render.trimToSec);
    info->setProperty ("created", juce::Time::getCurrentTime().toISO8601 (true));
    folder.getChildFile ("_batch.json").replaceWithText (juce::JSON::toString (juce::var (info)));

    if (threadShouldExit())
    {
        finish (false, "Cancelled after " + juce::String (written) + " samples.");
        return;
    }

    post (total, total, "Done");

    juce::String summary;
    summary << "Wrote " << (int) originals << " of " << (picking ? request.smartPickTarget : total) << " samples"
            << (rows.size() > originals ? " (+" + juce::String ((int) (rows.size() - originals)) + " in other registers"
                                          + (outOfRange > 0 ? ", " + juce::String (outOfRange) + " skipped as too low / high to be useful" : juce::String()) + ")"
                                        : juce::String())
            << " to " << folder.getFullPathName();
    if (pickSummary.isNotEmpty()) summary << "\n" << pickSummary;
    if (loudnessSummary.isNotEmpty()) summary << "\n" << loudnessSummary;
    if (! failures.isEmpty())
        summary << "\n" << failures.size() << " failed:\n  " << failures.joinIntoString ("\n  ");

    finish (failures.isEmpty(), summary);
}
}
