#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class DrumSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit DrumSynthAudioProcessorEditor(DrumSynthAudioProcessor&);
    ~DrumSynthAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DrumSynthAudioProcessor& audioProcessor;

    juce::TextButton triggerButton{ "TRIGGER KICK (C1)" };
    juce::Slider gainSlider;
    juce::Label gainLabel{ {}, "OUTPUT GAIN" };

    juce::TextEditor licenseInput;
    juce::TextButton activateButton{ "ACTIVATE LICENSE" };
    juce::TextButton copyMachineIdButton{ "COPY MACHINE ID" };
    juce::Label statusLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumSynthAudioProcessorEditor)
};