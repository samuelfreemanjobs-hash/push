#include "PluginProcessor.h"
#include "PluginEditor.h"

MemphisDustProcessor::MemphisDustProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "MemphisDust", md::createParameterLayout())
{
    auto get = [this] (const char* id) { auto* v = apvts.getRawParameterValue (id); jassert (v != nullptr); return v; };
    using namespace md;
    p.inputGain = get (pid::inputGain);   p.outputGain = get (pid::outputGain); p.mix = get (pid::mix);
    p.outClip = get (pid::outClip);
    p.crushOn = get (pid::crushOn);       p.crushRate = get (pid::crushRate);   p.crushBits = get (pid::crushBits);
    p.crushAA = get (pid::crushAA);
    p.eqOn = get (pid::eqOn);             p.lowCut = get (pid::lowCut);         p.boxFreq = get (pid::boxFreq);
    p.boxGain = get (pid::boxGain);       p.boxQ = get (pid::boxQ);             p.highCut = get (pid::highCut);
    p.tapeOn = get (pid::tapeOn);         p.tapeDrive = get (pid::tapeDrive);   p.tapeBias = get (pid::tapeBias);
    p.wow = get (pid::wow);               p.flutter = get (pid::flutter);       p.hiss = get (pid::hiss);
    p.compOn = get (pid::compOn);         p.compThresh = get (pid::compThresh); p.compRatio = get (pid::compRatio);
    p.compAttack = get (pid::compAttack); p.compRelease = get (pid::compRelease);
    p.compMakeup = get (pid::compMakeup); p.compMix = get (pid::compMix);
    p.subOn = get (pid::subOn);           p.subFreq = get (pid::subFreq);       p.subDrive = get (pid::subDrive);
    p.subHard = get (pid::subHard);       p.subLevel = get (pid::subLevel);
}

bool MemphisDustProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return out == layouts.getMainInputChannelSet();
}

void MemphisDustProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    maxBlock = juce::jmax (1, samplesPerBlock);
    numCh = juce::jlimit (1, 2, getTotalNumOutputChannels());

    oversampler = std::make_unique<juce::dsp::Oversampling<float>> (
        (size_t) numCh, 1, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true);
    oversampler->initProcessing ((size_t) maxBlock);

    crusher.prepare (sampleRate);
    eq.prepare (sampleRate);
    tape.prepare (sampleRate * 2.0);
    wowFlutter.prepare (sampleRate);
    hiss.prepare (sampleRate);
    subClip.prepare (sampleRate);
    compressor.prepare (sampleRate);

    const int latency = (int) oversampler->getLatencyInSamples() + wowFlutter.getLatencySamples();
    setLatencySamples (latency);

    dryWet.prepare ({ sampleRate, (juce::uint32) maxBlock, (juce::uint32) numCh });
    dryWet.setMixingRule (juce::dsp::DryWetMixingRule::linear);
    dryWet.setWetLatency ((float) latency);

    inGain.reset (sampleRate, 0.03);
    outGain.reset (sampleRate, 0.03);
    inGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (p.inputGain->load()));
    outGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (p.outputGain->load()));

    updateParameters();
}

void MemphisDustProcessor::updateParameters()
{
    auto on = [] (std::atomic<float>* v) { return v->load() > 0.5f; };

    crushOn = on (p.crushOn); eqOn = on (p.eqOn); tapeOn = on (p.tapeOn);
    subOn = on (p.subOn);     compOn = on (p.compOn); clipOn = on (p.outClip);

    inGain.setTargetValue (juce::Decibels::decibelsToGain (p.inputGain->load()));
    outGain.setTargetValue (juce::Decibels::decibelsToGain (p.outputGain->load()));
    dryWet.setWetMixProportion (p.mix->load());

    crusher.setParams (p.crushRate->load(), p.crushBits->load(), on (p.crushAA));
    eq.setParams (p.lowCut->load(), p.boxFreq->load(), p.boxGain->load(), p.boxQ->load(), p.highCut->load());
    tape.setParams (p.tapeDrive->load(), p.tapeBias->load());
    // Delay line always runs (constant latency); modulation is zeroed when the tape is off.
    wowFlutter.setParams (tapeOn ? p.wow->load() : 0.0f, tapeOn ? p.flutter->load() : 0.0f);
    hiss.setLevelDb (p.hiss->load());
    subClip.setParams (p.subFreq->load(), p.subDrive->load(), p.subHard->load(), p.subLevel->load());
    compressor.setParams (p.compThresh->load(), (int) p.compRatio->load(), p.compAttack->load(),
                          p.compRelease->load(), p.compMakeup->load(), p.compMix->load());
}

void MemphisDustProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear (c, 0, numSamples);

    if (oversampler == nullptr || numSamples == 0)
        return;

    updateParameters();

    const int chans = juce::jmin (numCh, buffer.getNumChannels());
    juce::dsp::AudioBlock<float> full (buffer.getArrayOfWritePointers(), (size_t) chans, (size_t) numSamples);

    // Never hand the oversampler / mixer more than they were prepared for.
    for (int pos = 0; pos < numSamples; pos += maxBlock)
    {
        const int len = juce::jmin (maxBlock, numSamples - pos);
        processChunk (full.getSubBlock ((size_t) pos, (size_t) len));
    }

    float peak = 0.0f;
    for (int c = 0; c < chans; ++c)
        peak = juce::jmax (peak, buffer.getMagnitude (c, 0, numSamples));
    if (peak > outPeak.load()) outPeak.store (peak);
}

void MemphisDustProcessor::processChunk (juce::dsp::AudioBlock<float> block)
{
    const int n = (int) block.getNumSamples();
    const int chans = (int) block.getNumChannels();
    float* ch[2] = { block.getChannelPointer (0), chans > 1 ? block.getChannelPointer (1) : nullptr };

    dryWet.pushDrySamples (block);

    // Input gain
    for (int i = 0; i < n; ++i)
    {
        const float g = inGain.getNextValue();
        for (int c = 0; c < chans; ++c) ch[c][i] *= g;
    }

    // 01 — 12-bit sampler
    if (crushOn) crusher.process (ch, chans, n);

    // 02 — cassette EQ curve
    if (eqOn) eq.process (ch, chans, n);

    // 03 — tape: saturation (2x oversampled), wow/flutter, hiss
    {
        auto up = oversampler->processSamplesUp (block);
        if (tapeOn)
        {
            float* upCh[2] = { up.getChannelPointer (0), chans > 1 ? up.getChannelPointer (1) : nullptr };
            tape.process (upCh, chans, (int) up.getNumSamples());
        }
        oversampler->processSamplesDown (block);
    }
    wowFlutter.process (ch, chans, n);
    if (tapeOn) hiss.process (ch, chans, n);

    // 05 — mono 808 clip (before the comp so the bus pumps against the knock)
    if (subOn) subClip.process (ch, chans, n);

    // 04 — FET bus compressor
    if (compOn) compressor.process (ch, chans, n);

    dryWet.mixWetSamples (block);

    // Output gain + optional "mixer pushed into the red" soft clip
    for (int i = 0; i < n; ++i)
    {
        const float g = outGain.getNextValue();
        for (int c = 0; c < chans; ++c)
        {
            float y = ch[c][i] * g;
            ch[c][i] = clipOn ? md::softClip (y, 0.7f) : y;
        }
    }
}

//==============================================================================
int MemphisDustProcessor::getNumPrograms() { return (int) md::getFactoryPresets().size(); }

const juce::String MemphisDustProcessor::getProgramName (int index)
{
    const auto& presets = md::getFactoryPresets();
    return juce::isPositiveAndBelow (index, (int) presets.size()) ? juce::String (presets[(size_t) index].name) : juce::String();
}

void MemphisDustProcessor::setCurrentProgram (int index)
{
    const auto& presets = md::getFactoryPresets();
    if (! juce::isPositiveAndBelow (index, (int) presets.size()))
        return;

    currentProgram = index;
    const auto& preset = presets[(size_t) index];

    for (auto* param : getParameters())
    {
        auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param);
        if (ranged == nullptr) continue;

        float normalised = ranged->getDefaultValue();
        for (const auto& [id, value] : preset.values)
            if (ranged->getParameterID() == id)
                normalised = ranged->convertTo0to1 (value);

        ranged->beginChangeGesture();
        ranged->setValueNotifyingHost (normalised);
        ranged->endChangeGesture();
    }
}

void MemphisDustProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void MemphisDustProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto state = juce::ValueTree::fromXml (*xml);
            currentProgram = state.getProperty ("program", 0);
            apvts.replaceState (state);
        }
}

juce::AudioProcessorEditor* MemphisDustProcessor::createEditor() { return new MemphisDustEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MemphisDustProcessor(); }
