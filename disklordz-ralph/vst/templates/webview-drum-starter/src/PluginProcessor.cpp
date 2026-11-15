#include "PluginProcessor.h"
#include "PluginEditor.h"

WebDrumAudioProcessor::WebDrumAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout WebDrumAudioProcessor::createParameterLayout()
{
    return {
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "outputGain", 1 },
            "Output Gain",
            juce::NormalisableRange<float>(0.0f, 1.5f, 0.01f),
            0.85f),
    };
}

void WebDrumAudioProcessor::prepareToPlay(double newSampleRate, int)
{
    sampleRate = newSampleRate;
    gainSmoother.reset(newSampleRate, 0.02);
    gainSmoother.setCurrentAndTargetValue(apvts.getRawParameterValue("outputGain")->load());
}

void WebDrumAudioProcessor::releaseResources() {}

bool WebDrumAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void WebDrumAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    gainSmoother.setTargetValue(apvts.getRawParameterValue("outputGain")->load());

    if (kickPending.exchange(false, std::memory_order_relaxed))
    {
        kickActive = true;
        env = 1.0f;
        phase = 0.0f;
    }

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn() && msg.getNoteNumber() == 36)
        {
            kickActive = true;
            env = 1.0f;
            phase = 0.0f;
        }
    }

    const int numSamples = buffer.getNumSamples();
    auto* left = buffer.getWritePointer(0);

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = 0.0f;
        if (kickActive)
        {
            const float freq = 48.0f + 90.0f * std::exp(-env * 4.0f);
            const float inc = juce::MathConstants<float>::twoPi * freq / static_cast<float>(sampleRate);
            phase += inc;
            if (phase > juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;
            sample = std::sin(phase) * env;
            env *= 0.9992f;
            if (env < 1.0e-4f)
                kickActive = false;
        }

        const float gain = gainSmoother.getNextValue();
        sample *= gain;
        left[i] = sample;
    }

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom(ch, 0, buffer, 0, 0, numSamples);
}

void WebDrumAudioProcessor::triggerKickFromUi()
{
    kickPending.store(true, std::memory_order_relaxed);
}

void WebDrumAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (const auto state = apvts.copyState(); state.isValid())
        if (const std::unique_ptr<juce::XmlElement> xml(state.createXml()))
            copyXmlToBinary(*xml, destData);
}

void WebDrumAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes <= 0 || sizeInBytes > 1024 * 1024)
        return;
    if (const std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
        xml != nullptr && xml->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessorEditor* WebDrumAudioProcessor::createEditor()
{
    return new WebDrumAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WebDrumAudioProcessor();
}