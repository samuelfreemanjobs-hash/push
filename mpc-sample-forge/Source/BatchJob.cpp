#include "BatchJob.h"
#include "BuiltInSynth.h"

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

void BatchJob::run()
{
    auto finish = [this] (bool ok, const juce::String& summary)
    {
        if (auto cb = onFinished)
            juce::MessageManager::callAsync ([cb, ok, summary] { cb (ok, summary); });
    };

    auto& folder = request.outputFolder;
    if (! folder.createDirectory())
    {
        finish (false, "Could not create output folder " + folder.getFullPathName());
        return;
    }

    const int total = (int) specs.size();
    post (0, total, "Preparing " + request.instrumentName + " ...");

    int written = 0;
    juce::StringArray failures;
    juce::String manifest = "file,recipe,type,name,voicing,engine,root_note,root_midi,midi_notes,velocities,hold_seconds,length_seconds\n";

    {
        OfflineRenderer renderer (proc, request.render);
        renderer.warmUp();

        for (int i = 0; i < total && ! threadShouldExit(); ++i)
        {
            auto& spec = specs[(size_t) i];
            post (i, total, "Rendering " + spec.fileStem);

            juce::AudioBuffer<float> audio;
            juce::String error;
            auto target = folder;
            if (request.splitBanks)
            {
                auto bank = i / 16;
                target = folder.getChildFile ("Bank_" + juce::String::charToString ((juce::juce_wchar) ('A' + bank % 26))
                                              + (bank >= 26 ? juce::String (bank / 26 + 1) : juce::String()));
                target.createDirectory();
            }
            auto file = target.getChildFile (spec.fileStem + ".wav");

            auto* builtIn = dynamic_cast<BuiltInSynth*> (&proc);
            if (builtIn != nullptr)
            {
                Engine r;
                if (parseEngine (spec.engine, r)) builtIn->setEngine (r);
                builtIn->setVariation (spec.variation);
            }

            if (! renderer.render (spec, audio, error)
                || ! OfflineRenderer::writeWav (file, audio, request.render.sampleRate, request.render.bitDepth, spec.rootNote, error))
            {
                failures.add (spec.fileStem + ": " + error);
                continue;
            }

            ++written;
            juce::StringArray notes, vels;
            for (auto& n : spec.notes) { notes.add (juce::String (n.note)); vels.add (juce::String (n.velocity)); }

            manifest << csv (file.getRelativePathFrom (folder).replaceCharacter ('\\', '/')) << "," << spec.tag << "," << spec.kind << ","
                     << csv (spec.label) << "," << spec.voicing << "," << (builtIn != nullptr ? spec.engine : juce::String ("-")) << ","
                     << midiNoteName (spec.rootNote) << "," << spec.rootNote << "," << csv (notes.joinIntoString (" ")) << ","
                     << csv (vels.joinIntoString (" ")) << "," << juce::String (spec.holdSec(), 3) << ","
                     << juce::String (audio.getNumSamples() / request.render.sampleRate, 3) << "\n";
        }
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
    info->setProperty ("count", request.plan.count);
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
    summary << "Wrote " << written << " of " << total << " samples to " << folder.getFullPathName();
    if (! failures.isEmpty())
        summary << "\n" << failures.size() << " failed:\n  " << failures.joinIntoString ("\n  ");

    finish (failures.isEmpty(), summary);
}
}
