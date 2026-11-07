#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace kf;

namespace
{
    // Transparent below 0.8 (~ -2 dBFS), then a smooth knee that never exceeds 1.0.
    inline float safetyClip (float x) noexcept
    {
        constexpr float knee = 0.8f, range = 1.0f - knee;
        const float a = std::abs (x);
        if (a <= knee)
            return x;
        return std::copysign (knee + range * std::tanh ((a - knee) / range), x);
    }
}

KitForgeProcessor::KitForgeProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "KitForge", createParameterLayout())
{
    for (int t = 0; t < numTracks; ++t)
        for (int vp = 0; vp < NumVP; ++vp)
            raw[(size_t) t][(size_t) vp] = apvts.getRawParameterValue (voiceParamID (t, vp));

    tempoParam   = apvts.getRawParameterValue (ids::tempo);
    swingParam   = apvts.getRawParameterValue (ids::swing);
    lengthParam  = apvts.getRawParameterValue (ids::length);
    patternParam = apvts.getRawParameterValue (ids::pattern);
    masterParam  = apvts.getRawParameterValue (ids::master);

    for (auto& c : triggerCount) c.store (0);
    for (auto& p : previewRequests) p.store (0.0f);

    // Out of the box: the first factory kit and a couple of grooves.
    loadKit (KitFactory::factoryKit (0));
    KitFactory::writeDefaultPatterns (patterns);

    events.reserve (1024);
}

//==============================================================================
bool KitForgeProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void KitForgeProcessor::prepareToPlay (double sr, int)
{
    sampleRate = sr;
    for (auto& v : voices)
        v.prepare (sr);

    masterGain.reset (sr, 0.02);
    masterGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (masterParam->load()));
    lastStepIndex = std::numeric_limits<juce::int64>::min();
}

int KitForgeProcessor::getCurrentPattern() const noexcept
{
    return juce::jlimit (0, numPatterns - 1, (int) patternParam->load());
}

//==============================================================================
void KitForgeProcessor::setInternalPlaying (bool shouldPlay)
{
    if (shouldPlay && ! internalPlaying.load())
        resetInternalClock.store (true);
    internalPlaying.store (shouldPlay);
}

void KitForgeProcessor::previewTrack (int track, float velocity)
{
    if (juce::isPositiveAndBelow (track, numTracks))
        previewRequests[(size_t) track].store (velocity);
}

void KitForgeProcessor::loadKit (const Kit& kit)
{
    KitFactory::applyKit (apvts, kit);
    kitName = kit.name;
}

//==============================================================================
void KitForgeProcessor::readVoiceParams()
{
    for (int t = 0; t < numTracks; ++t)
    {
        const auto& r = raw[(size_t) t];
        auto& p = voiceParams[(size_t) t];

        p.wave         = (int) r[Wave]->load();
        p.pitch        = r[Pitch]->load();
        p.pEnvAmt      = r[PEnvAmt]->load();
        p.pEnvDecayMs  = r[PEnvDecay]->load();
        p.toneDecayMs  = r[ToneDecay]->load();
        p.toneLevel    = r[ToneLevel]->load();
        p.fmAmt        = r[FmAmt]->load();
        p.fmRatio      = r[FmRatio]->load();
        p.noiseType    = (int) r[NoiseType]->load();
        p.noiseLevel   = r[NoiseLevel]->load();
        p.noiseDecayMs = r[NoiseDecay]->load();
        p.filterMode   = (int) r[FilterMode]->load();
        p.cutoff       = r[Cutoff]->load();
        p.reso         = r[Reso]->load();
        p.bursts       = (int) std::round (r[Burst]->load());
        p.click        = r[Click]->load();
        p.attackMs     = r[Attack]->load();
        p.drive        = r[Drive]->load();
        p.level        = r[Level]->load();
        p.pan          = r[Pan]->load();
        p.choke        = (int) std::round (r[Choke]->load());

        voices[(size_t) t].setParams (p);

        // Equal-power pan, unity at centre.
        const float angle = (p.pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
        panL[(size_t) t] = std::cos (angle) * juce::MathConstants<float>::sqrt2;
        panR[(size_t) t] = std::sin (angle) * juce::MathConstants<float>::sqrt2;
    }
}

void KitForgeProcessor::fire (int track, float velocity)
{
    const int group = voiceParams[(size_t) track].choke;
    if (group > 0)
        for (int t = 0; t < numTracks; ++t)
            if (t != track && voiceParams[(size_t) t].choke == group)
                voices[(size_t) t].choke();

    voices[(size_t) track].trigger (velocity);
    triggerCount[(size_t) track].fetch_add (1, std::memory_order_relaxed);
}

//==============================================================================
void KitForgeProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    buffer.clear();
    events.clear();

    readVoiceParams();

    // ---- Mixer state ----
    bool anySolo = false;
    for (int t = 0; t < numTracks; ++t)
        anySolo |= raw[(size_t) t][Solo]->load() > 0.5f;

    auto audible = [&] (int t)
    {
        if (raw[(size_t) t][Mute]->load() > 0.5f) return false;
        return ! anySolo || raw[(size_t) t][Solo]->load() > 0.5f;
    };

    // ---- Transport: follow the host if it's playing, otherwise the internal clock ----
    double bpm = tempoParam->load();
    double ppqStart = 0.0;
    bool hostIsPlaying = false;

    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (pos->getIsPlaying())
            {
                hostIsPlaying = true;
                if (auto b = pos->getBpm()) bpm = *b;
                if (auto ppq = pos->getPpqPosition())
                    ppqStart = *ppq;
                else if (auto secs = pos->getTimeInSeconds())
                    ppqStart = *secs * bpm / 60.0;
            }
        }
    }
    hostPlaying.store (hostIsPlaying);

    if (resetInternalClock.exchange (false))
    {
        internalPpq = 0.0;
        lastStepIndex = std::numeric_limits<juce::int64>::min();
    }

    const bool running = hostIsPlaying || internalPlaying.load();
    if (! hostIsPlaying)
        ppqStart = internalPpq;

    if (running && bpm > 0.0)
    {
        const double ppqPerSample = bpm / 60.0 / sampleRate;
        const double ppqEnd = ppqStart + numSamples * ppqPerSample;
        constexpr double stepLen = 0.25; // sixteenth notes
        const double swingOffset = ((double) swingParam->load() / 100.0 - 0.5) * 2.0 * stepLen;

        const int length = juce::jlimit (1, numSteps, (int) lengthParam->load());
        const int pattern = getCurrentPattern();

        const auto first = (juce::int64) std::floor (ppqStart / stepLen) - 1;
        const auto last  = (juce::int64) std::floor (ppqEnd / stepLen);

        for (auto k = first; k <= last; ++k)
        {
            const double t = (double) k * stepLen + ((k & 1) ? swingOffset : 0.0);
            if (t < ppqStart || t >= ppqEnd || k == lastStepIndex)
                continue;

            lastStepIndex = k;
            const int offset = juce::jlimit (0, numSamples - 1, (int) ((t - ppqStart) / ppqPerSample));
            const int step = (int) (((k % length) + length) % length);
            currentStep.store (step);

            for (int tr = 0; tr < numTracks; ++tr)
                if (const auto vel = patterns.get (pattern, tr, step); vel > 0 && audible (tr))
                    events.push_back ({ offset, tr, (float) vel / 127.0f });
        }

        if (! hostIsPlaying)
            internalPpq = ppqEnd;
    }
    else
    {
        currentStep.store (-1);
        lastStepIndex = std::numeric_limits<juce::int64>::min();
    }

    // ---- Pad previews from the GUI ----
    for (int t = 0; t < numTracks; ++t)
        if (const float v = previewRequests[(size_t) t].exchange (0.0f); v > 0.0f)
            events.push_back ({ 0, t, v });

    // ---- MIDI input: notes from C1 (36) upwards play tracks 1..8 ----
    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        if (msg.isNoteOn())
        {
            const int track = msg.getNoteNumber() - midiBaseNote;
            if (juce::isPositiveAndBelow (track, numTracks))
                events.push_back ({ juce::jlimit (0, numSamples - 1, meta.samplePosition), track, msg.getFloatVelocity() });
        }
    }
    midi.clear();

    std::stable_sort (events.begin(), events.end(), [] (const Event& a, const Event& b) { return a.offset < b.offset; });

    // ---- Render ----
    masterGain.setTargetValue (juce::Decibels::decibelsToGain (masterParam->load(), -60.0f));

    auto* left  = buffer.getWritePointer (0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
    size_t ev = 0;

    for (int s = 0; s < numSamples; ++s)
    {
        while (ev < events.size() && events[ev].offset <= s)
        {
            fire (events[ev].track, events[ev].velocity);
            ++ev;
        }

        float l = 0.0f, r = 0.0f;
        for (int t = 0; t < numTracks; ++t)
        {
            auto& voice = voices[(size_t) t];
            if (! voice.isActive())
                continue;
            const float x = voice.process();
            l += x * panL[(size_t) t];
            r += x * panR[(size_t) t];
        }

        const float g = masterGain.getNextValue();
        if (right != nullptr)
        {
            left[s] = safetyClip (l * g);
            right[s] = safetyClip (r * g);
        }
        else
        {
            left[s] = safetyClip (0.5f * (l + r) * g);
        }
    }
}

//==============================================================================
void KitForgeProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    xml->setAttribute ("kitName", kitName);

    auto* pats = xml->createNewChildElement ("Patterns");
    for (int p = 0; p < numPatterns; ++p)
    {
        auto* pat = pats->createNewChildElement ("Pattern");
        pat->setAttribute ("index", p);
        for (int t = 0; t < numTracks; ++t)
            pat->setAttribute ("t" + juce::String (t + 1), patterns.trackToString (p, t));
    }

    copyXmlToBinary (*xml, destData);
}

void KitForgeProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return;

    kitName = xml->getStringAttribute ("kitName", kitName);

    if (auto* pats = xml->getChildByName ("Patterns"))
    {
        patterns.clearAll();
        for (auto* pat : pats->getChildWithTagNameIterator ("Pattern"))
        {
            const int p = pat->getIntAttribute ("index", -1);
            if (! juce::isPositiveAndBelow (p, numPatterns))
                continue;
            for (int t = 0; t < numTracks; ++t)
                patterns.trackFromString (p, t, pat->getStringAttribute ("t" + juce::String (t + 1)));
        }
        xml->removeChildElement (pats, true);
    }

    apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessorEditor* KitForgeProcessor::createEditor()
{
    return new KitForgeEditor (*this);
}

#if ! KITFORGE_NO_PLUGIN_ENTRY
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KitForgeProcessor();
}
#endif
