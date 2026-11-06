#include "OfflineRenderer.h"

namespace forge
{
OfflineRenderer::OfflineRenderer (juce::AudioProcessor& p, const RenderSettings& s)
    : proc (p), settings (s)
{
    // Prefer a stereo main output; many instruments also expose extra outs.
    if (proc.getBusCount (false) > 0)
    {
        auto layout = proc.getBusesLayout();
        layout.getChannelSet (false, 0) = juce::AudioChannelSet::stereo();
        if (proc.checkBusesLayoutSupported (layout))
            proc.setBusesLayout (layout);
    }

    proc.setNonRealtime (true);
    proc.setPlayConfigDetails (proc.getTotalNumInputChannels(), proc.getTotalNumOutputChannels(),
                               settings.sampleRate, settings.blockSize);
    proc.prepareToPlay (settings.sampleRate, settings.blockSize);

    outChannels = juce::jlimit (1, 2, proc.getMainBusNumOutputChannels() > 0 ? proc.getMainBusNumOutputChannels()
                                                                             : proc.getTotalNumOutputChannels());
    block.setSize (juce::jmax (2, proc.getTotalNumInputChannels(), proc.getTotalNumOutputChannels()), settings.blockSize);
}

OfflineRenderer::~OfflineRenderer()
{
    proc.releaseResources();
    proc.setNonRealtime (false);
}

void OfflineRenderer::processBlock (juce::MidiBuffer& midi)
{
    block.clear();
    const juce::ScopedLock sl (proc.getCallbackLock());

    if (proc.isSuspended())
        return;

    proc.processBlock (block, midi);
}

float OfflineRenderer::blockPeak() const
{
    float peak = 0;
    for (int ch = 0; ch < outChannels; ++ch)
        peak = juce::jmax (peak, block.getMagnitude (ch, 0, block.getNumSamples()));
    return peak;
}

void OfflineRenderer::warmUp()
{
    // Some plugins (Kontakt-style samplers) stream content in the background:
    // give them time, in real time, and feed them silent blocks meanwhile.
    auto end = juce::Time::getMillisecondCounterHiRes() + settings.warmUpSec * 1000.0;
    juce::MidiBuffer none;
    while (juce::Time::getMillisecondCounterHiRes() < end)
    {
        processBlock (none);
        juce::Thread::sleep (2);
    }
    flush();
}

void OfflineRenderer::flush()
{
    // Kill anything still ringing so samples never bleed into each other.
    juce::MidiBuffer off;
    for (int ch = 1; ch <= 16; ++ch)
    {
        off.addEvent (juce::MidiMessage::allNotesOff (ch), 0);
        off.addEvent (juce::MidiMessage::allSoundOff (ch), 0);
        off.addEvent (juce::MidiMessage::controllerEvent (ch, 64, 0), 0);
    }
    processBlock (off);

    auto threshold = juce::Decibels::decibelsToGain ((float) settings.silenceDb - 10.0f);
    auto quietNeeded = (int) (0.2 * settings.sampleRate);
    auto maxSamples = (int) (5.0 * settings.sampleRate);
    int quiet = 0;
    juce::MidiBuffer none;

    for (int done = 0; done < maxSamples && quiet < quietNeeded; done += settings.blockSize)
    {
        processBlock (none);
        quiet = blockPeak() < threshold ? quiet + settings.blockSize : 0;
    }
}

bool OfflineRenderer::render (const SampleSpec& spec, juce::AudioBuffer<float>& out, juce::String& error)
{
    flush();

    struct Ev { juce::int64 pos; juce::MidiMessage msg; };
    std::vector<Ev> events;
    juce::int64 lastOff = 0;

    for (auto& n : spec.notes)
    {
        auto on  = (juce::int64) (n.startSec * settings.sampleRate);
        auto off = (juce::int64) ((n.startSec + n.lengthSec) * settings.sampleRate);
        events.push_back ({ on,  juce::MidiMessage::noteOn (1, n.note, (juce::uint8) n.velocity) });
        events.push_back ({ off, juce::MidiMessage::noteOff (1, n.note, (juce::uint8) 64) });
        lastOff = juce::jmax (lastOff, off);
    }
    std::stable_sort (events.begin(), events.end(), [] (auto& a, auto& b) { return a.pos < b.pos; });

    auto latency = (juce::int64) juce::jmax (0, proc.getLatencySamples());
    auto maxLen  = (juce::int64) (settings.maxLengthSec * settings.sampleRate) + latency;
    auto tailEnd = lastOff + (juce::int64) (settings.maxTailSec * settings.sampleRate) + latency;
    auto threshold = juce::Decibels::decibelsToGain ((float) settings.silenceDb);
    auto quietNeeded = (juce::int64) (0.3 * settings.sampleRate);

    juce::AudioBuffer<float> captured (2, (int) juce::jmin (maxLen, tailEnd) + settings.blockSize);
    captured.clear();

    juce::int64 pos = 0, quiet = 0, written = 0;
    size_t nextEvent = 0;
    const auto bs = settings.blockSize;

    while (pos < maxLen && pos < tailEnd)
    {
        juce::MidiBuffer midi;
        while (nextEvent < events.size() && events[nextEvent].pos < pos + bs)
        {
            midi.addEvent (events[nextEvent].msg, (int) (events[nextEvent].pos - pos));
            ++nextEvent;
        }

        processBlock (midi);

        // copy, skipping the plugin's reported latency
        auto skip = juce::jlimit<juce::int64> (0, bs, latency - pos);
        auto n = (int) (bs - skip);
        if (n > 0 && written + n <= captured.getNumSamples())
        {
            for (int ch = 0; ch < 2; ++ch)
                captured.copyFrom (ch, (int) written, block, juce::jmin (ch, outChannels - 1), (int) skip, n);
            written += n;
        }

        pos += bs;

        if (pos > lastOff + latency)
        {
            quiet = blockPeak() < threshold ? quiet + bs : 0;
            if (quiet >= quietNeeded)
                break;
        }
    }

    // ---- trim leading / trailing silence
    auto total = (int) written;
    auto onsetThreshold = juce::Decibels::decibelsToGain (-60.0f);
    int first = -1, last = -1;

    for (int i = 0; i < total && first < 0; ++i)
        if (std::abs (captured.getSample (0, i)) > onsetThreshold || std::abs (captured.getSample (1, i)) > onsetThreshold)
            first = i;

    if (first < 0)
    {
        error = "the instrument produced no sound (is a preset loaded? try a longer --warmup)";
        return false;
    }

    for (int i = total - 1; i > first && last < 0; --i)
        if (std::abs (captured.getSample (0, i)) > threshold || std::abs (captured.getSample (1, i)) > threshold)
            last = i;
    if (last < 0) last = total - 1;

    first = juce::jmax (0, first - 8);
    auto length = juce::jmin (total - first, last - first + 1 + (int) (0.01 * settings.sampleRate));

    out.setSize (2, length);
    for (int ch = 0; ch < 2; ++ch)
        out.copyFrom (ch, 0, captured, ch, first, length);

    // ---- tiny fade-in (avoids a click where we trimmed) and a fade-out
    auto fadeIn = juce::jmin (length, 16);
    if (first > 0)
        out.applyGainRamp (0, fadeIn, 0.0f, 1.0f);

    auto fadeOut = juce::jmin (length / 2, (int) (settings.fadeOutMs * 0.001 * settings.sampleRate));
    out.applyGainRamp (length - fadeOut, fadeOut, 1.0f, 0.0f);

    // ---- normalise
    if (settings.normalise)
    {
        auto peak = out.getMagnitude (0, length);
        if (peak > 0.0f)
            out.applyGain (juce::Decibels::decibelsToGain (settings.normaliseDb) / peak);
    }
    else
    {
        // never let a hot plugin clip the file
        auto peak = out.getMagnitude (0, length);
        if (peak > 0.999f)
            out.applyGain (0.999f / peak);
    }

    return true;
}

bool OfflineRenderer::writeWav (const juce::File& file, const juce::AudioBuffer<float>& audio,
                                double sampleRate, int bitDepth, int rootNote, juce::String& error)
{
    file.deleteFile();
    auto stream = std::make_unique<juce::FileOutputStream> (file);
    if (! stream->openedOk())
    {
        error = "cannot write " + file.getFullPathName();
        return false;
    }

    // Root note goes into the 'smpl' / 'inst' chunks so samplers (MPC keygroups
    // included) know the pitch of the file.
    juce::StringPairArray meta;
    meta.set ("MidiUnityNote", juce::String (rootNote));
    meta.set ("SamplePeriod", juce::String ((int) std::round (1.0e9 / sampleRate)));
    meta.set ("NumSampleLoops", "0");

    juce::WavAudioFormat wav;
    std::unique_ptr<juce::AudioFormatWriter> writer (wav.createWriterFor (stream.get(), sampleRate,
                                                                          (unsigned int) audio.getNumChannels(),
                                                                          bitDepth, meta, 0));
    if (writer == nullptr)
    {
        error = "unsupported WAV format (" + juce::String (sampleRate) + " Hz / " + juce::String (bitDepth) + " bit)";
        return false;
    }
    stream.release(); // owned by the writer now

    if (! writer->writeFromAudioSampleBuffer (audio, 0, audio.getNumSamples()))
    {
        error = "failed writing " + file.getFullPathName();
        return false;
    }
    return true;
}
}
