#pragma once

#include "PluginProcessor.h"
#include "ui/DustLookAndFeel.h"

namespace md::ui
{
// Rotary knob + caption, bound to a parameter.
class Knob final : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& caption);
    void resized() override;

private:
    juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Label label;
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
};

// Choice param shown as a combo box with caption.
class Choice final : public juce::Component
{
public:
    Choice (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& caption);
    void resized() override;

private:
    juce::ComboBox box;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};

// Bool param as a lit push button.
class Toggle final : public juce::Component
{
public:
    Toggle (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& text);
    void resized() override;
    juce::ToggleButton button;

private:
    juce::AudioProcessorValueTreeState::ButtonAttachment attachment;
};

// A numbered processing section: header with power toggle + a 2-column grid of controls.
class Section final : public juce::Component, private juce::Button::Listener
{
public:
    Section (juce::AudioProcessorValueTreeState& state, juce::String number, juce::String title,
             juce::String subtitle, const juce::String& enableParamId);
    ~Section() override;

    template <typename T, typename... Args>
    T& add (Args&&... args)
    {
        auto c = std::make_unique<T> (std::forward<Args> (args)...);
        auto& ref = *c;
        addAndMakeVisible (ref);
        items.push_back (std::move (c));
        return ref;
    }

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void buttonClicked (juce::Button*) override { refreshEnabled(); }
    void refreshEnabled();

    juce::String number, title, subtitle;
    Toggle power;
    std::vector<std::unique_ptr<juce::Component>> items;
};

class GrMeter final : public juce::Component
{
public:
    void setGr (float db);
    void paint (juce::Graphics&) override;

private:
    float display = 0.0f, hold = 0.0f;
    int holdCounter = 0;
};

// Peak meter for the level hitting the tape, with the -12..-10 dBFS sweet spot marked.
class TapeLevelMeter final : public juce::Component
{
public:
    void setPeak (float linear);
    void setAutoLevelDb (float db) { autoDb = db; }
    void paint (juce::Graphics&) override;

private:
    float displayDb = -60.0f, holdDb = -60.0f, autoDb = 0.0f;
    int holdCounter = 0;
};
} // namespace md::ui

class MemphisDustEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit MemphisDustEditor (MemphisDustProcessor&);
    ~MemphisDustEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawReel (juce::Graphics&, juce::Point<float> centre, float radius, float angle) const;
    void refreshPresetBox();

    MemphisDustProcessor& processor;
    md::ui::DustLookAndFeel lnf;

    juce::ComboBox presetBox;
    md::ui::Section sampler, cassetteEq, tapeSec, dbxSec, sub, comp;
    md::ui::Knob inputKnob, mixKnob, outputKnob;
    md::ui::Toggle clipToggle, autoToggle;
    md::ui::GrMeter grMeter;
    md::ui::TapeLevelMeter tapeMeter;

    juce::Rectangle<int> reelArea;
    float reelAngle = 0.0f, activity = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemphisDustEditor)
};
