#pragma once

#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

class DrumKitAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit DrumKitAudioProcessorEditor(DrumKitAudioProcessor&);
    ~DrumKitAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);

    DrumKitAudioProcessor& processorRef;
    juce::WebSliderRelay outputGainRelay{ "outputGain" };
    juce::WebBrowserComponent webView;
    juce::WebSliderParameterAttachment outputGainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumKitAudioProcessorEditor)
};