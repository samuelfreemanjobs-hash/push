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
    for (int bp = 0; bp < NumBP; ++bp)
        busRaw[(size_t) bp] = apvts.getRawParameterValue (busParamID (bp));

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
    crusher.prepare (sr);
    shaper.prepare (sr);
    vinyl.prepare (sr);

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
    voiceNames = kit.voiceNames;
}

//==============================================================================
void KitForgeProcessor::readVoiceParams()
{
    for (int t = 0; t < numTracks; ++t)
    {
        const auto& r = raw[(size_t) t];
        std::array<float, NumVP> values;
        for (size_t vp = 0; vp < (size_t) NumVP; ++vp)
            values[vp] = r[vp]->load();

        auto& p = voiceParams[(size_t) t];
        p = VoiceParams::fromValues (values);

        voices[(size_t) t].setParams (p);

        // Equal-power pan, unity at centre.
        const float angle = (p.pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
        panL[(size_t) t] = std::cos (angle) * juce::MathConstants<float>::sqrt2;
        panR[(size_t) t] = std::sin (angle) * juce::MathConstants<float>::sqrt2;
    }

    std::array<float, NumBP> busValues;
    for (size_t bp = 0; bp < (size_t) NumBP; ++bp)
        busValues[bp] = busRaw[bp]->load();
    const auto bus = BusParams::fromValues (busValues);
    crusher.setParams (bus);
    shaper.setAmounts (bus.punch, bus.tail);
    vinyl.setAmount (bus.vinyl);
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

        float cleanL = 0.0f, cleanR = 0.0f, sendL = 0.0f, sendR = 0.0f;
        bool anyActive = false;
        for (int t = 0; t < numTracks; ++t)
        {
            auto& voice = voices[(size_t) t];
            if (! voice.isActive())
                continue;
            anyActive = true;
            const float x = voice.process();
            const float send = voiceParams[(size_t) t].lofiSend;
            const float xl = x * panL[(size_t) t], xr = x * panR[(size_t) t];
            sendL  += xl * send;
            sendR  += xr * send;
            cleanL += xl * (1.0f - send);
            cleanR += xr * (1.0f - send);
        }

        // Vinyl surface plays while the machine is running or ringing out.
        if (running || anyActive)
            vinyl.process (sendL, sendR);

        crusher.process (sendL, sendR);
        float l = cleanL + sendL, r = cleanR + sendR;
        shaper.process (l, r);

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

    auto* names = xml->createNewChildElement ("VoiceNames");
    for (int t = 0; t < numTracks; ++t)
        names->setAttribute ("t" + juce::String (t + 1), voiceNames[(size_t) t]);

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

    if (auto* names = xml->getChildByName ("VoiceNames"))
    {
        for (int t = 0; t < numTracks; ++t)
            voiceNames[(size_t) t] = names->getStringAttribute ("t" + juce::String (t + 1), voiceNames[(size_t) t]);
        xml->removeChildElement (names, true);
    }

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
