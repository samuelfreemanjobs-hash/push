#include "CommandLine.h"
#include "BuiltInSynth.h"
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
      << "  --recipe <name>           built-in synth engine: epiano, bell, organ, pluck, choir,\n"
      << "                            supersaw or mixed (default: the style's recipe)\n\n"
      << "16 Levels (MPC drum-program one-shots):\n"
      << "  --16levels                same as --root C3 --banks\n"
      << "  --root <C3|Eb2|60>        build every sample on this note, so 16 Levels > Tune\n"
      << "                            spreads it chromatically from a known pitch\n"
      << "  --banks                   write files in folders of 16 (Bank_A, Bank_B ...)\n"
      << "  --max-length <seconds>    cut each one-shot to this length with a fade\n"
      << "                            (default: the style's maxLength; 0 = no cap)\n\n"
      << "Batch:\n"
      << "  --style <id>              built-in style (see --list-styles), default neo-soul\n"
      << "  --style-file <file.json>  your own style (see --dump-style for the format)\n"
      << "  --count <n>               number of samples, default 100\n"
      << "  --key <C|F#|Bb|random>    lock every chord to one key, default random\n"
      << "  --scale <name>            major, minor, dorian, ... (default: the style's)\n"
      << "  --seed <n>                same seed = same batch, default random\n"
      << "  --prefix <text>           file name prefix, default the style name\n"
      << "  --out <folder>            output folder, default ~/MPC Sample Forge/<style>\n\n"
      << "Audio:\n"
      << "  --rate <44100|48000>      sample rate, default 44100\n"
      << "  --bits <16|24>            bit depth, default 24\n"
      << "  --no-normalize            keep the plugin's level (default: normalise to -1 dBFS)\n"
      << "  --tail <seconds>          max release tail after note-off, default 6\n"
      << "  --warmup <seconds>        time for sample-based plugins to load, default 1.5\n\n"
      << "Info:\n"
      << "  --list-styles  --list-plugins  --dump-style <id>  --plan-only  --help\n\n"
      << "Example (16 Levels kit with the built-in synth):\n"
      << "  mpcforge --style one-shot-palette --16levels --count 64 --out ~/MPC/Palette\n";
    return s;
}

static void out (const juce::String& s) { std::cout << s << std::endl; }

bool CliRunner::wantsCli (const juce::StringArray& args)
{
    for (auto a : { "--cli", "--help", "-h", "--list-styles", "--list-plugins", "--dump-style" })
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
        auto* s = findBuiltInStyle (option (args, "--style", "neo-soul"));
        if (s == nullptr) return fail ("unknown style '" + option (args, "--style") + "' (see --list-styles)");
        req.style = *s;
    }

    req.plan.count  = option (args, "--count", "100").getIntValue();
    req.plan.key    = option (args, "--key", "random");
    req.plan.scale  = option (args, "--scale");
    req.plan.prefix = option (args, "--prefix");
    req.plan.recipe = option (args, "--recipe").toLowerCase();
    if (req.plan.recipe.isNotEmpty() && req.plan.recipe != "mixed")
    {
        Recipe r;
        if (! parseRecipe (req.plan.recipe, r))
            return fail ("unknown --recipe (use " + recipeIds().joinIntoString (", ") + " or mixed)");
    }

    auto rootText = option (args, "--root", args.contains ("--16levels") ? "C3" : "");
    if (rootText.isNotEmpty() && ! rootText.equalsIgnoreCase ("free"))
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
    req.render.trimToSec  = args.contains ("--max-length") ? option (args, "--max-length").getDoubleValue()
                                                           : req.style.maxLength;

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

    auto specs = planSamples (req.style, req.plan, error);
    if (specs.empty()) return fail (error);
    if (error.isNotEmpty()) std::cerr << "warning: " << error << std::endl;

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
         + (req.plan.fixedRoot >= 0 ? "   16 Levels root: " + midiNoteName (req.plan.fixedRoot) : juce::String())
         + (useBuiltIn ? "   recipe: " + (req.plan.recipe.isNotEmpty() ? req.plan.recipe : req.style.recipe) : juce::String()));
    out ("Output:     " + req.outputFolder.getFullPathName());

    job = std::make_unique<BatchJob> (*instrument, req, std::move (specs));
    job->onProgress = [] (int doneCount, int total, const juce::String& msg)
    {
        out ("[" + juce::String (doneCount).paddedLeft (' ', 3) + "/" + juce::String (total) + "] " + msg);
    };
    job->onFinished = [this] (bool ok, const juce::String& summary)
    {
        out (summary);
        job.reset();
        instrument.reset();
        onDone (ok ? 0 : 2);
    };
    job->startThread();
}
}
