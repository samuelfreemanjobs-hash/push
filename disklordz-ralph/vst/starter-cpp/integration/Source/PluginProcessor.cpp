#include "PluginProcessor.h"
#include "PluginEditor.h"

DetroitPhonkTrapAudioProcessor::DetroitPhonkTrapAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", {})
{
}

void DetroitPhonkTrapAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
}

void DetroitPhonkTrapAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    if (auto* playHead = getPlayHead())
    {
        if (auto pos = playHead->getPosition())
        {
            if (pos->getBpm().hasValue())
                padStore.setLastKnownBpm(*pos->getBpm());
        }
    }

    juce::ignoreUnused(midi);
    // shortcut: silent stub until Synthesiser + TrapCoreDSP voices wired, upgrade before public Lite build
}

juce::AudioProcessorEditor* DetroitPhonkTrapAudioProcessor::createEditor()
{
    return new DetroitPhonkTrapAudioProcessorEditor(*this);
}

void DetroitPhonkTrapAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        std::unique_ptr<juce::XmlElement> xml(state.createXml());
        copyXmlToBinary(*xml, destData);
    }
}

void DetroitPhonkTrapAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes); xml != nullptr)
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DetroitPhonkTrapAudioProcessor();
}