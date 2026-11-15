#pragma once

#include <atomic>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

/** UI hooks for background neural loop generation (TensorRT lab). */
class NeuralGeneratePanel final : public juce::Component
{
public:
    std::function<void(const juce::String& prompt, int bpm)> onGenerateRequested;

    NeuralGeneratePanel()
    {
        addAndMakeVisible(promptEditor);
        promptEditor.setMultiLine(false);
        promptEditor.setText("dark drift phonk cowbell loop", juce::dontSendNotification);
        promptEditor.setFont(juce::Font(14.0f));

        bpmSlider.setRange(80.0, 180.0, 1.0);
        bpmSlider.setValue(145.0);
        bpmSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
        addAndMakeVisible(bpmSlider);

        generateButton.onClick = [this] {
            if (onGenerateRequested)
                onGenerateRequested(promptEditor.getText(), static_cast<int>(bpmSlider.getValue()));
        };
        generateButton.setButtonText("Generate loop (GPU)");
        addAndMakeVisible(generateButton);

        statusLabel.setText("CPU fallback if no .trt engine", juce::dontSendNotification);
        statusLabel.setFont(juce::Font(12.0f));
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
        addAndMakeVisible(statusLabel);
    }

    void setStatus(const juce::String& s) { statusLabel.setText(s, juce::dontSendNotification); }

    void resized() override
    {
        auto r = getLocalBounds().reduced(8);
        promptEditor.setBounds(r.removeFromTop(28));
        r.removeFromTop(6);
        auto row = r.removeFromTop(40);
        bpmSlider.setBounds(row.removeFromLeft(120));
        generateButton.setBounds(row);
        r.removeFromTop(4);
        statusLabel.setBounds(r.removeFromTop(18));
    }

private:
    juce::TextEditor promptEditor;
    juce::Slider bpmSlider;
    juce::TextButton generateButton;
    juce::Label statusLabel;
};