#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <vector>

DrumSynthAudioProcessor::DrumSynthAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    refreshLicenseFromDisk();
}

juce::AudioProcessorValueTreeState::ParameterLayout DrumSynthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "outputGain", 1 },
        "Output Gain",
        juce::NormalisableRange<float>(0.0f, 1.5f, 0.01f),
        1.0f));
    return { params.begin(), params.end() };
}

void DrumSynthAudioProcessor::refreshLicenseFromDisk()
{
    const auto licFile = LicenseVerifier::defaultLicenseFile();
    if (!licFile.existsAsFile())
    {
        isLicensed.store(false, std::memory_order_relaxed);
        return;
    }
    const auto res = LicenseVerifier::verifyLicenseJson(licFile.loadFileAsString());
    isLicensed.store(res.isValid, std::memory_order_relaxed);
}

void DrumSynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    demoSamplesCounter = 0;
    refreshLicenseFromDisk();
    voiceDsp.prepare(sampleRate, samplesPerBlock);
}

void DrumSynthAudioProcessor::releaseResources()
{
    voiceDsp.reset();
}

bool DrumSynthAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void DrumSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    if (uiTriggerPending.exchange(false, std::memory_order_relaxed))
        voiceDsp.triggerNote(36, 1.0f);

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
            voiceDsp.triggerNote(msg.getNoteNumber(), msg.getFloatVelocity());
    }

    voiceDsp.processBlock(buffer);

    if (const float gain = apvts.getRawParameterValue("outputGain")->load(); gain != 1.0f)
        buffer.applyGain(gain);

    if (!isLicensed.load(std::memory_order_relaxed))
    {
        const int numSamples = buffer.getNumSamples();
        const int periodSamples = juce::jmax(1, static_cast<int>(currentSampleRate * 20.0));
        const int dipLength = juce::jmax(1, static_cast<int>(currentSampleRate * 0.4));

        if (demoSamplesCounter < dipLength)
            buffer.applyGain(0.15f);
        demoSamplesCounter = (demoSamplesCounter + numSamples) % periodSamples;
    }
}

void DrumSynthAudioProcessor::triggerTestHit()
{
    uiTriggerPending.store(true, std::memory_order_relaxed);
}

bool DrumSynthAudioProcessor::activateLicenseJson(const juce::String& jsonText)
{
    const auto res = LicenseVerifier::verifyLicenseJson(jsonText);
    if (!res.isValid)
        return false;

    const auto licFile = LicenseVerifier::defaultLicenseFile();
    licFile.getParentDirectory().createDirectory();
    licFile.replaceWithText(jsonText);
    isLicensed.store(true, std::memory_order_relaxed);
    return true;
}

void DrumSynthAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    const auto state = apvts.copyState();
    if (const std::unique_ptr<juce::XmlElement> xml(state.createXml()))
        copyXmlToBinary(*xml, destData);
}

void DrumSynthAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes <= 0 || sizeInBytes > 1024 * 1024)
        return;

    if (const std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
        xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessorEditor* DrumSynthAudioProcessor::createEditor()
{
    return new DrumSynthAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrumSynthAudioProcessor();
}