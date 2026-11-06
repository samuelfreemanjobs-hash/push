#include "CommandLine.h"
#include "BuiltInSynth.h"
#include "Diagnostics.h"
#include "Analysis.h"
#include <iostream>

namespace forge
{
juce::String cliHelpText()
{
    juce::String s;
    s << "MPC Sample Forge - render one-shot keys & chords from a VST3 instrument\n\n"
      << "Usage:\n"
      << "  mpcforge [options]                     render a batch from the command line\n"
      << "  \"MPC Sample Forge\" --cli [options]   same, via the app (macOS / Linux)\n\n"
      << "Example:\n"
      << "  mpcforge --plugin \"C:/Program Files/Common Files/VST3/Keyscape.vst3\" --sound rhodes.forgesound \\\n"
      << "           --style neo-soul --key Eb --count 100 --out D:/MPC/NeoSoulKeys\n\n"
      << "Instrument (one of):\n"
      << "  --plugin <file.vst3>      VST3 instrument to load\n"
      << "  --sound <file.forgesound> sound saved from the app (plugin + its settings)\n"
      << "  --builtin                 use the built-in synth (default if nothing given)\n"
      << "  --engine <name>           override the built-in synth engine: epiano, bell, organ,\n"
      << "                            pluck, choir, supersaw, epbell, triangle, cowbell,\n"
      << "                            comboorgan, harp, brass or mixed (default: the style's)\n\n"
      << "Recipe guide (one-shot recipes with trim, envelope and FX baked in):\n"
      << "  --recipe <id>             same as --style; recipes: jetlife-ep-bell, polyester-pluck,\n"
      << "                            memphis-organ, phonk-cowbell, zaytoven-organ, luxury-harp,\n"
      << "                            screwed-choir, trap-brass, rhodes-bass\n"
      << "                            kits: westcoast-kit, memphis-kit, atlanta-kit, guide-palette\n"
      << "  --tempo <bpm>             tempo for synced delays (default 90)\n\n"
      << "16 Levels (MPC drum-program one-shots):\n"
      << "  --16levels                same as --root recipe --banks\n"
      << "  --root <C3|Eb2|60|recipe> build every sample on this note, so 16 Levels > Tune\n"
      << "                            spreads it chromatically from a known pitch ('recipe' =\n"
      << "                            each recipe's own root, C3 for styles without one)\n"
      << "  --banks                   write files in folders of 16 (Bank_A, Bank_B ...)\n"
      << "  --max-length <seconds>    cut each one-shot to this length with a fade\n"
      << "                            (default: the style's end point; 0 = no cap)\n\n"
      << "Batch:\n"
      << "  --style <id>              built-in style (see --list-styles), default neo-soul\n"
      << "  --style-file <file.json>  your own style (see --dump-style for the format)\n"
      << "  --count <n>               number of samples, default 100\n"
      << "  --key <C|F#|Bb|random>    lock every chord to one key, default random\n"
      << "  --scale <name>            major, minor, dorian, ... (default: the style's)\n"
      << "  --seed <n>                same seed = same batch, default random\n"
      << "  --prefix <text>           file name prefix, default the style name\n"
      << "  --out <folder>            output folder, default ~/MPC Sample Forge/<style>\n\n"
      << "Smart Pick (better selections):\n"
      << "  --smart-pick              render 3x the samples, score them (pitch clarity, mud,\n"
      << "                            harshness, clicks) and keep only the best, skipping\n"
      << "                            near-duplicates. Writes _smart_pick.csv with every decision.\n"
      << "  --pick-factor <2-6>       candidates per kept sample (default 3)\n\n"
      << "MPC Sample friendly output:\n"
      << "  --registers <list>        also render every kept sample in other registers, in\n"
      << "                            semitones, e.g. -12,12 (= an octave down and up) or\n"
      << "                            'octaves' (same as -12,12). Folders: Octave_Down, Octave_Up\n"
      << "  --loudness                level all files to the same perceived loudness (LUFS)\n"
      << "                            instead of the same peak, so pads play evenly\n"
      << "  --short-names             014_EPBell_C3 instead of JetLifeEPBell_C3_014_Key_C3\n\n"
      << "Audio:\n"
      << "  --rate <44100|48000>      sample rate, default 44100\n"
      << "  --bits <16|24>            bit depth, default 24\n"
      << "  --no-normalize            keep the plugin's level (default: normalise to -1 dBFS)\n"
      << "  --tail <seconds>          max release tail after note-off, default 6\n"
      << "  --warmup <seconds>        time for sample-based plugins to load, default 1.5\n\n"
      << "Info:\n"
      << "  --list-styles  --list-plugins  --dump-style <id>  --plan-only  --version  --help\n"
      << "  --analyse <file.wav>      print loudness (LUFS), peak and the Smart Pick measurements\n"
      << "  --diagnose [--plugin X.vst3]  system report + self-test (renders every engine and\n"
      << "                            recipe, checks WAV writing, test-renders the plugin).\n"
      << "                            Exit code 3 if anything failed. Log: " << Log::file().getFullPathName() << "\n\n"
      << "Example (16 Levels kit with the built-in synth):\n"
      << "  mpcforge --style one-shot-palette --16levels --count 64 --out ~/MPC/Palette\n"
      << "Example (a recipe from the guide):\n"
      << "  mpcforge --recipe jetlife-ep-bell --16levels --count 32 --out ~/MPC/JetLifeBell\n";
    return s;
}

static void out (const juce::String& s) { std::cout << s << std::endl; }

bool CliRunner::wantsCli (const juce::StringArray& args)
{
    for (auto a : { "--cli", "--help", "-h", "--list-styles", "--list-plugins", "--dump-style", "--diagnose", "--version", "--analyse" })
        if (args.contains (a))
            return true;
    return false;
}

static juce::String option (const juce::StringArray& args, const juce::String& name, const juce::String& def = {})
{
    auto i = args.indexOf (name);
    return (i >= 0 && i + 1 < args.size()) ? args[i + 1] : def;
}

void CliRunner::fail (const juce::String& msg)
{
    Log::write ("error: " + msg);
    std::cerr << "error: " << msg << std::endl;
    onDone (1);
}

void CliRunner::start (const juce::StringArray& args, std::function<void (int)> done)
{
    onDone = std::move (done);

    if (args.contains ("--help") || args.contains ("-h"))
    {
        out (cliHelpText());
        return onDone (0);
    }

    if (args.contains ("--version"))
    {
        out ("mpcforge " FORGE_VERSION_STRING " (JUCE " + juce::SystemStats::getJUCEVersion() + ")");
        return onDone (0);
    }

    if (args.contains ("--diagnose"))
        return runDiagnose (args);

    if (args.contains ("--analyse"))
    {
        auto f = juce::File::getCurrentWorkingDirectory().getChildFile (option (args, "--analyse"));
        juce::AudioFormatManager fm;
        fm.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> reader (fm.createReaderFor (f));
        if (reader == nullptr) return fail ("cannot read " + f.getFullPathName());
        juce::AudioBuffer<float> audio ((int) reader->numChannels, (int) reader->lengthInSamples);
        reader->read (&audio, 0, audio.getNumSamples(), 0, true, true);
        auto root = reader->metadataValues.getValue ("MidiUnityNote", "60").getIntValue();
        auto feat = analyseSample (audio, reader->sampleRate, root, true);
        out (f.getFileName());
        out ("  loudness   " + juce::String (momentaryLoudness (audio, reader->sampleRate), 2) + " LUFS (loudest 400 ms)");
        out ("  peak       " + juce::String (juce::Decibels::gainToDecibels (feat.peak), 2) + " dBFS");
        out ("  length     " + juce::String (feat.lengthSec, 3) + " s, " + juce::String (reader->sampleRate, 0) + " Hz, "
             + juce::String (reader->bitsPerSample) + "-bit, root " + midiNoteName (root));
        out ("  centroid   " + juce::String (juce::roundToInt (feat.centroidHz)) + " Hz, mud " + juce::String (feat.mudShare, 2)
             + ", harsh " + juce::String (feat.harshShare, 2) + ", pitch clarity " + juce::String (feat.harmonicity, 2));
        return onDone (0);
    }

    if (args.contains ("--list-styles"))
    {
        for (auto& s : builtInStyles())
            out (s.id.paddedRight (' ', 18) + s.name.paddedRight (' ', 18) + s.description);
        return onDone (0);
    }

    if (args.contains ("--dump-style"))
    {
        auto* s = findBuiltInStyle (option (args, "--dump-style", "neo-soul"));
        if (s == nullptr) return fail ("unknown style (see --list-styles)");
        out (juce::JSON::toString (s->toVar()));
        return onDone (0);
    }

    if (args.contains ("--list-plugins"))
    {
        auto found = host.findInstalledVST3();
        for (auto& p : found) out (p);
        if (found.isEmpty()) out ("(no VST3 plugins found in the standard folders)");
        return onDone (0);
    }

    // ------------------------------------------------------------ the style
    BatchRequest req;
    juce::String error;

    if (args.contains ("--style-file"))
    {
        if (! loadStyleFile (juce::File::getCurrentWorkingDirectory().getChildFile (option (args, "--style-file")), req.style, error))
            return fail (error);
    }
    else
    {
        auto name = option (args, "--style", option (args, "--recipe", "neo-soul"));
        auto* s = findBuiltInStyle (name);
        if (s == nullptr) return fail ("unknown style '" + name + "' (see --list-styles)");
        req.style = *s;
    }

    req.plan.count  = option (args, "--count", "100").getIntValue();
    req.plan.key    = option (args, "--key", "random");
    req.plan.scale  = option (args, "--scale");
    req.plan.prefix = option (args, "--prefix");
    req.plan.engine = option (args, "--engine").toLowerCase();
    if (req.plan.engine.isNotEmpty() && req.plan.engine != "mixed")
    {
        Engine r;
        if (! parseEngine (req.plan.engine, r))
            return fail ("unknown --engine (use " + engineIds().joinIntoString (", ") + " or mixed)");
    }

    auto rootText = option (args, "--root", args.contains ("--16levels") ? "recipe" : "");
    if (rootText.equalsIgnoreCase ("recipe") || rootText.equalsIgnoreCase ("style"))
    {
        req.plan.fixedRoot = PlanOptions::styleRoot;
    }
    else if (rootText.isNotEmpty() && ! rootText.equalsIgnoreCase ("free"))
    {
        req.plan.fixedRoot = parseMidiNote (rootText);
        if (req.plan.fixedRoot < 0) return fail ("bad --root '" + rootText + "' (use e.g. C3, Eb2 or 60)");
    }
    req.splitBanks = args.contains ("--banks") || args.contains ("--16levels");

    req.plan.seed   = args.contains ("--seed") ? option (args, "--seed").getLargeIntValue()
                                               : juce::Random::getSystemRandom().nextInt64() & 0xffffffff;

    req.render.sampleRate = option (args, "--rate", "44100").getDoubleValue();
    req.render.bitDepth   = option (args, "--bits", "24").getIntValue();
    req.render.normalise  = ! args.contains ("--no-normalize");
    req.render.maxTailSec = option (args, "--tail", "6").getDoubleValue();
    req.render.warmUpSec  = option (args, "--warmup", "1.5").getDoubleValue();
    req.render.loudnessMatch = args.contains ("--loudness");
    req.plan.shortNames = args.contains ("--short-names");
    if (args.contains ("--registers"))
    {
        auto list = option (args, "--registers");
        if (list.equalsIgnoreCase ("octaves")) list = "-12,12";
        for (auto& t : juce::StringArray::fromTokens (list, ",", ""))
        {
            auto v = t.trim().getIntValue();
            if (t.trim().isEmpty() || std::abs (v) > 36 || (v == 0 && t.trim() != "0"))
                return fail ("bad --registers value '" + t + "' (use semitones like -12,12)");
            if (v != 0) req.registers.push_back (v);
        }
    }
    req.render.trimToSec  = args.contains ("--max-length") ? option (args, "--max-length").getDoubleValue() : -1.0;

    juce::String pluginPath = option (args, "--plugin");
    SoundState sound;
    bool haveSound = false;

    if (args.contains ("--sound"))
    {
        if (! sound.load (juce::File::getCurrentWorkingDirectory().getChildFile (option (args, "--sound")), error))
            return fail (error);
        haveSound = true;
        if (pluginPath.isEmpty()) pluginPath = sound.pluginPath;
    }

    const bool useBuiltIn = pluginPath.isEmpty() || args.contains ("--builtin");
    req.plan.builtIn = useBuiltIn;

    if (req.render.sampleRate < 8000 || req.render.sampleRate > 192000) return fail ("bad --rate");
    if (req.render.bitDepth != 16 && req.render.bitDepth != 24) return fail ("--bits must be 16 or 24");

    // Smart Pick: plan more candidates than needed, keep the best after rendering
    const int wanted = juce::jlimit (1, 2000, req.plan.count);
    if (args.contains ("--smart-pick") || args.contains ("--pick-factor"))
    {
        auto factor = juce::jlimit (2, 6, option (args, "--pick-factor", "3").getIntValue());
        req.plan.count = juce::jmin (2000, wanted * factor);
        req.smartPickTarget = wanted;
    }

    auto specs = planSamples (req.style, req.plan, error);
    if (specs.empty()) return fail (error);
    if (req.smartPickTarget > 0)
    {
        if ((int) specs.size() <= wanted)
        {
            std::cerr << "warning: only " << specs.size() << " distinct candidates possible - Smart Pick has nothing to choose from" << std::endl;
            req.smartPickTarget = 0;
        }
        error = {}; // fewer candidates than asked is fine when picking
    }
    req.plan.count = wanted;
    if (error.isNotEmpty()) std::cerr << "warning: " << error << std::endl;

    if (args.contains ("--tempo"))
        for (auto& sp : specs)
            sp.processing.tempo = juce::jlimit (30.0, 300.0, option (args, "--tempo").getDoubleValue());

    if (args.contains ("--plan-only"))
    {
        for (auto& s : specs) out (describeSpec (s));
        return onDone (0);
    }

    req.outputFolder = args.contains ("--out")
        ? juce::File::getCurrentWorkingDirectory().getChildFile (option (args, "--out"))
        : juce::File::getSpecialLocation (juce::File::userHomeDirectory)
              .getChildFile ("MPC Sample Forge").getChildFile (safeFileName (req.style.name));

    // -------------------------------------------------------- the instrument
    if (! useBuiltIn)
    {
        out ("Loading " + pluginPath + " ...");
        auto plugin = host.load (pluginPath, req.render.sampleRate, req.render.blockSize, error);
        if (plugin == nullptr) return fail (error);
        if (haveSound && sound.state.getSize() > 0)
            plugin->setStateInformation (sound.state.getData(), (int) sound.state.getSize());
        instrument = std::move (plugin);
    }
    else
    {
        instrument = std::make_unique<BuiltInSynth>();
    }

    req.instrumentName = instrument->getName();
    out ("Instrument: " + req.instrumentName);
    out ("Style:      " + req.style.name + "   key: " + req.plan.key + "   seed: " + juce::String (req.plan.seed)
         + (req.plan.fixedRoot >= 0 ? "   16 Levels root: " + midiNoteName (req.plan.fixedRoot)
            : req.plan.fixedRoot == PlanOptions::styleRoot ? juce::String ("   16 Levels root: per recipe") : juce::String())
         + (useBuiltIn ? "   engine: " + (req.plan.engine.isNotEmpty() ? req.plan.engine : req.style.engine) : juce::String()));
    out ("Output:     " + req.outputFolder.getFullPathName());
    if (req.smartPickTarget > 0)
        out ("Smart Pick: rendering " + juce::String ((int) specs.size()) + " candidates, keeping the best " + juce::String (req.smartPickTarget));

    Log::write ("batch: " + req.instrumentName + " / " + req.style.id + " / " + juce::String ((int) specs.size())
                + " samples -> " + req.outputFolder.getFullPathName());
    job = std::make_unique<BatchJob> (*instrument, req, std::move (specs));
    job->onProgress = [] (int doneCount, int total, const juce::String& msg)
    {
        out ("[" + juce::String (doneCount).paddedLeft (' ', 3) + "/" + juce::String (total) + "] " + msg);
    };
    job->onFinished = [this] (bool ok, const juce::String& summary)
    {
        out (summary);
        Log::write (summary);
        job.reset();
        instrument.reset();
        onDone (ok ? 0 : 2);
    };
    job->startThread();
}

void CliRunner::runDiagnose (const juce::StringArray& args)
{
    juce::String error;
    auto pluginPath = option (args, "--plugin");
    if (pluginPath.isNotEmpty())
    {
        out ("Loading " + pluginPath + " ...");
        auto plugin = host.load (pluginPath, 44100.0, 256, error);
        if (plugin == nullptr) out ("[FAIL] Plugin load           " + error);
        else instrument = std::move (plugin);
    }

    out (systemReport (host, nullptr, instrument.get(), pluginPath));
    out ("SELF-TEST");

    struct DiagThread final : juce::Thread
    {
        DiagThread (CliRunner& o) : juce::Thread ("diagnose"), owner (o) {}
        void run() override
        {
            DiagSummary summary;
            runSelfTest ([&summary] (const DiagResult& r) { summary.add (r); std::cout << r.toString() << std::endl; },
                         owner.instrument.get(), owner.diagCancel);
            std::cout << "\nRESULT: " << summary.toString() << std::endl;
            Log::write ("self-test: " + summary.toString());
            auto code = summary.fail > 0 ? 3 : 0;
            auto* o = &owner;
            juce::MessageManager::callAsync ([o, code] { o->instrument.reset(); o->onDone (code); });
        }
        CliRunner& owner;
    };

    diagThread = std::make_unique<DiagThread> (*this);
    diagThread->startThread();
}
}