#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../Factory808.h"
#include "../../Modules/TrapCoreDSP/SoftClipper.h"
#include "../../Modules/TrapCoreDSP/Trap808Voice.h"
#include "../../Modules/TrapCoreDSP/TrapSamplerSound.h"

DetroitPhonkTrapAudioProcessor::DetroitPhonkTrapAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout DetroitPhonkTrapAudioProcessor::createParameterLayout()
{
    return {
        std::make_unique<juce::AudioParameterFloat>("glide", "Glide", juce::NormalisableRange<float>(0.0f, 0.4f, 0.001f),
                                                    0.08f),
        std::make_unique<juce::AudioParameterFloat>(
            "clip", "808 Clip", juce::NormalisableRange<float>(1.0f, 4.0f, 0.01f), 1.8f),
    };
}

void DetroitPhonkTrapAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    synthesiser.setCurrentPlaybackSampleRate(sampleRate);
    synthesiser.clearVoices();
    synthesiser.clearSounds();
    sounds.clear();

    synthesiser.addVoice(std::make_unique<Trap808Voice>());
    for (auto* voice : synthesiser.getVoices())
        if (auto* trapVoice = dynamic_cast<Trap808Voice*>(voice))
            trapVoice->prepare(sampleRate);

    const auto factory = Factory808::render(sampleRate);
    sounds.push_back(std::make_unique<TrapSamplerSound>("BR-808", factory, sampleRate, 36, 36, 48));
    for (const auto& sound : sounds)
        synthesiser.addSound(sound.get());

    padStore.assignPadBuffer(0, factory, sampleRate);
}

void DetroitPhonkTrapAudioProcessor::releaseResources()
{
    synthesiser.clearVoices();
    synthesiser.clearSounds();
    sounds.clear();
}

void DetroitPhonkTrapAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    if (auto* playHead = getPlayHead())
    {
        if (const auto pos = playHead->getPosition())
            if (pos->getBpm().hasValue())
                padStore.setLastKnownBpm(*pos->getBpm());
    }

    const float glide = apvts.getRawParameterValue("glide")->load();
    const float clipDrive = apvts.getRawParameterValue("clip")->load();
    for (auto* voice : synthesiser.getVoices())
        if (auto* trapVoice = dynamic_cast<Trap808Voice*>(voice))
            trapVoice->setGlideSeconds(glide);

    buffer.clear();
    synthesiser.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* samples = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            samples[i] = SoftClipper::processSample(samples[i], clipDrive);
    }
}

juce::AudioBuffer<float>* DetroitPhonkTrapAudioProcessor::getPadBuffer(int i)
{
    ensurePadPreview(i);
    return padStore.getPadBufferForDrag(i);
}

void DetroitPhonkTrapAudioProcessor::loadUserWavOntoPad(int i, const juce::File& f)
{
    padStore.loadUserWavOntoPad(i, f);
    // shortcut: only pad store updated for drag; sampler map refresh on next prepareToPlay
}

void DetroitPhonkTrapAudioProcessor::ensurePadPreview(int pad)
{
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    if (!juce::isPositiveAndBelow(pad, PadSampleStore::kMaxPads))
        return;

    if (auto* existing = padStore.getPadBufferForDrag(pad);
        existing != nullptr && existing->getNumSamples() > 0)
        return;

    if (pad == 0)
        padStore.assignPadBuffer(0, Factory808::render(currentSampleRate), currentSampleRate);
}

juce::AudioProcessorEditor* DetroitPhonkTrapAudioProcessor::createEditor()
{
    return new DetroitPhonkTrapAudioProcessorEditor(*this);
}

void DetroitPhonkTrapAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (const auto state = apvts.copyState(); state.isValid())
    {
        const std::unique_ptr<juce::XmlElement> xml(state.createXml());
        copyXmlToBinary(*xml, destData);
    }
}

void DetroitPhonkTrapAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (const auto xml = getXmlFromBinary(data, sizeInBytes); xml != nullptr)
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DetroitPhonkTrapAudioProcessor();
}