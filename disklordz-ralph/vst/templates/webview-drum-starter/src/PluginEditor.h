#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class WebDrumAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit WebDrumAudioProcessorEditor(WebDrumAudioProcessor&);
    ~WebDrumAudioProcessorEditor() override;

    void resized() override;

private:
    WebDrumAudioProcessor& processorRef;
    std::unique_ptr<juce::WebBrowserComponent> webView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebDrumAudioProcessorEditor)
};