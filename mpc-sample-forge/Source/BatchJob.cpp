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
    juce::String manifest = "file,type,name,voicing,recipe,root_note,root_midi,midi_notes,velocities,hold_seconds,length_seconds\n";

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
                Recipe r;
                if (parseRecipe (spec.recipe, r)) builtIn->setRecipe (r);
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

            manifest << csv (file.getRelativePathFrom (folder).replaceCharacter ('\\', '/')) << "," << spec.kind << ","
                     << csv (spec.label) << "," << spec.voicing << "," << (builtIn != nullptr ? spec.recipe : juce::String ("-")) << ","
                     << midiNoteName (spec.rootNote) << "," << spec.rootNote << "," << csv (notes.joinIntoString (" ")) << ","
                     << csv (vels.joinIntoString (" ")) << "," << juce::String (spec.holdSec(), 3) << ","
                     << juce::String (audio.getNumSamples() / request.render.sampleRate, 3) << "\n";
        }
    }

    folder.getChildFile ("_manifest.csv").replaceWithText (manifest);

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
    info->setProperty ("recipe", request.plan.recipe.isNotEmpty() ? request.plan.recipe : request.style.recipe);
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
