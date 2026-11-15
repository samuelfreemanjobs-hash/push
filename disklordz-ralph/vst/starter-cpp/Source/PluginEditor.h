#pragma once

#include <JuceHeader.h>
#include "../PadGridComponent.h"
#include "PluginProcessor.h"

class DetroitPhonkTrapAudioProcessorEditor : public juce::AudioProcessorEditor,
                                             public juce::DragAndDropContainer
{
public:
    explicit DetroitPhonkTrapAudioProcessorEditor(DetroitPhonkTrapAudioProcessor&);
    ~DetroitPhonkTrapAudioProcessorEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    DetroitPhonkTrapAudioProcessor& processorRef;
    std::unique_ptr<PadGridComponent> padGrid;
    juce::Slider glideSlider;
    juce::Slider clipSlider;
    juce::Label glideLabel;
    juce::Label clipLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> glideAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> clipAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetroitPhonkTrapAudioProcessorEditor)
};