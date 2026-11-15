#pragma once

#include <JuceHeader.h>
#include "../../PadGridComponent.h"
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetroitPhonkTrapAudioProcessorEditor)
};