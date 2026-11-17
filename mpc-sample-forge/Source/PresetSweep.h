#pragma once
#include "BatchJob.h"
#include "Analysis.h"
#include <memory>

namespace forge
{
    /** One sound a sweep can try: a plugin program, a preset file, a variation of the
        current sound, or (built-in synth) an engine + timbre seed. */
    struct PresetEntry
    {
        juce::String name;
        int program = -1;                                  // plugin program number
        juce::File file;                                   // .vstpreset or .forgesound
        juce::String matchText;                            // what the name filters look at (name + category folders)
        std::shared_ptr<juce::MemoryBlock> baseState;      // variations: the sound they start from
        std::vector<std::pair<int, float>> paramValues;    // variations: parameter index -> new value
        juce::String engine;                               // built-in synth
        juce::int64 variation = 0;
    };

    enum class SweepSource { Programs, PresetFolder, Variations };

    struct SweepOptions
    {
        SweepSource source = SweepSource::Programs;
        juce::File folder;            // PresetFolder: .vstpreset / .forgesound files (empty = the standard VST3 preset folders)
        int variations = 48;          // Variations: how many to try
        float strength = 0.25f;       // Variations: how far each parameter may move (0..1 of its range)
        int maxPresets = 0;           // try at most this many, spread over the list (0 = all)
        int keep = 12;                // presets to keep (at most)
        float minScore = 70.0f;       // ... and only those scoring at least this (the best one is always kept)
        int perPreset = 16;           // samples rendered for each kept preset
        juce::String include;         // only names containing one of these words (comma separated); empty = all
        bool skipByName = true;       // skip arps / sequences / drums / FX / init patches by name
        bool skipRhythmic = true;     // skip presets whose test chord pulses (arps, gates, sequences)
        bool keepProbes = true;       // write every preset's test render to _Preset_Probes (to listen through them)
    };

    /** Lists the presets to try. Call on the message thread. */
    std::vector<PresetEntry> collectPresets (juce::AudioProcessor& instrument, const SweepOptions& o, juce::int64 seed,
                                             juce::String& error);

    /** Loads a preset into an instrument. Call on the message thread. */
    bool applyPreset (juce::AudioProcessor& instrument, const PresetEntry& e, juce::String& error);

    /** Saves each of the plugin's own presets (programs) as a .vstpreset file (or .forgesound if the
        plugin isn't a VST3). Returns how many were written. Call on the message thread. */
    int exportPresets (juce::AudioProcessor& instrument, const juce::String& pluginPath, const RenderSettings& settings,
                       const juce::File& folder, juce::String& error);

    /** Folders that may hold .vstpreset files for this plugin (vendor/plugin folders under the standard locations). */
    juce::Array<juce::File> standardPresetFolders (const juce::String& pluginName);

    /** True if a preset name suggests it won't make a good melodic one-shot (arp, drum kit, FX, init ...). */
    bool nameLooksUnsuitable (const juce::String& text, juce::String& why);

    /** How many times the sound swells up again while the key is held (arps, gates, sequences: 2+). */
    int countRepeatedOnsets (const juce::AudioBuffer<float>& audio, double sampleRate, double untilSec);

    /** Milliseconds from the start until the sound first reaches -6 dB of its peak. */
    double attackMs (const juce::AudioBuffer<float>& audio, double sampleRate);

    struct ProbeResult
    {
        PresetEntry entry;
        bool probed = false;          // false = skipped before rendering (name filter / limit)
        bool kept = false;
        float score = 0;
        juce::StringArray reasons;    // why it was skipped or lost points
        juce::String folderName;      // kept: the batch folder it was rendered into
    };

    /** A whole preset sweep, driven from the message thread:
        1. renders one test chord / note per preset (on a background thread),
        2. scores them (silence, pulsing arps, unclear pitch, harshness, clicks, slow attack)
           and keeps the best, most different ones (Smart Pick),
        3. renders a normal batch (16-pad bank) for every kept preset, one after another.
        Writes _Preset_Sweep.txt and _preset_sweep.csv to the sweep folder. */
    class PresetSweep
    {
    public:
        /** `base` holds the style, plan options, render settings and output folder of the sweep;
            each kept preset gets a sub folder. Instruments must outlive the sweep. */
        PresetSweep (juce::AudioProcessor& instrument, std::vector<juce::AudioProcessor*> extraInstances,
                     BatchRequest base, SweepOptions options);
        ~PresetSweep();

        std::function<void (const juce::String&)> onLog;
        std::function<void (double)> onProgress;   // 0..1 for the whole sweep
        std::function<void (bool ok, const juce::String& summary, const juce::File& folder)> onFinished;

        /** Collects the presets and starts. Returns false (with a reason) if there is nothing to try. */
        bool start (juce::String& error);
        void cancel();

        const std::vector<ProbeResult>& results() const { return probeResults; }

    private:
        class ProbeThread;
        void probesFinished (bool ok, const juce::String& message);
        void renderNext();
        void finish (bool ok, const juce::String& summary);
        void writeReport() const;
        void log (const juce::String& s) { if (onLog) onLog (s); }
        void progress (double p) { if (onProgress) onProgress (juce::jlimit (0.0, 1.0, p)); }

        juce::AudioProcessor& instrument;
        std::vector<juce::AudioProcessor*> extras;
        BatchRequest base;
        SweepOptions options;
        std::vector<ProbeResult> probeResults;
        std::vector<int> keptOrder;                 // indices into probeResults, best first
        size_t nextKept = 0;
        int probeCount = 0, renderedSamples = 0;
        juce::StringArray batchSummaries;
        std::unique_ptr<ProbeThread> probeThread;
        std::unique_ptr<BatchJob> job;
        std::shared_ptr<std::atomic<bool>> alive = std::make_shared<std::atomic<bool>> (true);
        bool cancelled = false, finished = false;
        double startedMs = 0;
    };
}
