#include "Diagnostics.h"
#include "BuiltInSynth.h"
#include "OfflineRenderer.h"
#include "Styles.h"
#include "Analysis.h"
#include <cmath>

namespace forge
{
//==============================================================================
namespace Log
{
    static std::unique_ptr<juce::FileLogger> logger;
    static juce::CriticalSection lock;

    juce::File file()
    {
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("MPCSampleForge").getChildFile ("Logs").getChildFile ("forge.log");
    }

    static void crashHandler (void*)
    {
        // Best effort: we are in a crash, keep this minimal.
        write ("*** CRASH ***\n" + juce::SystemStats::getStackBacktrace());
    }

    void init (const juce::String& appName)
    {
        const juce::ScopedLock sl (lock);
        if (logger != nullptr) return;
        file().getParentDirectory().createDirectory();
        logger = std::make_unique<juce::FileLogger> (file(), appName + " log", 512 * 1024); // trimmed to 512 KB
        juce::SystemStats::setApplicationCrashHandler (crashHandler);
        logger->logMessage ("==== " + appName + " started " + juce::Time::getCurrentTime().toString (true, true)
                            + " on " + juce::SystemStats::getOperatingSystemName());
    }

    void write (const juce::String& message)
    {
        const juce::ScopedLock sl (lock);
        if (logger != nullptr)
            logger->logMessage (juce::Time::getCurrentTime().toString (false, true, true, true) + "  " + message);
    }

    void shutdown()
    {
        const juce::ScopedLock sl (lock);
        logger.reset();
    }
}

//==============================================================================
juce::String DiagResult::toString() const
{
    const char* tag = level == Level::Pass ? "PASS" : level == Level::Warn ? "WARN" : level == Level::Fail ? "FAIL" : "info";
    return juce::String ("[") + tag + "] " + area.paddedRight (' ', 24) + " " + message;
}

void DiagSummary::add (const DiagResult& r)
{
    if (r.level == DiagResult::Level::Pass) ++pass;
    if (r.level == DiagResult::Level::Warn) ++warn;
    if (r.level == DiagResult::Level::Fail) ++fail;
}

juce::String DiagSummary::toString() const
{
    return juce::String (pass) + " passed, " + juce::String (warn) + " warnings, " + juce::String (fail) + " failed"
           + (fail == 0 ? (warn == 0 ? "  -> all good" : "  -> working, see warnings") : "  -> problems found, please send this report");
}

//==============================================================================
juce::String systemReport (PluginHost& host, juce::AudioDeviceManager* devices,
                           juce::AudioProcessor* instrument, const juce::String& instrumentPath)
{
    juce::String r;
    auto line = [&r] (const juce::String& k, const juce::String& v) { r << "  " << k.paddedRight (' ', 20) << v << "\n"; };

    r << "MPC SAMPLE FORGE - DIAGNOSTICS REPORT\n"
      << "Generated " << juce::Time::getCurrentTime().toString (true, true) << "\n\n";

    r << "Application\n";
    line ("Version", juce::String (FORGE_VERSION_STRING));
    line ("JUCE", juce::SystemStats::getJUCEVersion());
    line ("Built", juce::String (__DATE__) + " " + __TIME__);
    line ("Executable", juce::File::getSpecialLocation (juce::File::currentExecutableFile).getFullPathName());
    line ("Log file", Log::file().getFullPathName());

    r << "\nSystem\n";
    line ("OS", juce::SystemStats::getOperatingSystemName() + (juce::SystemStats::isOperatingSystem64Bit() ? " (64-bit)" : ""));
    line ("CPU", juce::SystemStats::getCpuModel() + " / " + juce::String (juce::SystemStats::getNumCpus()) + " cores");
    line ("Memory", juce::String (juce::SystemStats::getMemorySizeInMegabytes()) + " MB");
    line ("Device", juce::SystemStats::getDeviceDescription());
    line ("Language", juce::SystemStats::getUserLanguage() + "-" + juce::SystemStats::getUserRegion());

    r << "\nAudio\n";
    if (devices == nullptr)
        line ("Device", "(command line: no audio device used)");
    else if (auto* d = devices->getCurrentAudioDevice())
    {
        line ("Type", d->getTypeName());
        line ("Device", d->getName());
        line ("Sample rate", juce::String (juce::roundToInt (d->getCurrentSampleRate())) + " Hz");
        line ("Buffer", juce::String (d->getCurrentBufferSizeSamples()) + " samples");
        line ("Outputs", juce::String (d->getActiveOutputChannels().countNumberOfSetBits()));
        line ("CPU usage", juce::String (devices->getCpuUsage() * 100.0, 1) + " %");
    }
    else
        line ("Device", "NONE OPEN - auditioning will be silent (rendering still works)");

    auto midi = juce::MidiInput::getAvailableDevices();
    juce::StringArray midiNames;
    for (auto& m : midi) midiNames.add (m.name);
    line ("MIDI inputs", midiNames.isEmpty() ? juce::String ("none") : midiNames.joinIntoString (", "));

    r << "\nVST3\n";
    for (auto* f : host.getFormatManager().getFormats())
        if (f->getName() == "VST3")
            line ("Search folders", f->getDefaultLocationsToSearch().toString().replace (";", "\n" + juce::String::repeatedString (" ", 22)));
    auto found = host.findInstalledVST3();
    line ("Plugins found", juce::String (found.size()));
    for (auto& p : found) r << "      " << p << "\n";

    r << "\nCurrent instrument\n";
    if (instrument == nullptr)
        line ("Instrument", "none");
    else
    {
        line ("Name", instrument->getName());
        line ("Path", instrumentPath.isEmpty() ? juce::String ("(built-in)") : instrumentPath);
        if (auto* inst = dynamic_cast<juce::AudioPluginInstance*> (instrument))
        {
            auto d = inst->getPluginDescription();
            line ("Manufacturer", d.manufacturerName);
            line ("Version", d.version);
            line ("Format", d.pluginFormatName);
            line ("Is instrument", d.isInstrument ? "yes" : "NO (this looks like an effect)");
        }
        line ("Inputs / outputs", juce::String (instrument->getTotalNumInputChannels()) + " / " + juce::String (instrument->getTotalNumOutputChannels()));
        line ("Accepts MIDI", instrument->acceptsMidi() ? "yes" : "NO");
        line ("Latency", juce::String (instrument->getLatencySamples()) + " samples");
        line ("Has editor", instrument->hasEditor() ? "yes" : "no");
        juce::MemoryBlock state;
        instrument->getStateInformation (state);
        line ("State size", juce::String ((int) state.getSize()) + " bytes");
    }

    r << "\nStyles: " << builtInStyles().size() << " built in.  Engines: " << engineIds().joinIntoString (", ") << "\n";
    return r;
}

//==============================================================================
namespace
{
    /** Fundamental by autocorrelation around the expected period (sub-sample accurate). */
    double estimatePitch (const juce::AudioBuffer<float>& b, double sr, double expectedHz)
    {
        auto start = (int) (0.02 * sr);
        auto n = juce::jmin (b.getNumSamples() - start, (int) (0.3 * sr));
        if (n < (int) (4 * sr / expectedHz)) return 0;

        std::vector<double> x ((size_t) n);
        for (int i = 0; i < n; ++i)
            x[(size_t) i] = 0.5 * (b.getSample (0, start + i) + b.getSample (juce::jmin (1, b.getNumChannels() - 1), start + i));

        auto period = sr / expectedHz;
        auto lo = juce::jmax (2, (int) (period * 0.94)), hi = (int) (period * 1.06) + 2;
        std::vector<double> r ((size_t) (hi + 2), 0.0);
        for (int lag = lo - 1; lag <= hi + 1; ++lag)
        {
            double s = 0;
            for (int i = 0; i + lag < n; ++i) s += x[(size_t) i] * x[(size_t) (i + lag)];
            r[(size_t) lag] = s / (n - lag);
        }
        int best = lo;
        for (int lag = lo; lag <= hi; ++lag) if (r[(size_t) lag] > r[(size_t) best]) best = lag;
        auto a = r[(size_t) best - 1], c = r[(size_t) best + 1], m = r[(size_t) best];
        auto denom = a - 2 * m + c;
        auto p = std::abs (denom) > 1e-12 ? 0.5 * (a - c) / denom : 0.0;
        return sr / (best + p);
    }

    bool isFinite (const juce::AudioBuffer<float>& b)
    {
        for (int ch = 0; ch < b.getNumChannels(); ++ch)
            for (int i = 0; i < b.getNumSamples(); ++i)
                if (! std::isfinite (b.getSample (ch, i))) return false;
        return true;
    }

    SampleSpec testNote (int note, double hold, int velocity = 110)
    {
        SampleSpec s;
        s.fileStem = "diag";
        s.kind = "Key";
        s.label = midiNoteName (note);
        s.rootNote = note;
        s.variation = 12345;
        NoteEvent e;
        e.note = note; e.velocity = velocity; e.startSec = 0; e.lengthSec = hold;
        s.notes.push_back (e);
        return s;
    }
}

void runSelfTest (std::function<void (const DiagResult&)> report, juce::AudioProcessor* plugin, const std::atomic<bool>& cancel)
{
    using L = DiagResult::Level;
    auto say = [&] (L level, const juce::String& area, const juce::String& msg)
    {
        DiagResult r { level, area, msg };
        Log::write ("diag " + r.toString());
        report (r);
    };

    RenderSettings rs;
    rs.warmUpSec = 0;
    const auto sr = rs.sampleRate;

    // ------------------------------------------------------------ planning
    {
        int ok = 0;
        juce::StringArray problems;
        for (auto& st : builtInStyles())
        {
            for (auto root : { -1, PlanOptions::styleRoot })
            {
                PlanOptions o; o.count = 100; o.seed = 7; o.builtIn = true; o.fixedRoot = root;
                juce::String err;
                auto specs = planSamples (st, o, err);
                if ((int) specs.size() == 100 && err.isEmpty()) ++ok;
                else problems.add (st.id + (root == -1 ? "" : " (16 Levels)") + ": " + juce::String ((int) specs.size()) + " samples " + err);
            }
        }
        if (problems.isEmpty()) say (L::Pass, "Sample planner", juce::String (ok) + " style/mode combinations each plan 100 unique samples");
        else for (auto& p : problems) say (L::Fail, "Sample planner", p);
    }
    if (cancel) return;

    // ------------------------------------------------------------ built-in engines
    {
        BuiltInSynth synth;
        OfflineRenderer renderer (synth, rs);
        for (auto& id : engineIds())
        {
            if (cancel) return;
            Engine e;
            parseEngine (id, e);
            synth.setEngine (e);
            synth.setVariation (12345);

            juce::AudioBuffer<float> audio;
            juce::String err;
            auto t0 = juce::Time::getMillisecondCounterHiRes();
            auto spec = testNote (60, 0.4);
            if (! renderer.render (spec, audio, err)) { say (L::Fail, "Engine " + id, err); continue; }
            auto ms = juce::Time::getMillisecondCounterHiRes() - t0;

            if (! isFinite (audio)) { say (L::Fail, "Engine " + id, "audio contains NaN / infinity"); continue; }

            auto peakDb = juce::Decibels::gainToDecibels (audio.getMagnitude (0, audio.getNumSamples()));
            juce::String info = juce::String (juce::roundToInt (audio.getNumSamples() / sr * 1000.0)) + " ms, peak " + juce::String (peakDb, 1)
                                + " dB, rendered in " + juce::String (juce::roundToInt (ms)) + " ms";

            // bell and cowbell are inharmonic by design: no single-period pitch to measure
            if (e == Engine::Bell || e == Engine::Cowbell)
            {
                say (L::Pass, "Engine " + id, info);
                continue;
            }

            auto f0 = estimatePitch (audio, sr, 261.6256);
            auto cents = f0 > 0 ? 1200.0 * std::log2 (f0 / 261.6256) : 9999.0;
            auto tolerance = (e == Engine::Supersaw || e == Engine::Choir || e == Engine::Organ) ? 25.0 : 10.0;
            say (std::abs (cents) <= tolerance ? L::Pass : L::Warn, "Engine " + id,
                 info + ", tuning " + (f0 > 0 ? juce::String (cents, 1) + " cents" : juce::String ("not measurable")));
        }
    }
    if (cancel) return;

    // ------------------------------------------------------------ recipes (FX chains + lengths)
    {
        BuiltInSynth synth;
        OfflineRenderer renderer (synth, rs);
        for (auto& st : builtInStyles())
        {
            if (cancel) return;
            if (st.isKit() || st.processing.isEmpty()) continue;

            PlanOptions o; o.count = 1; o.seed = 3; o.builtIn = true; o.fixedRoot = PlanOptions::styleRoot;
            juce::String err;
            auto specs = planSamples (st, o, err);
            if (specs.empty()) { say (L::Fail, "Recipe " + st.tag, "could not plan: " + err); continue; }

            Engine e;
            if (parseEngine (specs[0].engine, e)) synth.setEngine (e);
            synth.setVariation (specs[0].variation);

            juce::AudioBuffer<float> audio;
            if (! renderer.render (specs[0], audio, err)) { say (L::Fail, "Recipe " + st.tag, err); continue; }
            if (! isFinite (audio)) { say (L::Fail, "Recipe " + st.tag, "audio contains NaN / infinity"); continue; }

            auto lenMs = audio.getNumSamples() / sr * 1000.0;
            auto& P = st.processing;
            auto maxMs = (P.maxLength > 0 ? P.maxLength * 1000.0 : 1e9) + P.fxTailMs + 2.0;
            auto peak = audio.getMagnitude (0, audio.getNumSamples());
            auto ok = lenMs <= maxMs && peak > 0.1f && peak <= 1.0f;
            say (ok ? L::Pass : L::Fail, "Recipe " + st.tag,
                 juce::String (juce::roundToInt (lenMs)) + " ms (end point " + juce::String (juce::roundToInt (P.maxLength * 1000.0)) + " ms"
                 + (P.fxTailMs > 0 ? " + " + juce::String (juce::roundToInt (P.fxTailMs)) + " ms tail" : juce::String()) + "), "
                 + juce::String (P.fx.isArray() ? P.fx.size() : 0) + " FX, peak " + juce::String (juce::Decibels::gainToDecibels (peak), 1) + " dB");
        }
    }
    if (cancel) return;

    // ------------------------------------------------------------ Smart Pick
    {
        BuiltInSynth synth;
        OfflineRenderer renderer (synth, rs);
        auto* st = findBuiltInStyle ("one-shot-palette");
        PlanOptions o; o.count = 24; o.seed = 21; o.builtIn = true; o.fixedRoot = PlanOptions::styleRoot;
        juce::String err;
        auto specs = st != nullptr ? planSamples (*st, o, err) : std::vector<SampleSpec>();
        std::vector<PickCandidate> cands;
        for (auto& sp : specs)
        {
            Engine e;
            if (parseEngine (sp.engine, e)) synth.setEngine (e);
            synth.setVariation (sp.variation);
            juce::AudioBuffer<float> audio;
            PickCandidate c;
            c.valid = renderer.render (sp, audio, err);
            if (c.valid) c.features = analyseSample (audio, sr, sp.rootNote, sp.kind == "Key");
            c.rootNote = sp.rootNote;
            c.group = sp.kind;
            cands.push_back (c);
        }
        // plus a deliberately muddy chord: Smart Pick must mark it down
        PickCandidate muddy = cands.empty() ? PickCandidate() : cands.front();
        muddy.lowIntervalViolations = lowIntervalViolations ({ 40, 43, 47 }, &muddy.lowIntervalDetail);
        cands.push_back (muddy);

        auto verdicts = smartPick (cands, 8);
        int kept = 0;
        for (auto& v : verdicts) kept += v.kept ? 1 : 0;
        auto muddyMarked = ! verdicts.empty() && verdicts.back().score < 100 && verdicts.back().reasons.joinIntoString (" ").contains ("muddy");
        bool finite = true;
        for (auto& c : cands) for (auto x : c.features.fingerprint) finite = finite && std::isfinite (x);

        say (kept == 8 && muddyMarked && finite ? L::Pass : L::Fail, "Smart Pick",
             "picked " + juce::String (kept) + " of " + juce::String ((int) cands.size()) + " candidates"
             + (muddyMarked ? ", muddy test chord caught (" + verdicts.back().reasons.joinIntoString ("; ") + ")" : juce::String (", muddy test chord NOT caught"))
             + (finite ? juce::String() : juce::String (", NaN in analysis")));
    }
    if (cancel) return;

    // ------------------------------------------------------------ WAV round trip
    {
        auto tmp = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("mpcforge_diag.wav");
        juce::AudioBuffer<float> b (2, 4410);
        for (int i = 0; i < b.getNumSamples(); ++i)
        {
            auto v = 0.5f * (float) std::sin (juce::MathConstants<double>::twoPi * 440.0 * i / 44100.0);
            b.setSample (0, i, v); b.setSample (1, i, v);
        }
        juce::String err;
        if (! OfflineRenderer::writeWav (tmp, b, 44100.0, 24, 57, err))
            say (L::Fail, "WAV writing", err + " (temp folder: " + tmp.getParentDirectory().getFullPathName() + ")");
        else
        {
            juce::WavAudioFormat wav;
            std::unique_ptr<juce::AudioFormatReader> reader (wav.createReaderFor (tmp.createInputStream().release(), true));
            if (reader == nullptr) say (L::Fail, "WAV writing", "file written but could not be read back");
            else
            {
                auto root = reader->metadataValues.getValue ("MidiUnityNote", "?");
                auto good = reader->lengthInSamples == 4410 && reader->bitsPerSample == 24 && reader->numChannels == 2 && root == "57";
                say (good ? L::Pass : L::Fail, "WAV writing",
                     juce::String (reader->bitsPerSample) + "-bit, " + juce::String (reader->numChannels) + " ch, "
                     + juce::String (reader->lengthInSamples) + " samples, root note " + root);
            }
            tmp.deleteFile();
        }
    }
    if (cancel) return;

    // ------------------------------------------------------------ the loaded plugin
    if (plugin != nullptr && dynamic_cast<BuiltInSynth*> (plugin) == nullptr)
    {
        auto name = plugin->getName();
        if (! plugin->acceptsMidi()) say (L::Fail, "Plugin", name + " does not accept MIDI - it is probably an effect, not an instrument");

        RenderSettings prs;
        prs.warmUpSec = 1.5;
        OfflineRenderer renderer (*plugin, prs);
        auto t0 = juce::Time::getMillisecondCounterHiRes();
        renderer.warmUp();
        say (L::Info, "Plugin", name + ": prepared + warm-up in " + juce::String (juce::roundToInt (juce::Time::getMillisecondCounterHiRes() - t0))
                                 + " ms, latency " + juce::String (plugin->getLatencySamples()) + " samples");

        for (auto note : { 48, 60, 72 })
        {
            if (cancel) return;
            juce::AudioBuffer<float> audio;
            juce::String err;
            auto start = juce::Time::getMillisecondCounterHiRes();
            if (! renderer.render (testNote (note, 0.5), audio, err))
            {
                say (L::Fail, "Plugin " + midiNoteName (note), err);
                continue;
            }
            auto ms = juce::Time::getMillisecondCounterHiRes() - start;
            auto lenSec = audio.getNumSamples() / prs.sampleRate;
            if (! isFinite (audio)) { say (L::Fail, "Plugin " + midiNoteName (note), "audio contains NaN / infinity"); continue; }

            auto f0 = estimatePitch (audio, prs.sampleRate, juce::MidiMessage::getMidiNoteInHertz (note));
            auto cents = f0 > 0 ? 1200.0 * std::log2 (f0 / juce::MidiMessage::getMidiNoteInHertz (note)) : 0.0;
            auto speed = ms > 0 ? lenSec * 1000.0 / ms : 0.0;
            say (speed < 1.0 ? L::Warn : L::Pass, "Plugin " + midiNoteName (note),
                 juce::String (lenSec, 2) + " s of audio in " + juce::String (juce::roundToInt (ms)) + " ms (" + juce::String (speed, 1) + "x real time)"
                 + (f0 > 0 && std::abs (cents) < 60 ? ", pitch " + juce::String (cents, 1) + " cents"
                                                    : juce::String (", pitch not measurable (fine for pads / noisy patches)")));
        }

        juce::MemoryBlock state;
        plugin->getStateInformation (state);
        plugin->setStateInformation (state.getData(), (int) state.getSize());
        say (L::Pass, "Plugin state", "saved and restored " + juce::String ((int) state.getSize()) + " bytes (needed for .forgesound files)");
    }
    else
        say (L::Info, "Plugin", "no VST3 loaded - load one and run the self-test again to check it");
}
}
