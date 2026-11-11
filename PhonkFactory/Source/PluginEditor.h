#pragma once
#include "PluginProcessor.h"

namespace ui
{
    const juce::Colour bg      { 0xff120e16 };
    const juce::Colour panel   { 0xff1d1724 };
    const juce::Colour edge    { 0xff33283d };
    const juce::Colour text    { 0xffd9d0c1 };
    const juce::Colour dim     { 0xff7d7388 };
    const juce::Colour accent  { 0xffe63946 };   // blood red
    const juce::Colour amber   { 0xfff2a900 };
    inline juce::Colour voiceColour (int v)
    {
        static const juce::Colour c[] = { juce::Colour (0xffe63946), juce::Colour (0xfff2a900), juce::Colour (0xffb8c4c9),
                                          juce::Colour (0xff9b6bff), juce::Colour (0xff3ddc97) };
        return c[v % 5];
    }
}

class PhonkLook : public juce::LookAndFeel_V4
{
public:
    PhonkLook();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool hover, bool down) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool hover, bool down) override;
};

class PhonkEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PhonkEditor (PhonkProcessor&);
    ~PhonkEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class Knob;  class PadGrid;  class PatternView;  class DragHandle;
    struct Panel { juce::String title; juce::Rectangle<int> bounds; };

    void timerCallback() override;
    void chooseExportFolder();
    void setStatus (const juce::String& s) { status = s; statusSetAt = juce::Time::getMillisecondCounter(); repaint(); }

    PhonkProcessor& proc;
    PhonkLook look;
    std::vector<std::unique_ptr<Knob>> knobs;
    std::unique_ptr<PadGrid> pads;
    std::unique_ptr<PatternView> patternView;
    std::unique_ptr<DragHandle> dragHandle;
    juce::TextButton exportButton { "EXPORT KIT" }, newPatternButton { "NEW PATTERN" }, rerollButton { "REROLL KIT" };
    juce::ToggleButton playButton { "PLAY" }, hostButton { "HOST SYNC" };
    juce::ComboBox barsBox;
    std::unique_ptr<juce::ComboBoxParameterAttachment> barsAttach;
    std::unique_ptr<juce::ButtonParameterAttachment> playAttach, hostAttach;
    std::unique_ptr<juce::FileChooser> chooser;
    std::vector<Panel> panels;
    juce::String status;
    juce::uint32 statusSetAt = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhonkEditor)
};
