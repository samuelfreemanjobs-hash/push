#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace md::ui
{
namespace colours
{
    const juce::Colour bg       { 0xff15110e };
    const juce::Colour panel    { 0xff241d17 };
    const juce::Colour panelHi  { 0xff2f261e };
    const juce::Colour outline  { 0xff3d3127 };
    const juce::Colour text     { 0xffd9cbb0 };
    const juce::Colour textDim  { 0xff8c7d66 };
    const juce::Colour amber    { 0xffe39b3a };
    const juce::Colour red      { 0xffc0432f };
    const juce::Colour cream    { 0xffe9dcc0 };
    const juce::Colour knobDark { 0xff191410 };
}

class DustLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    DustLookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId, colours::text);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxHighlightColourId, colours::amber.withAlpha (0.4f));
        setColour (juce::Label::textColourId, colours::text);
        setColour (juce::ComboBox::backgroundColourId, colours::knobDark);
        setColour (juce::ComboBox::outlineColourId, colours::outline);
        setColour (juce::ComboBox::textColourId, colours::cream);
        setColour (juce::ComboBox::arrowColourId, colours::amber);
        setColour (juce::PopupMenu::backgroundColourId, colours::panel);
        setColour (juce::PopupMenu::textColourId, colours::text);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, colours::amber.withAlpha (0.25f));
        setColour (juce::PopupMenu::highlightedTextColourId, colours::cream);
        setColour (juce::TextEditor::backgroundColourId, colours::knobDark);
        setColour (juce::TextEditor::textColourId, colours::cream);
        setColour (juce::CaretComponent::caretColourId, colours::amber);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider& s) override
    {
        auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
        const float size = juce::jmin (bounds.getWidth(), bounds.getHeight());
        bounds = bounds.withSizeKeepingCentre (size, size);
        const auto centre = bounds.getCentre();
        const float radius = size * 0.5f;
        const float angle = startAngle + pos * (endAngle - startAngle);
        const float trackW = juce::jmax (2.5f, radius * 0.11f);

        // track
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, radius - trackW, radius - trackW, 0.0f, startAngle, endAngle, true);
        g.setColour (colours::knobDark);
        g.strokePath (track, juce::PathStrokeType (trackW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // value arc (bipolar params arc from centre)
        const bool bipolar = s.getMinimum() < 0.0 && s.getMaximum() > 0.0;
        const float from = bipolar ? startAngle + (float) s.valueToProportionOfLength (0.0) * (endAngle - startAngle) : startAngle;
        juce::Path arc;
        arc.addCentredArc (centre.x, centre.y, radius - trackW, radius - trackW, 0.0f,
                           juce::jmin (from, angle), juce::jmax (from, angle), true);
        g.setColour (s.isEnabled() ? colours::amber : colours::textDim);
        g.strokePath (arc, juce::PathStrokeType (trackW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // knob body: cream "Portastudio" cap
        const float bodyR = radius - trackW * 2.4f;
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillEllipse (juce::Rectangle<float> (bodyR * 2.0f, bodyR * 2.0f).withCentre (centre.translated (0.0f, 2.0f)));
        juce::ColourGradient grad (colours::cream, centre.x, centre.y - bodyR, colours::cream.darker (0.55f),
                                   centre.x, centre.y + bodyR, false);
        g.setGradientFill (grad);
        g.fillEllipse (juce::Rectangle<float> (bodyR * 2.0f, bodyR * 2.0f).withCentre (centre));
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.drawEllipse (juce::Rectangle<float> (bodyR * 2.0f, bodyR * 2.0f).withCentre (centre), 1.0f);

        // pointer
        juce::Path ptr;
        ptr.addRoundedRectangle (-1.6f, -bodyR + 3.0f, 3.2f, bodyR * 0.55f, 1.6f);
        g.setColour (colours::knobDark);
        g.fillPath (ptr, juce::AffineTransform::rotation (angle).translated (centre));
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool) override
    {
        auto r = b.getLocalBounds().toFloat().reduced (1.0f);
        const bool on = b.getToggleState();
        g.setColour (on ? colours::amber.withAlpha (highlighted ? 0.95f : 0.85f) : colours::knobDark);
        g.fillRoundedRectangle (r, 4.0f);
        g.setColour (on ? colours::amber.brighter (0.3f) : colours::outline);
        g.drawRoundedRectangle (r, 4.0f, 1.0f);
        g.setColour (on ? colours::knobDark : colours::textDim);
        g.setFont (juce::FontOptions (11.5f, juce::Font::bold));
        g.drawFittedText (b.getButtonText(), r.toNearestInt(), juce::Justification::centred, 1);
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::FontOptions (13.0f, juce::Font::bold); }
    juce::Font getPopupMenuFont() override { return juce::FontOptions (14.0f); }
    juce::Font getLabelFont (juce::Label& l) override { return l.getFont(); }

    juce::Label* createSliderTextBox (juce::Slider& s) override
    {
        auto* l = LookAndFeel_V4::createSliderTextBox (s);
        l->setFont (juce::FontOptions (11.5f));
        l->setColour (juce::Label::textColourId, colours::textDim);
        l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        return l;
    }
};
} // namespace md::ui
