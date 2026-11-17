#pragma once
#include "Styles.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace forge
{
    /** Edit a style with checkboxes and sliders instead of JSON. Saved styles go to
        <app data>/MPCSampleForge/Styles and load at every start. */
    class StyleEditor final : public juce::Component
    {
    public:
        explicit StyleEditor (const Style& base);

        std::function<void (const Style&)> onSave;    // "Save as my style"
        std::function<void (const Style&)> onApply;   // "Use now" (this session only)

        static juce::File userStyleFolder();
        static juce::Array<Style> loadUserStyles (juce::StringArray& errors);

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        bool build (Style& out, juce::String& error) const;
        juce::Slider& makeRange (juce::Slider& s, double lo, double hi, double step, double a, double b, const juce::String& suffix);

        Style base;
        juce::Label title, nameLabel, engineLabel, scaleLabel, bankLabel, chordsLabel, voicingsLabel, status;
        juce::TextEditor nameEditor;
        juce::ComboBox engineBox, scaleBox, bankBox;
        juce::OwnedArray<juce::ToggleButton> chordToggles, voicingToggles;
        juce::Slider chordRange, keyRange, velocity, hold, chordRatio, strum, stabChance, maxLength;
        juce::OwnedArray<juce::Label> sliderLabels;
        juce::TextButton saveButton { "Save as my style" }, applyButton { "Use now" }, resetButton { "Reset" };
    };
}
