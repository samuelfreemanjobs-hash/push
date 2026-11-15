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

    for (auto* slider : { &glideSlider, &clipSlider })
    {
        slider->setSliderStyle(juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 18);
        addAndMakeVisible(*slider);
    }
    glideLabel.setText("Glide", juce::dontSendNotification);
    clipLabel.setText("808 Clip", juce::dontSendNotification);
    for (auto* label : { &glideLabel, &clipLabel })
    {
        label->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*label);
    }

    auto& apvts = processorRef.getApvts();
    glideAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "glide",
                                                                                         glideSlider);
    clipAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "clip", clipSlider);

    setSize(640, 200);
}

void DetroitPhonkTrapAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff121216));
}

void DetroitPhonkTrapAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    const auto macroRow = area.removeFromBottom(72);
    const int half = macroRow.getWidth() / 2;

    auto glideArea = macroRow.removeFromLeft(half).reduced(8);
    glideLabel.setBounds(glideArea.removeFromTop(18));
    glideSlider.setBounds(glideArea);

    auto clipArea = macroRow.reduced(8);
    clipLabel.setBounds(clipArea.removeFromTop(18));
    clipSlider.setBounds(clipArea);

    padGrid->setBounds(area);
}