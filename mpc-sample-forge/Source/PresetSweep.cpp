#include "PresetSweep.h"
#include "BuiltInSynth.h"
#include "PluginHost.h"
#include <cmath>
#include <set>
#include <map>

namespace forge
{
//==============================================================================
// Name filters

/** Lower-case words of a name: "005_ArpPulse FX" -> 005, arp, pulse, fx. */
static juce::StringArray nameTokens (const juce::String& text)
{
    juce::StringArray tokens;
    juce::String token;
    juce::juce_wchar previous = 0;
    auto flush = [&] { if (token.isNotEmpty()) { tokens.add (token.toLowerCase()); token.clear(); } };
    for (auto c : text + " ")
    {
        const bool alnum = juce::CharacterFunctions::isLetterOrDigit (c);
        if (! alnum) { flush(); previous = 0; continue; }
        const bool split = previous != 0
                           && ((juce::CharacterFunctions::isLowerCase (previous) && juce::CharacterFunctions::isUpperCase (c))
                               || (juce::CharacterFunctions::isDigit (previous) != juce::CharacterFunctions::isDigit (c)));
        if (split) flush();
        token += juce::String::charToString (c);
        previous = c;
    }
    return tokens;
}

bool nameLooksUnsuitable (const juce::String& text, juce::String& why)
{
    // whole words (or the start of one, for the longer stems) in the preset name or its category folders
    static const char* exact[] = { "arp", "seq", "sq", "fx", "sfx", "kit", "hit", "hits", "drm", "prc", "atm", "init", "default", "gate",
                                   "loop", "loops", "rise", "noise" };
    static const char* stems[] = { "arpegg", "sequenc", "pattern", "groove", "drum", "perc", "riser", "sweep", "impact", "atmos",
                                   "ambien", "texture", "soundscape", "drone", "gated", "trancegate", "rhythm" };
    auto tokens = nameTokens (text);
    for (auto& t : tokens)
    {
        for (auto* w : exact) if (t == w) { why = "name says '" + t + "'"; return true; }
        for (auto* w : stems) if (t.startsWith (w)) { why = "name says '" + t + "'"; return true; }
    }
    return false;
}

/** Words that suggest a preset suits a recipe's sound (by its engine). */
static juce::StringArray recipeWords (const juce::String& engine)
{
    static const std::map<juce::String, const char*> words {
        { "epiano", "ep,e.piano,e-piano,epiano,electric piano,rhodes,suitcase,stage,wurli,keys,tine" },
        { "epbell", "ep,dx,fm,tine,bell,keys,e.piano,electric piano" },
        { "wurli", "wurli,ep,e.piano,keys,electric piano" },
        { "bell", "bell,glock,celest,mallet,chime,vibe,marimba,kalimba" },
        { "musicbox", "music box,musicbox,box,celest,toy,glock,kalimba" },
        { "organ", "organ,b3,hammond,drawbar,gospel,church" },
        { "comboorgan", "organ,combo,farfisa,vox,compact" },
        { "pluck", "pluck,guitar,harp,koto,pizz,picked" },
        { "harp", "harp,pluck,lyre" },
        { "pizz", "pizz,string,pluck" },
        { "triangle", "triangle,sine,soft lead,whistle,flute" },
        { "whistle", "whistle,sine,lead,flute" },
        { "lead", "lead,saw,solo,synth" },
        { "supersaw", "saw,super,trance,lead,stab,synth" },
        { "synth", "synth,poly,stab,saw" },
        { "brass", "brass,horn,trumpet,trombone,section,stab" },
        { "choir", "choir,vox,voice,vocal,aah,ooh,chant" },
        { "bass", "bass,sub,low" },
        { "moogbass", "bass,moog,mini,funk,sub" },
        { "808", "808,bass,sub,boom" },
        { "logdrum", "log,bass,amapiano" },
        { "clav", "clav,funk" },
        { "talkbox", "talk,vocoder,voice,robot" },
        { "flute", "flute,mellotron,tron,pan,recorder,woodwind" },
        { "sax", "sax,alto,tenor,horn,woodwind" },
        { "cowbell", "cowbell,bell" },
    };
    auto it = words.find (engine);
    return it != words.end() ? juce::StringArray::fromTokens (it->second, ",", "") : juce::StringArray();
}

/** Short words must be whole words ("ep" is not in "deep"); longer ones can be part of one ("rhodes" in "MkIRhodes"). */
static bool matchesAnyWord (const juce::String& text, const juce::StringArray& words)
{
    auto tokens = nameTokens (text);
    for (auto w : words)
    {
        w = w.trim().toLowerCase();
        if (w.isEmpty()) continue;
        if (w.length() <= 3 ? tokens.contains (w) : text.containsIgnoreCase (w)) return true;
    }
    return false;
}

static bool nameMatchesInclude (const juce::String& text, const juce::String& include)
{
    auto words = juce::StringArray::fromTokens (include, ",;", "");
    words.trim();
    words.removeEmptyStrings();
    if (words.isEmpty()) return true;
    for (auto& w : words) if (text.containsIgnoreCase (w)) return true;
    return false;
}

//==============================================================================
// Envelope measurements

static std::vector<float> envelopeDb (const juce::AudioBuffer<float>& audio, double sampleRate, double frameSec)
{
    const int frame = juce::jmax (16, (int) (sampleRate * frameSec));
    std::vector<float> env;
    for (int start = 0; start + frame <= audio.getNumSamples(); start += frame)
    {
        double sum = 0;
        for (int ch = 0; ch < audio.getNumChannels(); ++ch)
        {
            auto* d = audio.getReadPointer (ch, start);
            for (int i = 0; i < frame; ++i) sum += (double) d[i] * d[i];
        }
        auto rms = std::sqrt (sum / (double) (frame * juce::jmax (1, audio.getNumChannels())));
        env.push_back ((float) juce::Decibels::gainToDecibels (rms, -120.0));
    }
    return env;
}

int countRepeatedOnsets (const juce::AudioBuffer<float>& audio, double sampleRate, double untilSec)
{
    // Re-triggered notes (arps, gates, sequences) dip 10+ dB and come back quickly. Tremolo, chorus
    // and the beating of chord notes wobble only a few dB, so they don't count.
    const double frameSec = 0.02;
    auto env = envelopeDb (audio, sampleRate, frameSec);
    if (env.size() < 6) return 0;
    const auto peak = *std::max_element (env.begin(), env.end());
    const auto floorDb = peak - 45.0f;

    size_t i = 0;
    while (i < env.size() && env[i] < peak - 10.0f) ++i;   // past the first attack
    const auto end = juce::jmin (env.size(), (size_t) (untilSec / frameSec));

    int onsets = 0;
    float high = i < env.size() ? env[i] : peak, low = high;
    bool inGap = false;
    for (; i < end; ++i)
    {
        const auto e = env[i];
        if (! inGap)
        {
            high = std::max (high, e);
            if (e < high - 10.0f) { inGap = true; low = e; }        // a real gap between notes
        }
        else
        {
            low = std::min (low, e);
            // back up 10 dB within 3 frames (60 ms): a new attack, not a slow swell
            if (e > low + 10.0f && e > floorDb && i >= 3 && env[i - 3] < e - 8.0f)
            {
                ++onsets;
                inGap = false;
                high = e;
            }
        }
    }
    return onsets;
}

/** How far (dB) the sound has fallen from its peak by `atSec` (sustained ~0, plucked 20+). */
static float sustainDrop (const juce::AudioBuffer<float>& audio, double sampleRate, double atSec)
{
    auto env = envelopeDb (audio, sampleRate, 0.02);
    if (env.empty()) return 0;
    const auto peak = *std::max_element (env.begin(), env.end());
    auto i = juce::jlimit ((size_t) 0, env.size() - 1, (size_t) (atSec / 0.02));
    return juce::jlimit (0.0f, 60.0f, peak - env[i]);
}

double attackMs (const juce::AudioBuffer<float>& audio, double sampleRate)
{
    auto env = envelopeDb (audio, sampleRate, 0.005);
    if (env.empty()) return 0;
    const auto peak = *std::max_element (env.begin(), env.end());
    for (size_t i = 0; i < env.size(); ++i)
        if (env[i] >= peak - 6.0f) return (double) i * 5.0;
    return 0;
}

//==============================================================================
// Collecting and loading presets

juce::Array<juce::File> standardPresetFolders (const juce::String& pluginName)
{
    juce::Array<juce::File> roots;
   #if JUCE_WINDOWS
    roots.add (juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("VST3 Presets"));
    roots.add (juce::File::getSpecialLocation (juce::File::commonApplicationDataDirectory).getChildFile ("VST3 Presets"));
   #elif JUCE_MAC
    roots.add (juce::File ("~/Library/Audio/Presets"));
    roots.add (juce::File ("/Library/Audio/Presets"));
   #else
    roots.add (juce::File ("~/.vst3/presets"));
    roots.add (juce::File ("/usr/share/vst3/presets"));
    roots.add (juce::File ("/usr/local/share/vst3/presets"));
   #endif

    auto simplify = [] (const juce::String& s) { return s.toLowerCase().retainCharacters ("abcdefghijklmnopqrstuvwxyz0123456789"); };
    const auto wanted = simplify (pluginName);
    juce::Array<juce::File> found;
    if (wanted.isEmpty()) return found;

    for (auto& root : roots)
    {
        if (! root.isDirectory()) continue;
        // <root>/<Vendor>/<Plugin>, or <root>/<Plugin>
        for (auto& level1 : root.findChildFiles (juce::File::findDirectories, false))
        {
            if (simplify (level1.getFileName()) == wanted) { found.addIfNotAlreadyThere (level1); continue; }
            for (auto& level2 : level1.findChildFiles (juce::File::findDirectories, false))
                if (simplify (level2.getFileName()) == wanted) found.addIfNotAlreadyThere (level2);
        }
    }
    return found;
}

static bool parameterSafeToVary (juce::AudioProcessorParameter& p)
{
    if (! p.isAutomatable() || p.isDiscrete() || p.isBoolean() || p.isMetaParameter()) return false;
    auto name = p.getName (64).toLowerCase();
    // pitch, level and routing would make every variation out of tune / quiet / broken
    static const char* avoid[] = { "tune", "pitch", "octave", "oct ", "transp", "semi", "coarse", "fine", "detune", "volume", "vol",
                                   "gain", "level", "master", "output", "bypass", "mix", "program", "bank", "preset", "midi", "channel",
                                   "bend", "wheel", "poly", "voices", "glide", "porta", "tempo", "sync", "arp", "seq", "mono", "legato",
                                   "pan", "key track", "keytrack", "random", "seed" };
    for (auto* a : avoid) if (name.contains (a)) return false;
    return true;
}

std::vector<PresetEntry> collectPresets (juce::AudioProcessor& p, const SweepOptions& o, juce::int64 seed, juce::String& error)
{
    std::vector<PresetEntry> out;
    juce::Random rng (seed);

    if (dynamic_cast<BuiltInSynth*> (&p) != nullptr)
    {
        // the built-in synth: its engines are the "presets", each with a few timbre seeds
        if (o.source == SweepSource::PresetFolder) { error = "preset files need a VST3 instrument (the built-in synth has none)"; return {}; }
        auto ids = engineIds();
        if (o.source == SweepSource::Programs)
        {
            for (auto& id : ids)
            {
                Engine e;
                parseEngine (id, e);
                for (int k = 0; k < 3; ++k)
                {
                    PresetEntry entry;
                    entry.engine = id;
                    entry.variation = rng.nextInt64();
                    entry.name = engineDisplayName (e) + " " + juce::String (k + 1);
                    entry.matchText = entry.name;
                    out.push_back (entry);
                }
            }
        }
        else
        {
            for (int k = 0; k < juce::jlimit (1, 500, o.variations); ++k)
            {
                PresetEntry entry;
                entry.engine = {};   // PresetSweep::start fills in the current engine
                entry.variation = rng.nextInt64();
                entry.name = "Variation " + juce::String (k + 1).paddedLeft ('0', 2);
                entry.matchText = entry.name;
                out.push_back (entry);
            }
        }
        return out;
    }

    switch (o.source)
    {
        case SweepSource::Programs:
        {
            const int n = p.getNumPrograms();
            if (n <= 1)
            {
                error = p.getName() + " doesn't list its presets to host apps. Try 'Preset files' (if it has .vstpreset files) "
                        "or 'Variations of the current sound'.";
                return {};
            }
            for (int i = 0; i < n; ++i)
            {
                PresetEntry entry;
                entry.program = i;
                entry.name = p.getProgramName (i).trim();
                if (entry.name.isEmpty()) entry.name = "Program " + juce::String (i + 1);
                entry.matchText = entry.name;
                out.push_back (entry);
            }
            break;
        }
        case SweepSource::PresetFolder:
        {
            juce::Array<juce::File> folders;
            if (o.folder.isDirectory()) folders.add (o.folder);
            else folders = standardPresetFolders (p.getName());
            if (folders.isEmpty())
            {
                error = "no preset folder found for " + p.getName() + " - choose the folder that holds its .vstpreset (or .forgesound) files";
                return {};
            }
            for (auto& folder : folders)
            {
                auto files = folder.findChildFiles (juce::File::findFiles, true, "*.vstpreset;*.forgesound");
                files.sort();
                for (auto& f : files)
                {
                    PresetEntry entry;
                    entry.file = f;
                    entry.name = f.getFileNameWithoutExtension();
                    entry.matchText = f.getRelativePathFrom (folder);
                    out.push_back (entry);
                }
            }
            if (out.empty()) error = "no .vstpreset or .forgesound files in " + folders.getFirst().getFullPathName();
            break;
        }
        case SweepSource::Variations:
        {
            auto state = std::make_shared<juce::MemoryBlock>();
            p.getStateInformation (*state);
            std::vector<int> eligible;
            auto params = p.getParameters();
            for (int i = 0; i < params.size(); ++i)
                if (parameterSafeToVary (*params[i])) eligible.push_back (i);
            if (eligible.empty()) { error = p.getName() + " has no parameters that can be varied safely"; return {}; }

            const auto strength = juce::jlimit (0.02f, 1.0f, o.strength);
            for (int k = 0; k < juce::jlimit (1, 500, o.variations); ++k)
            {
                PresetEntry entry;
                entry.baseState = state;
                entry.name = "Variation " + juce::String (k + 1).paddedLeft ('0', 2);
                entry.matchText = entry.name;
                // move a handful of parameters a little: same character, new colour
                auto picks = eligible;
                for (size_t i = picks.size(); i > 1; --i) std::swap (picks[i - 1], picks[(size_t) rng.nextInt ((int) i)]);
                const int count = juce::jmin ((int) picks.size(), 3 + rng.nextInt (6));
                for (int c = 0; c < count; ++c)
                {
                    auto* prm = params[picks[(size_t) c]];
                    auto v = juce::jlimit (0.0f, 1.0f, prm->getValue() + (rng.nextFloat() * 2.0f - 1.0f) * strength);
                    entry.paramValues.push_back ({ picks[(size_t) c], v });
                }
                out.push_back (entry);
            }
            break;
        }
    }
    return out;
}

bool applyPreset (juce::AudioProcessor& p, const PresetEntry& e, juce::String& error)
{
    if (auto* b = dynamic_cast<BuiltInSynth*> (&p))
    {
        Engine engine;
        if (e.engine.isNotEmpty() && parseEngine (e.engine, engine)) b->setEngine (engine);
        b->setVariation (e.variation);
        return true;
    }
    if (e.program >= 0)
    {
        p.setCurrentProgram (e.program);
        return true;
    }
    if (e.file != juce::File())
    {
        if (e.file.hasFileExtension ("forgesound"))
        {
            SoundState s;
            if (! s.load (e.file, error)) return false;
            if (s.pluginName.isNotEmpty() && s.pluginName != p.getName())
            {
                error = "saved for " + s.pluginName + ", not " + p.getName();
                return false;
            }
            p.setStateInformation (s.state.getData(), (int) s.state.getSize());
            return true;
        }
       #if JUCE_PLUGINHOST_VST3
        juce::MemoryBlock data;
        if (! e.file.loadFileAsData (data)) { error = "could not read " + e.file.getFileName(); return false; }
        auto* instance = dynamic_cast<juce::AudioPluginInstance*> (&p);
        if (instance == nullptr || ! juce::VST3PluginFormat::setStateFromVSTPresetFile (instance, data))
        {
            error = "not a preset for this plugin";
            return false;
        }
        return true;
       #else
        error = "this build cannot read .vstpreset files";
        return false;
       #endif
    }
    if (e.baseState != nullptr)
    {
        p.setStateInformation (e.baseState->getData(), (int) e.baseState->getSize());
        auto params = p.getParameters();
        for (auto& [index, value] : e.paramValues)
            if (juce::isPositiveAndBelow (index, params.size())) params[index]->setValue (value);
        return true;
    }
    error = "empty preset";
    return false;
}

int exportPresets (juce::AudioProcessor& p, const juce::String& pluginPath, const RenderSettings& settings,
                   const juce::File& folder, juce::String& error)
{
    const int n = p.getNumPrograms();
    if (n <= 1) { error = p.getName() + " doesn't list its presets to host apps"; return 0; }
    if (! folder.createDirectory()) { error = "could not create " + folder.getFullPathName(); return 0; }

    struct PresetGrabber final : juce::ExtensionsVisitor
    {
        juce::MemoryBlock data;
        void visitVST3Client (const VST3Client& c) override { data = c.getPreset(); }
    };

    OfflineRenderer renderer (p, settings);
    int written = 0;
    for (int i = 0; i < n; ++i)
    {
        p.setCurrentProgram (i);
        renderer.settle (0.05); // let the program change reach the plugin's processor
        auto name = juce::String (i + 1).paddedLeft ('0', 3) + "_" + safeFileName (p.getProgramName (i).trim());
        PresetGrabber grab;
        if (auto* instance = dynamic_cast<juce::AudioPluginInstance*> (&p)) instance->getExtensions (grab);
        if (grab.data.getSize() > 0)
        {
            if (folder.getChildFile (name + ".vstpreset").replaceWithData (grab.data.getData(), grab.data.getSize())) ++written;
        }
        else if (captureState (p, pluginPath).save (folder.getChildFile (name + ".forgesound")))
            ++written;
    }
    return written;
}

//==============================================================================
// The probe thread: one test render per preset

class PresetSweep::ProbeThread final : public juce::Thread
{
public:
    struct Probe
    {
        bool rendered = false;
        SampleFeatures features;
        int onsets = 0;
        double attack = 0;
        float drop = 0;
        float rawPeak = 0;
        juce::String error;
    };

    ProbeThread (PresetSweep& o, std::vector<int> which, SampleSpec spec, juce::File probesFolder)
        : juce::Thread ("Preset sweep probes"), owner (o), indices (std::move (which)), probeSpec (std::move (spec)),
          folder (std::move (probesFolder)), probes (o.probeResults.size()) {}

    ~ProbeThread() override { stopThread (20000); }

    std::vector<Probe> takeProbes() { return std::move (probes); }
    SampleFeatures referenceFeatures;
    float referenceDrop = 0;
    bool hasReference = false;

    void run() override
    {
        auto alive = owner.alive;
        auto& instrument = owner.instrument;
        auto* builtIn = dynamic_cast<BuiltInSynth*> (&instrument);
        auto settings = owner.base.render;
        // the recipe's own sound (built-in synth, its engine) as a reference for "fits the recipe"
        {
            BuiltInSynth reference;
            OfflineRenderer refRenderer (reference, settings);
            auto refSpec = probeSpec;
            refSpec.engine = owner.base.style.engine == "mixed" ? juce::String ("epiano") : owner.base.style.engine;
            refSpec.variation = 1;
            Engine e;
            if (parseEngine (refSpec.engine, e)) reference.setEngine (e);
            reference.setVariation (refSpec.variation);
            juce::AudioBuffer<float> refAudio;
            juce::String refError;
            if (refRenderer.render (refSpec, refAudio, refError) && refAudio.getMagnitude (0, refAudio.getNumSamples()) > 1.0e-4f)
            {
                refAudio.applyGain (0.89f / refAudio.getMagnitude (0, refAudio.getNumSamples()));
                referenceFeatures = analyseSample (refAudio, settings.sampleRate, refSpec.rootNote, refSpec.kind != "Chord");
                referenceDrop = sustainDrop (refAudio, settings.sampleRate, refSpec.holdSec() * 0.8);
                hasReference = true;
            }
        }

        OfflineRenderer renderer (instrument, settings);
        if (builtIn == nullptr) renderer.warmUp();
        if (owner.options.keepProbes) folder.createDirectory();

        for (size_t k = 0; k < indices.size(); ++k)
        {
            if (threadShouldExit()) break;
            const auto index = (size_t) indices[k];
            auto& probe = probes[index];
            const auto entry = owner.probeResults[index].entry;
            auto spec = probeSpec;

            if (builtIn != nullptr)
            {
                builtIn->reset();
                spec.engine = entry.engine;
                spec.variation = entry.variation;
                juce::String err;
                applyPreset (*builtIn, entry, err);
            }
            else
            {
                // presets are loaded on the message thread (plugins expect that), then given a moment to settle
                struct Result { juce::WaitableEvent done; bool ok = false; juce::String error; };
                auto result = std::make_shared<Result>();
                auto* target = &instrument;
                juce::MessageManager::callAsync ([alive, result, target, entry]
                {
                    if (*alive) result->ok = applyPreset (*target, entry, result->error);
                    result->done.signal();
                });
                while (! result->done.wait (50))
                    if (threadShouldExit()) return;
                if (! result->ok) { probe.error = "could not load: " + result->error; continue; }
                renderer.settle (0.25);
            }

            juce::AudioBuffer<float> audio;
            juce::String err;
            if (! renderer.render (spec, audio, err))
            {
                if (err.contains ("no sound")) { probe.rendered = true; probe.rawPeak = 0; continue; } // silent preset
                probe.error = "render failed: " + err;
                continue;
            }

            probe.rawPeak = audio.getMagnitude (0, audio.getNumSamples());
            if (probe.rawPeak > 1.0e-4f)
            {
                audio.applyGain (0.89f / probe.rawPeak); // level-independent scoring (and comfortable listening)
                probe.features = analyseSample (audio, settings.sampleRate, spec.rootNote, spec.kind != "Chord");
                probe.onsets = countRepeatedOnsets (audio, settings.sampleRate, spec.holdSec());
                probe.attack = attackMs (audio, settings.sampleRate);
                probe.drop = sustainDrop (audio, settings.sampleRate, spec.holdSec() * 0.8);
            }
            probe.rendered = true;

            if (owner.options.keepProbes)
            {
                auto name = juce::String ((int) index + 1).paddedLeft ('0', 3) + "_" + safeFileName (entry.name);
                OfflineRenderer::writeWav (folder.getChildFile (name + ".wav"), audio, settings.sampleRate, 16, spec.rootNote, err);
            }

            const auto fraction = (double) (k + 1) / (double) indices.size();
            const auto message = "Tried " + juce::String ((int) k + 1) + "/" + juce::String ((int) indices.size()) + ": " + entry.name;
            auto* o = &owner;
            juce::MessageManager::callAsync ([alive, o, fraction, message, k]
            {
                if (! *alive) return;
                o->progress (fraction * 0.25);
                if (k % 10 == 0) o->log (message);
            });
        }

        auto* o = &owner;
        const bool stopped = threadShouldExit();
        juce::MessageManager::callAsync ([alive, o, stopped]
        {
            if (*alive) o->probesFinished (! stopped, stopped ? "cancelled" : juce::String());
        });
    }

private:
    PresetSweep& owner;
    std::vector<int> indices;
    SampleSpec probeSpec;
    juce::File folder;
    std::vector<Probe> probes;
};

//==============================================================================
PresetSweep::PresetSweep (juce::AudioProcessor& p, std::vector<juce::AudioProcessor*> e, BatchRequest b, SweepOptions o)
    : instrument (p), extras (std::move (e)), base (std::move (b)), options (std::move (o)) {}

PresetSweep::~PresetSweep()
{
    *alive = false;
    probeThread.reset();
    job.reset();
}

bool PresetSweep::start (juce::String& error)
{
    startedMs = juce::Time::getMillisecondCounterHiRes();
    auto entries = collectPresets (instrument, options, base.plan.seed, error);
    if (entries.empty()) { if (error.isEmpty()) error = "no presets to try"; return false; }

    auto* builtIn = dynamic_cast<BuiltInSynth*> (&instrument);
    if (builtIn != nullptr && options.source == SweepSource::Variations)
        for (auto& en : entries)
            en.engine = base.plan.engine.isNotEmpty() && base.plan.engine != "mixed" ? base.plan.engine
                                                                                       : (base.style.engine == "mixed" ? juce::String ("epiano") : base.style.engine);

    // filters first, then an even spread over what is left (category variety, not just the first N)
    std::vector<int> candidates;
    for (auto& en : entries)
    {
        ProbeResult r;
        r.entry = en;
        juce::String why;
        if (! nameMatchesInclude (en.matchText, options.include)) r.reasons.add ("name doesn't match '" + options.include + "'");
        else if (options.skipByName && nameLooksUnsuitable (en.matchText, why)) r.reasons.add (why);
        else candidates.push_back ((int) probeResults.size());
        probeResults.push_back (r);
    }
    std::vector<int> toProbe = candidates;
    if (options.maxPresets > 0 && (int) candidates.size() > options.maxPresets)
    {
        toProbe.clear();
        std::set<int> chosen;
        for (int k = 0; k < options.maxPresets; ++k)
            chosen.insert (candidates[(size_t) ((double) k * (double) candidates.size() / options.maxPresets)]);
        toProbe.assign (chosen.begin(), chosen.end());
        for (auto c : candidates)
            if (chosen.count (c) == 0) probeResults[(size_t) c].reasons.add ("not tried (limit " + juce::String (options.maxPresets) + ")");
    }
    if (toProbe.empty())
    {
        error = juce::String ((int) entries.size()) + " presets found, but the name filters skipped all of them";
        return false;
    }
    probeCount = (int) toProbe.size();

    // the test sound: the style's own first chord (or note) in its own register
    PlanOptions po = base.plan;
    po.count = 24;
    po.scaleBank = false;
    po.fixedRoot = -1;
    po.key = "random";
    po.progressionNumber = po.progressionNumber > 0 ? po.progressionNumber : 1;
    juce::String planError;
    auto plan = planSamples (base.style, po, planError);
    if (plan.empty()) { error = "could not plan a test sound: " + planError; return false; }
    const bool chordStyle = base.style.chordRatio >= 0.5f;
    SampleSpec probe = plan.front();
    for (auto& s : plan)
        if ((s.kind == "Chord") == chordStyle) { probe = s; break; }
    probe.fileStem = "probe";

    if (! base.outputFolder.createDirectory()) { error = "could not create " + base.outputFolder.getFullPathName(); return false; }
    log ("==== Preset sweep: " + juce::String ((int) entries.size()) + " presets found, trying " + juce::String (probeCount)
         + " (" + juce::String ((int) entries.size() - (int) candidates.size()) + " skipped by name). Test sound: "
         + probe.label + " " + probe.voicing + ".");

    probeThread = std::make_unique<ProbeThread> (*this, toProbe, probe, base.outputFolder.getChildFile ("_Preset_Probes"));
    probeThread->startThread();
    return true;
}

void PresetSweep::cancel()
{
    cancelled = true;
    if (probeThread != nullptr) probeThread->signalThreadShouldExit();
    if (job != nullptr) job->signalThreadShouldExit();
}

void PresetSweep::probesFinished (bool ok, const juce::String& message)
{
    auto probes = probeThread != nullptr ? probeThread->takeProbes() : std::vector<ProbeThread::Probe>();
    const bool hasReference = probeThread != nullptr && probeThread->hasReference;
    const auto reference = probeThread != nullptr ? probeThread->referenceFeatures : SampleFeatures();
    const auto referenceDrop = probeThread != nullptr ? probeThread->referenceDrop : 0.0f;
    probeThread.reset();
    if (! ok || cancelled) { writeReport(); finish (false, "Preset sweep " + (message.isNotEmpty() ? message : juce::String ("stopped"))); return; }

    // ---- score: Smart Pick's checks + preset-specific ones, then the best different-sounding presets
    std::vector<PickCandidate> cands;
    std::vector<int> candIndex;
    for (size_t i = 0; i < probeResults.size() && i < probes.size(); ++i)
    {
        auto& r = probeResults[i];
        auto& p = probes[i];
        if (! p.rendered && p.error.isEmpty()) continue; // never tried
        r.probed = true;
        if (! p.rendered) { r.reasons.add (p.error); continue; }
        if (p.rawPeak < 0.001f) { r.reasons.add ("silent"); continue; }
        if (options.skipRhythmic && p.onsets >= 2)
        {
            r.reasons.add ("pulses " + juce::String (p.onsets) + "x while held (arp / gate / sequence?)");
            continue;
        }
        PickCandidate c;
        c.features = p.features;
        c.group = "preset";
        c.rootNote = 60;
        if (p.attack > 150.0) { c.extraPenalty = 15.0f; c.extraReason = "slow attack (" + juce::String (juce::roundToInt (p.attack)) + " ms)"; }
        if (p.onsets == 1 && c.extraPenalty == 0) { c.extraPenalty = 8.0f; c.extraReason = "swells once while held"; }
        cands.push_back (c);
        candIndex.push_back ((int) i);
    }
    if (cands.empty()) { writeReport(); finish (false, "No preset passed the checks - see _Preset_Sweep.txt for why."); return; }

    // ---- fit to the recipe: sound (closeness to the recipe's reference) and name (words that suit it)
    const auto recipeEngine = base.style.engine == "mixed" ? juce::String ("epiano") : base.style.engine;
    const auto words = recipeWords (recipeEngine);
    bool anyNameFits = false;
    for (auto i : candIndex) anyNameFits = anyNameFits || matchesAnyWord (probeResults[(size_t) i].entry.matchText, words);
    std::vector<float> drops;
    for (auto i : candIndex) drops.push_back (probes[(size_t) i].drop);
    for (size_t c = 0; c < cands.size(); ++c)
    {
        auto& cand = cands[c];
        juce::StringArray why;
        if (cand.extraReason.isNotEmpty()) why.add (cand.extraReason);
        if (hasReference && reference.centroidHz > 0 && cand.features.centroidHz > 0)
        {
            // brightness: octaves away from the recipe's own sound
            auto octaves = std::abs (std::log2 (cand.features.centroidHz / reference.centroidHz));
            if (octaves > 0.5f)
            {
                auto points = juce::jmin (20.0f, (octaves - 0.5f) * 16.0f);
                cand.extraPenalty += points;
                why.add (juce::String (cand.features.centroidHz > reference.centroidHz ? "brighter" : "darker")
                         + " than the recipe (-" + juce::String (juce::roundToInt (points)) + ")");
            }
            // envelope: sustains vs. dies away, compared with the recipe's sound
            auto envOff = std::abs (drops[c] - referenceDrop) - 8.0f;
            if (envOff > 0)
            {
                auto points = juce::jmin (20.0f, envOff * 0.8f);
                cand.extraPenalty += points;
                why.add (juce::String (drops[c] > referenceDrop ? "dies away faster" : "sustains longer")
                         + " than the recipe (-" + juce::String (juce::roundToInt (points)) + ")");
            }
        }
        if (anyNameFits && ! matchesAnyWord (probeResults[(size_t) candIndex[c]].entry.matchText, words))
        {
            cand.extraPenalty += 10.0f;
            why.add ("name doesn't suggest " + words.joinIntoString ("/", 0, juce::jmin (3, words.size())));
        }
        cand.extraReason = why.joinIntoString ("; ");
    }

    PickSettings ps;
    ps.strictVariety = true;  // two presets that sound the same are one preset
    ps.varietyPerDb = 1.0f;   // fit to the recipe matters more than being different
    ps.varietyCapDb = 4.0f;
    auto verdicts = smartPick (cands, juce::jmin (options.keep, (int) cands.size()), ps);
    for (size_t c = 0; c < verdicts.size(); ++c)
    {
        auto& r = probeResults[(size_t) candIndex[c]];
        r.score = verdicts[c].score;
        r.kept = verdicts[c].kept;
        r.reasons.addArray (verdicts[c].reasons);
        if (! r.kept && verdicts[c].similarTo >= 0 && juce::isPositiveAndBelow (verdicts[c].similarTo, (int) candIndex.size()))
            r.reasons.add ("sounds like " + probeResults[(size_t) candIndex[(size_t) verdicts[c].similarTo]].entry.name);
        if (r.kept) keptOrder.push_back (candIndex[c]);
    }
    std::stable_sort (keptOrder.begin(), keptOrder.end(), [this] (int a, int b)
                      { return probeResults[(size_t) a].score > probeResults[(size_t) b].score; });

    // "the best" means good ones: drop weak presets that only made it because there were few candidates
    int belowMin = 0;
    for (size_t k = 1; k < keptOrder.size();)
    {
        auto& r = probeResults[(size_t) keptOrder[k]];
        if (r.score < options.minScore)
        {
            r.kept = false;
            r.reasons.add ("score below " + juce::String (juce::roundToInt (options.minScore)));
            keptOrder.erase (keptOrder.begin() + (std::ptrdiff_t) k);
            ++belowMin;
        }
        else ++k;
    }
    if (belowMin > 0) log (juce::String (belowMin) + " more presets scored below " + juce::String (juce::roundToInt (options.minScore)) + " and were left out.");

    juce::StringArray names;
    for (auto k : keptOrder) names.add (probeResults[(size_t) k].entry.name + " (" + juce::String (juce::roundToInt (probeResults[(size_t) k].score)) + ")");
    log ("Kept " + juce::String ((int) keptOrder.size()) + " of " + juce::String (probeCount) + " presets: " + names.joinIntoString (", "));
    renderNext();
}

void PresetSweep::renderNext()
{
    if (cancelled) { writeReport(); finish (false, "Preset sweep cancelled after " + juce::String ((int) nextKept) + " presets."); return; }
    if (nextKept >= keptOrder.size())
    {
        writeReport();
        auto minutes = (juce::Time::getMillisecondCounterHiRes() - startedMs) / 60000.0;
        finish (true, "Preset sweep done: " + juce::String ((int) keptOrder.size()) + " presets x " + juce::String (options.perPreset)
                + " samples in " + juce::String (minutes, 1) + " min. Overview: " + base.outputFolder.getChildFile ("_Preset_Sweep.txt").getFullPathName());
        return;
    }

    auto& r = probeResults[(size_t) keptOrder[nextKept]];
    juce::String err;
    if (! applyPreset (instrument, r.entry, err)) log ("WARNING: " + r.entry.name + ": " + err);
    for (auto* e : extras) if (e != nullptr) applyPreset (*e, r.entry, err);

    auto* builtIn = dynamic_cast<BuiltInSynth*> (&instrument);
    BatchRequest req = base;
    r.folderName = juce::String ((int) nextKept + 1).paddedLeft ('0', 2) + "_" + safeFileName (r.entry.name);
    req.outputFolder = base.outputFolder.getChildFile (r.folderName).getNonexistentSibling (false);
    r.folderName = req.outputFolder.getFileName();
    req.instrumentName = instrument.getName() + " - " + r.entry.name;
    req.extraInstances = extras;
    req.plan.seed = base.plan.seed + 7919 * (juce::int64) (nextKept + 1);
    req.plan.count = options.perPreset;
    if (builtIn != nullptr) { req.plan.engine = r.entry.engine; req.plan.builtIn = true; }

    const bool banks = req.plan.scaleBank || req.style.scaleBankOnly;
    req.smartPickTarget = 0;
    if (banks) { req.splitBanks = true; req.registers.clear(); }
    else if (base.smartPickTarget > 0) { req.plan.count = (int) std::ceil (options.perPreset * 1.5); req.smartPickTarget = options.perPreset; }

    juce::String planError;
    auto specs = planSamples (req.style, req.plan, planError);
    if (builtIn != nullptr) for (auto& s : specs) s.variation = r.entry.variation; // one timbre = one "preset"
    if (req.smartPickTarget > 0 && (int) specs.size() <= req.smartPickTarget) req.smartPickTarget = 0;
    if (specs.empty())
    {
        log ("WARNING: nothing planned for " + r.entry.name + ": " + planError);
        ++nextKept;
        renderNext();
        return;
    }

    log ("---- Preset " + juce::String ((int) nextKept + 1) + "/" + juce::String ((int) keptOrder.size()) + ": " + r.entry.name
         + "  (score " + juce::String (juce::roundToInt (r.score)) + ") -> " + r.folderName);

    const auto share = 0.75 / (double) keptOrder.size();
    const auto startAt = 0.25 + share * (double) nextKept;
    auto alivePtr = alive;
    job = std::make_unique<BatchJob> (instrument, req, std::move (specs));
    job->onProgress = [this, alivePtr, share, startAt] (int done, int total, const juce::String& msg)
    {
        if (! *alivePtr) return;
        progress (startAt + share * (total > 0 ? (double) done / total : 0.0));
        if (msg.startsWith ("Wrote") || msg.startsWith ("Smart Pick") || msg.startsWith ("WARNING")) log (msg);
    };
    job->onFinished = [this, alivePtr] (bool ok, const juce::String& summary)
    {
        if (! *alivePtr) return;
        auto& done = probeResults[(size_t) keptOrder[nextKept]];
        batchSummaries.add (done.folderName + ": " + summary.upToFirstOccurrenceOf ("\n", false, false));
        if (! ok) log ("WARNING: " + done.entry.name + ": " + summary);
        job.reset();
        ++nextKept;
        renderNext();
    };
    job->startThread();
}

void PresetSweep::finish (bool ok, const juce::String& summary)
{
    if (finished) return;
    finished = true;
    progress (ok ? 1.0 : 0.0);
    if (onFinished) onFinished (ok, summary, base.outputFolder);
}

void PresetSweep::writeReport() const
{
    auto csv = [] (const juce::String& s) { return "\"" + s.replace ("\"", "\"\"") + "\""; };
    juce::String table = "rank,preset,tried,kept,score,folder,notes\n";
    juce::String text;
    text << "MPC Sample Forge - preset sweep\n"
         << "Instrument: " << instrument.getName() << "\nRecipe: " << base.style.name << "\n"
         << "Date: " << juce::Time::getCurrentTime().toString (true, true) << "\n\n";

    text << "KEPT (best first) - each has its own folder, a normal batch you can Review:\n";
    int rank = 0;
    for (auto k : keptOrder)
    {
        auto& r = probeResults[(size_t) k];
        text << "  " << juce::String (++rank).paddedLeft (' ', 2) << ". " << r.entry.name.paddedRight (' ', 32)
             << " score " << juce::roundToInt (r.score) << "   -> " << r.folderName << "\n";
    }

    std::vector<const ProbeResult*> tried, skipped;
    for (auto& r : probeResults)
        if (! r.kept) (r.probed ? tried : skipped).push_back (&r);
    std::stable_sort (tried.begin(), tried.end(), [] (auto* a, auto* b) { return a->score > b->score; });

    text << "\nTRIED, NOT KEPT (best first):\n";
    for (auto* r : tried)
        text << "  " << r->entry.name.paddedRight (' ', 32) << " " << juce::String (juce::roundToInt (r->score)).paddedLeft (' ', 3)
             << "  " << r->reasons.joinIntoString ("; ") << "\n";
    text << "\nSKIPPED WITHOUT TRYING: " << (int) skipped.size() << "\n";
    for (auto* r : skipped)
        text << "  " << r->entry.name.paddedRight (' ', 32) << " " << r->reasons.joinIntoString ("; ") << "\n";
    if (options.keepProbes)
        text << "\nEvery tried preset's test sound is in _Preset_Probes (numbered like the list of presets) - listen through them\n"
             << "to find more you like, then load that preset and Generate.\n";
    if (! batchSummaries.isEmpty()) text << "\nBATCHES:\n  " << batchSummaries.joinIntoString ("\n  ") << "\n";

    rank = 0;
    std::map<int, int> rankOf;
    for (auto k : keptOrder) rankOf[k] = ++rank;
    for (size_t i = 0; i < probeResults.size(); ++i)
    {
        auto& r = probeResults[i];
        table << (rankOf.count ((int) i) ? juce::String (rankOf[(int) i]) : juce::String()) << "," << csv (r.entry.name) << ","
              << (r.probed ? "yes" : "no") << "," << (r.kept ? "yes" : "no") << "," << (r.probed ? juce::String (juce::roundToInt (r.score)) : juce::String())
              << "," << csv (r.folderName) << "," << csv (r.reasons.joinIntoString ("; ")) << "\n";
    }
    base.outputFolder.getChildFile ("_Preset_Sweep.txt").replaceWithText (text);
    base.outputFolder.getChildFile ("_preset_sweep.csv").replaceWithText (table);
}
}
