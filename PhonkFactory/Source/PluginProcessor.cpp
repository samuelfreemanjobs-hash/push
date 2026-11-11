#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace phonk;

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PhonkProcessor::createLayout()
{
    using F = juce::AudioParameterFloat;
    using I = juce::AudioParameterInt;
    using R = juce::NormalisableRange<float>;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    auto f = [&] (const char* id, const char* name, float lo, float hi, float def, float step = 0.001f, float centre = 0.f, const char* unit = "")
    {
        R r (lo, hi, step);
        if (centre > 0.f) r.setSkewForCentre (centre);
        p.push_back (std::make_unique<F> (juce::ParameterID { id, 1 }, name, r, def, juce::AudioParameterFloatAttributes().withLabel (unit)));
    };
    auto i = [&] (const char* id, const char* name, int lo, int hi, int def)
    { p.push_back (std::make_unique<I> (juce::ParameterID { id, 1 }, name, lo, hi, def)); };
    auto b = [&] (const char* id, const char* name, bool def)
    { p.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { id, 1 }, name, def)); };

    // kit
    i (ids::kitSeed, "Kit Seed", 1, 9999, 1);
    f (ids::kickTune, "Kick Tune", -12.f, 12.f, 0.f, 0.01f, 0.f, " st");
    f (ids::kickDecay, "Kick Decay", 0.5f, 2.f, 1.f);
    f (ids::snareTune, "Snare Tune", -12.f, 12.f, 0.f, 0.01f, 0.f, " st");
    f (ids::snareDecay, "Snare Decay", 0.5f, 2.f, 1.f);
    f (ids::hatDecay, "Hat Decay", 0.5f, 2.f, 1.f);
    f (ids::cowTune, "Cowbell Tune", -12.f, 12.f, 0.f, 0.01f, 0.f, " st");
    f (ids::grit, "Grit", 0.2f, 2.f, 1.f);
    i (ids::bitDepth, "Bit Depth", 6, 16, 12);
    f (ids::hiss, "Kit Hiss", 0.f, 0.02f, 0.004f, 0.0001f);
    f (ids::tone, "Kit Tone", 0.5f, 1.5f, 1.f);
    // bus
    f (ids::busDrive, "Tape Drive", 1.f, 6.f, 2.4f);
    f (ids::busHiss, "Tape Hiss", 0.f, 0.02f, 0.005f, 0.0001f);
    f (ids::busTone, "Tape LPF", 2000.f, 16000.f, 8800.f, 1.f, 6000.f, " Hz");
    f (ids::wow, "Wow & Flutter", 0.f, 1.f, 0.5f);
    f (ids::master, "Master", -24.f, 6.f, -3.f, 0.1f, 0.f, " dB");
    // mix
    f (ids::lvlKick, "Kick Level", 0.f, 1.5f, 1.f);
    f (ids::lvlSnare, "Snare Level", 0.f, 1.5f, 1.f);
    f (ids::lvlHat, "Hat Level", 0.f, 1.5f, 1.f);
    f (ids::lvlCow, "Cowbell Level", 0.f, 1.5f, 1.f);
    // sequencer
    f (ids::bpm, "BPM", 60.f, 200.f, 116.f, 0.1f);
    f (ids::swing, "Swing", 0.f, 0.3f, 0.03f);
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { ids::bars, 1 }, "Bars",
                                                               juce::StringArray { "1", "2", "4" }, 1));
    f (ids::variation, "Variation", 0.f, 1.f, 0.5f);
    f (ids::rolls, "Hat Rolls", 0.f, 1.f, 0.3f);
    f (ids::cowbell, "Cowbell", 0.f, 1.f, 0.2f);
    i (ids::patSeed, "Pattern Seed", 1, 9999, 1);
    b (ids::run, "Play", false);
    b (ids::hostSync, "Host Sync", true);
    return { p.begin(), p.end() };
}

//==============================================================================
PhonkProcessor::PhonkProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PHONK", createLayout())
{
    for (auto& fl : flash) fl.store (0.f);
    for (auto* prm : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (prm))
            apvts.addParameterListener (rp->paramID, this);
    bus.prepare (44100.0);
    publishPattern();
    startTimer (30);
}

PhonkProcessor::~PhonkProcessor()
{
    stopTimer();
    for (auto* prm : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (prm))
            apvts.removeParameterListener (rp->paramID, this);
    worker.stopThread (5000);
    exportThread.stopThread (15000);
}

bool PhonkProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto out = l.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo()) && l.getMainInputChannelSet().isDisabled();
}

void PhonkProcessor::prepareToPlay (double sr, int)
{
    sampleRateAtomic = sr;
    bus.prepare (sr);
    lastBus = busSettings();
    bus.setSettings (lastBus);
    for (auto& v : voices) v.active = false;
    wasRunning = false; internalSteps = lastEndSteps = 0;
    kitDirty = true;               // re-render the one-shots at the device rate
}

//==============================================================================
phonk::KitSettings PhonkProcessor::kitSettings() const
{
    KitSettings k;
    k.seed = (int) raw (ids::kitSeed);
    k.kickTune = raw (ids::kickTune);   k.kickDecay = raw (ids::kickDecay);
    k.snareTune = raw (ids::snareTune); k.snareDecay = raw (ids::snareDecay);
    k.hatDecay = raw (ids::hatDecay);   k.cowTune = raw (ids::cowTune);
    k.grit = raw (ids::grit);           k.bitDepth = raw (ids::bitDepth);
    k.hiss = raw (ids::hiss);           k.tone = raw (ids::tone);
    return k;
}

phonk::PatternSettings PhonkProcessor::patternSettings() const
{
    PatternSettings s;
    s.seed = (int) raw (ids::patSeed);
    s.bars = 1 << (int) raw (ids::bars);        // choice index 0,1,2 -> 1,2,4 bars
    s.variation = raw (ids::variation);
    s.rolls = raw (ids::rolls);
    s.cowbell = raw (ids::cowbell);
    return s;
}

phonk::BusSettings PhonkProcessor::busSettings() const
{
    BusSettings b;
    b.drive = raw (ids::busDrive); b.bitDepth = raw (ids::bitDepth); b.hiss = raw (ids::busHiss);
    b.lpfHz = raw (ids::busTone);  b.wow = raw (ids::wow);
    return b;
}

phonk::MixSettings PhonkProcessor::mixSettings() const
{
    MixSettings m;
    m.level[Kick] = raw (ids::lvlKick);  m.level[Snare] = raw (ids::lvlSnare);
    m.level[ClosedHat] = m.level[OpenHat] = raw (ids::lvlHat);
    m.level[Cowbell] = raw (ids::lvlCow);
    return m;
}

void PhonkProcessor::parameterChanged (const juce::String& id, float)
{
    if (id == ids::kitSeed || id == ids::kickTune || id == ids::kickDecay || id == ids::snareTune || id == ids::snareDecay
        || id == ids::hatDecay || id == ids::cowTune || id == ids::grit || id == ids::bitDepth || id == ids::hiss || id == ids::tone)
        kitDirty = true;
    else if (id == ids::patSeed || id == ids::bars || id == ids::variation || id == ids::rolls || id == ids::cowbell)
        patternDirty = true;
}

void PhonkProcessor::publishPattern()
{
    patternForUi = generatePattern (patternSettings());
    patternSlot.publish (new Pattern (patternForUi));
}

void PhonkProcessor::timerCallback()
{
    if (patternDirty.exchange (false)) publishPattern();
    if (kitDirty.load() && ! worker.isThreadRunning())
    {
        kitDirty = false;
        worker.startThread();
    }
    kitSlot.collectGarbage();
    patternSlot.collectGarbage();
}

void PhonkProcessor::Worker::run()
{
    auto* kit = new Kit (renderKit (owner.kitSettings(), owner.sampleRateAtomic.load()));
    if (threadShouldExit()) { delete kit; return; }
    owner.kitSlot.publish (kit);
}

void PhonkProcessor::decayFlashes (float amount)
{
    for (auto& f : flash) f.store (juce::jmax (0.f, f.load() - amount));
}

static void setIntParam (juce::AudioProcessorValueTreeState& s, const char* id, int v)
{
    if (auto* p = s.getParameter (id)) p->setValueNotifyingHost (p->convertTo0to1 ((float) v));
}
void PhonkProcessor::rerollKit()    { setIntParam (apvts, ids::kitSeed, juce::Random::getSystemRandom().nextInt ({ 1, 10000 })); }
void PhonkProcessor::newPattern()   { setIntParam (apvts, ids::patSeed, juce::Random::getSystemRandom().nextInt ({ 1, 10000 })); }

//==============================================================================
void PhonkProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const double sr = getSampleRate();
    buffer.clear();
    keyboardState.processNextMidiBuffer (midi, 0, n, true);

    Kit* kit = kitSlot.acquire();
    Pattern* pat = patternSlot.acquire();
    if (kitSlot.takeSwapped()) for (auto& v : voices) v.active = false;   // old sample memory is about to be retired
    if (kit != nullptr && std::abs (kit->sampleRate - sr) > 1.0) kit = nullptr;   // wrong rate: wait for re-render
    if (kit == nullptr) { midi.clear(); return; }

    // ---- gather triggers: MIDI pads + sequencer ----
    int nTrig = 0;
    auto addTrigger = [&] (int sample, int pad, float vel)
    {
        if (nTrig < kMaxTriggers) triggers[(size_t) nTrig++] = { juce::jlimit (0, n - 1, sample), pad, vel };
    };
    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();
        if (m.isNoteOn())
        {
            const int pad = m.getNoteNumber() - kFirstMidiNote;
            if (pad >= 0 && pad < kNumPads) addTrigger (meta.samplePosition, pad, m.getFloatVelocity());
        }
    }
    midi.clear();

    bool useHost = false, running = false;
    double tempo = raw (ids::bpm), startSteps = 0;
    if (raw (ids::hostSync) > 0.5f)
        if (auto* ph = getPlayHead())
            if (auto pos = ph->getPosition())
                if (auto ppq = pos->getPpqPosition())
                {
                    useHost = true;
                    running = pos->getIsPlaying();
                    startSteps = *ppq * 4.0;                       // 16th-note steps
                    if (auto hb = pos->getBpm()) tempo = *hb;
                }
    if (! useHost) { running = raw (ids::run) > 0.5f; startSteps = internalSteps; }

    if (running && pat != nullptr && tempo > 1.0)
    {
        const double spp = tempo / 60.0 * 4.0 / sr;                // steps per sample
        if (wasRunning && std::abs (startSteps - lastEndSteps) < 0.05) startSteps = lastEndSteps;   // keep continuity
        const double total = pat->totalSteps(), swing = raw (ids::swing);
        double a = std::fmod (startSteps, total);
        if (a < 0) a += total;
        for (const auto& e : pat->events)
        {
            const double t = std::fmod (eventTime (e, swing), total);
            for (int k = 0; k < 2; ++k)                            // k = 1 catches events just after the loop wraps
            {
                // ceil with a tiny bias: an event lying exactly on a sample boundary resolves identically
                // whichever block it is computed from, so each hit fires exactly once at the same sample.
                const int smp = (int) std::ceil ((t + k * total - a) / spp - 1e-6);
                if (smp >= 0 && smp < n) addTrigger (smp, padIndex (e.voice, e.variation), e.velocity);
            }
        }
        wasRunning = true;
        lastEndSteps = startSteps + n * spp;
        if (! useHost) internalSteps = lastEndSteps;
        stepForUi.store ((float) a);
    }
    else
    {
        wasRunning = false;
        if (! useHost) internalSteps = 0;
        stepForUi.store (-1.f);
    }
    std::sort (triggers.begin(), triggers.begin() + nTrig, [] (const Trigger& x, const Trigger& y) { return x.sample < y.sample; });

    // ---- render ----
    const auto mix = mixSettings();
    const auto bs = busSettings();
    if (! (bs == lastBus)) { bus.setSettings (bs); lastBus = bs; }
    const float master = juce::Decibels::decibelsToGain (raw (ids::master));
    const float chokeStep = 1.f / (0.006f * (float) sr);

    auto fire = [&] (int pad, float vel)
    {
        const auto& smp = kit->samples[(size_t) pad];
        if (smp.empty()) return;
        const int group = pad / kVariations;
        if (group == ClosedHat || group == OpenHat)                // hats choke any ringing open hat
            for (auto& v : voices) if (v.active && v.group == OpenHat) v.choke = true;
        Voice* slot = nullptr;
        for (auto& v : voices) if (! v.active) { slot = &v; break; }
        if (slot == nullptr) { slot = &voices[0]; for (auto& v : voices) if (v.pos > slot->pos) slot = &v; }   // steal oldest
        *slot = Voice { smp.data(), (int) smp.size(), 0, vel * mix.level[(size_t) group], group, false, 1.f, true };
        flash[(size_t) pad].store (1.f);
    };

    float* ch0 = buffer.getWritePointer (0);
    float* ch1 = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
    int ti = 0;
    for (int i = 0; i < n; ++i)
    {
        while (ti < nTrig && triggers[(size_t) ti].sample <= i) { fire (triggers[(size_t) ti].pad, triggers[(size_t) ti].vel); ++ti; }
        float sum = 0.f;
        for (auto& v : voices)
        {
            if (! v.active) continue;
            float s = v.data[v.pos++] * v.gain;
            if (v.choke)
            {
                v.env -= chokeStep;
                if (v.env <= 0.f) { v.active = false; continue; }
                s *= v.env;
            }
            sum += s;
            if (v.pos >= v.len) v.active = false;
        }
        const float g = lastMaster + (master - lastMaster) * (float) (i + 1) / (float) n;
        const float y = bus.process (sum) * g;
        ch0[i] = y;
        if (ch1 != nullptr) ch1[i] = y;
    }
    lastMaster = master;
}

//==============================================================================
bool PhonkProcessor::exportKitTo (const juce::File& folder, std::function<void (int)> onDone)
{
    if (exportThread.isThreadRunning()) return false;
    ExportOptions o;
    o.kit = kitSettings(); o.bus = busSettings(); o.mix = mixSettings(); o.pattern = patternSettings();
    o.swing = raw (ids::swing); o.currentBpm = raw (ids::bpm);
    const std::string path = folder.getFullPathName().toStdString();
    exportThread.job = [o, path, onDone]
    {
        const int count = phonk::exportKit (path, o);
        juce::MessageManager::callAsync ([onDone, count] { if (onDone) onDone (count); });
    };
    exportThread.startThread();
    return true;
}

juce::File PhonkProcessor::renderLoopFile()
{
    const double sr = 44100.0, tempo = raw (ids::bpm);
    const auto kit = renderKit (kitSettings(), sr);
    const auto loop = renderLoop (kit, generatePattern (patternSettings()), tempo, raw (ids::swing), mixSettings(), busSettings());
    auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("PhonkFactory");
    dir.createDirectory();
    auto file = dir.getChildFile ("Phonk_Loop_" + juce::String (juce::roundToInt (tempo)) + "BPM_Seed" + juce::String ((int) raw (ids::patSeed)) + ".wav");
    return writeWav (file.getFullPathName().toStdString(), loop, sr, 16) ? file : juce::File();
}

//==============================================================================
void PhonkProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void PhonkProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
            kitDirty = patternDirty = true;
        }
}

juce::AudioProcessorEditor* PhonkProcessor::createEditor() { return new PhonkEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PhonkProcessor(); }
