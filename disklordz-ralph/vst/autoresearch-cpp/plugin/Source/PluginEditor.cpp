#include "PluginEditor.h"
#include "LicenseVerifier.h"

DrumSynthAudioProcessorEditor::DrumSynthAudioProcessorEditor(DrumSynthAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    setSize(480, 320);

    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2563eb));
    triggerButton.onClick = [this] { audioProcessor.triggerTestHit(); };
    addAndMakeVisible(triggerButton);

    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    addAndMakeVisible(gainSlider);

    gainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(gainLabel);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "outputGain", gainSlider);

    licenseInput.setMultiLine(false);
    licenseInput.setTextToShowWhenEmpty("Paste signed license JSON here...", juce::Colours::grey);
    addAndMakeVisible(licenseInput);

    activateButton.onClick = [this] {
        if (audioProcessor.activateLicenseJson(licenseInput.getText()))
        {
            statusLabel.setText("STATUS: LICENSED (PRO)", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colours::limegreen);
            licenseInput.clear();
        }
        else
        {
            statusLabel.setText("STATUS: INVALID SIGNATURE OR MACHINE ID", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colours::orangered);
        }
    };
    addAndMakeVisible(activateButton);

    const bool licensed = audioProcessor.isPluginLicensed();
    if (licensed)
    {
        statusLabel.setText("STATUS: LICENSED (PRO)", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::limegreen);
    }
    else
    {
        statusLabel.setText("STATUS: DEMO MODE (Machine ID: "
                                + LicenseVerifier::getLocalMachineId().substring(0, 12) + "...)",
                            juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    }
    addAndMakeVisible(statusLabel);
}

void DrumSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0f172a));
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    g.drawText("DISKLORDZ // AUTORESEARCH KICK", 20, 16, getWidth() - 40, 30, juce::Justification::left);
}

void DrumSynthAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(42);

    auto topRow = area.removeFromTop(140);
    triggerButton.setBounds(topRow.removeFromLeft(220).reduced(8));

    const auto knobArea = topRow.reduced(8);
    gainLabel.setBounds(knobArea.removeFromTop(20));
    gainSlider.setBounds(knobArea);

    area.removeFromTop(12);
    statusLabel.setBounds(area.removeFromTop(24));

    auto licRow = area.removeFromTop(36);
    activateButton.setBounds(licRow.removeFromRight(150).reduced(2));
    licenseInput.setBounds(licRow.reduced(2));
}