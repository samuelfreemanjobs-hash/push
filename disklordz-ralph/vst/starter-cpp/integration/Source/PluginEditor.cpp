#include "PluginEditor.h"

DetroitPhonkTrapAudioProcessorEditor::DetroitPhonkTrapAudioProcessorEditor(
    DetroitPhonkTrapAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    padGrid = std::make_unique<PadGridComponent>(*this);
    addAndMakeVisible(*padGrid);

    padGrid->setPadCount(5);
    padGrid->setGetPadAudio([this](int i) { return processorRef.getPadBuffer(i); });
    padGrid->setGetPadRate([this](int i) { return processorRef.getPadSampleRate(i); });
    padGrid->setGetPattern([this](int i) { return processorRef.getPadMidiPattern(i); });
    padGrid->setGetHostTempo([this]() { return processorRef.getLastKnownBpm(); });
    padGrid->setLoadPadFromFile([this](int i, const juce::File& f) {
        processorRef.loadUserWavOntoPad(i, f);
    });

    setSize(640, 128);
}

void DetroitPhonkTrapAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff121216));
}

void DetroitPhonkTrapAudioProcessorEditor::resized()
{
    padGrid->setBounds(getLocalBounds());
}