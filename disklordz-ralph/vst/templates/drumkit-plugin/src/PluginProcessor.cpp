#include "PluginProcessor.h"
#include "PluginEditor.h"

DrumKitAudioProcessor::DrumKitAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    outputGainParam = apvts.getRawParameterValue("outputGain");
}

juce::AudioProcessorValueTreeState::ParameterLayout DrumKitAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "outputGain", 1 },
        "Output Gain",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
        1.0f));

    return { params.begin(), params.end() };
}

void DrumKitAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    smoothedGain.reset(sampleRate, 0.02);
    smoothedGain.setCurrentAndTargetValue(outputGainParam != nullptr ? outputGainParam->load() : 1.0f);
}

void DrumKitAudioProcessor::releaseResources() {}

bool DrumKitAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void DrumKitAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const float targetGain =
        outputGainParam != nullptr ? outputGainParam->load(std::memory_order_relaxed) : 1.0f;
    smoothedGain.setTargetValue(targetGain);

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            (void) metadata.samplePosition;
        }
    }

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const float gain = smoothedGain.getNextValue();
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, buffer.getSample(channel, sample) * gain);
    }
}

juce::AudioProcessorEditor* DrumKitAudioProcessor::createEditor()
{
    return new DrumKitAudioProcessorEditor(*this);
}

void DrumKitAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    xml->setAttribute("pluginVersion", 1);
    copyXmlToBinary(*xml, destData);
}

void DrumKitAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrumKitAudioProcessor();
}