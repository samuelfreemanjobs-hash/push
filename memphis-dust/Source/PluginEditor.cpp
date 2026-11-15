#include "PluginEditor.h"

namespace md::ui
{
//==============================================================================
Knob::Knob (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& caption)
    : attachment (state, paramId, slider)
{
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 16);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxTextColourId, colours::textDim);
    slider.setRotaryParameters (juce::degreesToRadians (225.0f), juce::degreesToRadians (495.0f), true);
    if (auto* param = state.getParameter (paramId))
        slider.setDoubleClickReturnValue (true, param->convertFrom0to1 (param->getDefaultValue()));
    slider.setPopupDisplayEnabled (false, false, nullptr);
    addAndMakeVisible (slider);

    label.setText (caption.toUpperCase(), juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, colours::text);
    label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (label);
}

void Knob::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromTop (15));
    slider.setBounds (r);
}

//==============================================================================
Choice::Choice (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& caption)
{
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (state.getParameter (paramId)))
        box.addItemList (choice->choices, 1);
    box.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (box);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, paramId, box);

    label.setText (caption.toUpperCase(), juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    addAndMakeVisible (label);
}

void Choice::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromTop (15));
    box.setBounds (r.withSizeKeepingCentre (juce::jmin (r.getWidth() - 8, 84), 26));
}

//==============================================================================
Toggle::Toggle (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& text)
    : attachment (state, paramId, button)
{
    button.setButtonText (text);
    button.setClickingTogglesState (true);
    addAndMakeVisible (button);
}

void Toggle::resized() { button.setBounds (getLocalBounds()); }

//==============================================================================
Section::Section (juce::AudioProcessorValueTreeState& state, juce::String num, juce::String t,
                  juce::String sub, const juce::String& enableParamId)
    : number (std::move (num)), title (std::move (t)), subtitle (std::move (sub))
{
    // Sections without their own enable (e.g. MASTER) hold per-item toggles instead.
    if (enableParamId.isNotEmpty())
    {
        power = std::make_unique<Toggle> (state, enableParamId, "ON");
        addAndMakeVisible (*power);
        power->button.addListener (this);
    }
}

Section::~Section() { if (power) power->button.removeListener (this); }

void Section::refreshEnabled()
{
    const bool on = power == nullptr || power->button.getToggleState();
    for (auto& c : items) { c->setEnabled (on); c->setAlpha (on ? 1.0f : 0.4f); }
    repaint();
}

void Section::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (colours::panel);
    g.fillRoundedRectangle (r, 8.0f);
    g.setColour (colours::outline);
    g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);

    // "tape label" stripe
    const bool on = power == nullptr || power->button.getToggleState();
    g.setColour ((on ? colours::red : colours::outline).withAlpha (0.85f));
    g.fillRect (r.getX() + 12.0f, 50.0f, r.getWidth() - 24.0f, 2.0f);

    g.setColour (on ? colours::amber : colours::textDim);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText (number, 12, 10, 40, 24, juce::Justification::centredLeft);

    g.setColour (colours::cream);
    g.setFont (juce::FontOptions (13.5f, juce::Font::bold));
    g.drawFittedText (title, 50, 9, getWidth() - (power ? 104 : 60), 18, juce::Justification::centredLeft, 1, 0.75f);
    g.setColour (colours::textDim);
    g.setFont (juce::FontOptions (10.5f));
    g.drawText (subtitle, 50, 27, getWidth() - 60, 14, juce::Justification::centredLeft);
}

void Section::resized()
{
    if (power) power->setBounds (getWidth() - 50, 10, 38, 18);

    auto area = getLocalBounds().reduced (8, 0).withTrimmedTop (60).withTrimmedBottom (8);
    const int cols = 2;
    const int rows = juce::jmax (1, ((int) items.size() + cols - 1) / cols);
    const int cellW = area.getWidth() / cols;
    const int cellH = area.getHeight() / rows;

    for (size_t i = 0; i < items.size(); ++i)
    {
        const int col = (int) i % cols, row = (int) i / cols;
        auto cell = juce::Rectangle<int> (area.getX() + col * cellW, area.getY() + row * cellH, cellW, cellH).reduced (2);
        if (auto* t = dynamic_cast<Toggle*> (items[i].get()))
            t->setBounds (cell.withSizeKeepingCentre (juce::jmin (cell.getWidth() - 10, 86), 24));
        else
            items[i]->setBounds (cell);
    }
    refreshEnabled();
}

//==============================================================================
void GrMeter::setGr (float db)
{
    const float v = juce::jlimit (0.0f, 30.0f, -db);
    display = v > display ? v : display + (v - display) * 0.25f;
    if (v >= hold) { hold = v; holdCounter = 45; }
    else if (--holdCounter <= 0) hold = juce::jmax (display, hold - 0.5f);
    repaint();
}

void GrMeter::setLimiterGr (float db, bool active)
{
    limActive = active;
    const float v = juce::jlimit (0.0f, 30.0f, -db);
    limDisplay = v > limDisplay ? v : limDisplay + (v - limDisplay) * 0.15f;
}

void GrMeter::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (colours::text);
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    auto title = r.removeFromTop (16);
    g.drawText ("COMP GAIN REDUCTION", title, juce::Justification::centredLeft);
    if (limActive)
    {
        g.setColour (limDisplay > 6.0f ? colours::red : colours::amber);
        g.drawText ("LIMITER -" + juce::String (limDisplay, 1) + " dB", title, juce::Justification::centredRight);
    }

    r = r.withTrimmedRight (60.0f); // room for the numeric readout
    auto bar = r.removeFromTop (18);
    g.setColour (colours::knobDark);
    g.fillRoundedRectangle (bar, 3.0f);

    const float maxDb = 20.0f;
    auto fill = bar.reduced (2.0f);
    const float wFill = fill.getWidth() * juce::jmin (1.0f, display / maxDb);
    juce::ColourGradient grad (colours::amber, fill.getX(), 0, colours::red, fill.getRight(), 0, false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (fill.withWidth (wFill), 2.0f);

    // 6–10 dB "Memphis zone" marker
    g.setColour (colours::cream.withAlpha (0.18f));
    g.fillRect (fill.getX() + fill.getWidth() * 6.0f / maxDb, fill.getY(), fill.getWidth() * 4.0f / maxDb, fill.getHeight());

    g.setColour (colours::cream);
    const float hx = fill.getX() + fill.getWidth() * juce::jmin (1.0f, hold / maxDb);
    g.fillRect (hx - 1.0f, fill.getY(), 2.0f, fill.getHeight());

    // scale
    g.setFont (juce::FontOptions (10.0f));
    g.setColour (colours::textDim);
    for (int db : { 0, 3, 6, 10, 15, 20 })
    {
        const float x = fill.getX() + fill.getWidth() * (float) db / maxDb;
        g.drawText (juce::String (db), juce::Rectangle<float> (x - 12.0f, r.getY() + 1.0f, 24.0f, 12.0f), juce::Justification::centred);
    }
    g.setColour (colours::cream);
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("-" + juce::String (display, 1) + " dB", bar.withX (bar.getRight() + 6.0f).withWidth (56.0f), juce::Justification::centredLeft);
}
//==============================================================================
void TapeLevelMeter::setPeak (float linear)
{
    const float db = juce::jlimit (-60.0f, 6.0f, juce::Decibels::gainToDecibels (linear, -60.0f));
    displayDb = db > displayDb ? db : displayDb - 0.8f;  // ~24 dB/s fall
    if (db >= holdDb) { holdDb = db; holdCounter = 45; }
    else if (--holdCounter <= 0) holdDb = juce::jmax (displayDb, holdDb - 0.5f);
    repaint();
}

void TapeLevelMeter::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    auto title = r.removeFromTop (16);
    g.setColour (colours::text);
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText ("TAPE INPUT (PEAK)", title, juce::Justification::centredLeft);
    if (std::abs (autoDb) > 0.05f)
    {
        g.setColour (colours::amber);
        g.drawText ("AUTO " + juce::String (autoDb > 0 ? "+" : "") + juce::String (autoDb, 1) + " dB", title,
                    juce::Justification::centredRight);
    }

    r = r.withTrimmedRight (60.0f);
    auto bar = r.removeFromTop (18);
    g.setColour (colours::knobDark);
    g.fillRoundedRectangle (bar, 3.0f);
    auto fill = bar.reduced (2.0f);
    constexpr float lo = -48.0f, hi = 0.0f;
    auto xOf = [&] (float db) { return fill.getX() + fill.getWidth() * juce::jlimit (0.0f, 1.0f, (db - lo) / (hi - lo)); };

    // sweet spot: peaks around -12..-10 dBFS
    g.setColour (juce::Colour (0xff5d8a4a).withAlpha (0.45f));
    g.fillRect (juce::Rectangle<float>::leftTopRightBottom (xOf (-12.0f), fill.getY(), xOf (-10.0f), fill.getBottom()));

    const float x = xOf (displayDb);
    const juce::Colour c = displayDb > -6.0f ? colours::red : displayDb > -10.0f ? colours::amber : juce::Colour (0xff8fbf6a);
    g.setColour (c);
    g.fillRoundedRectangle (fill.withRight (x), 2.0f);
    g.setColour (colours::cream);
    g.fillRect (xOf (holdDb) - 1.0f, fill.getY(), 2.0f, fill.getHeight());

    g.setFont (juce::FontOptions (10.0f));
    g.setColour (colours::textDim);
    for (int db : { -48, -36, -24, -12, -6, 0 })
        g.drawText (juce::String (db), juce::Rectangle<float> (juce::jmax (r.getX(), xOf ((float) db) - 12.0f), r.getY() + 1.0f, 24.0f, 12.0f),
                    db == -48 ? juce::Justification::centredLeft : juce::Justification::centred);
    g.setColour (colours::cream);
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText (juce::String (holdDb, 1), bar.withX (bar.getRight() + 6.0f).withWidth (56.0f), juce::Justification::centredLeft);
}
} // namespace md::ui

//==============================================================================
using namespace md;

MemphisDustPanel::MemphisDustPanel (MemphisDustProcessor& proc)
    : processor (proc),
      sampler    (proc.apvts, "01", "SAMPLER",     "12-bit / low-rate grit",    pid::crushOn),
      cassetteEq (proc.apvts, "02", "CASSETTE EQ", "roll highs, boxy low-mids", pid::eqOn),
      tapeSec    (proc.apvts, "03", "TAPE",        "heads, transport, hiss",    pid::tapeOn),
      dbxSec     (proc.apvts, "04", "dbx II NR",   "2:1 / 1:2 compander pump",  pid::dbxOn),
      sub        (proc.apvts, "05", "808 CLIP",    "mono sub, clipped + LPF",   pid::subOn),
      comp       (proc.apvts, "06", "BUS COMP",    "FET slam / VCA glue",       pid::compOn),
      texture    (proc.apvts, "07", "TEXTURE",     "hiss + dust, ducked, narrow", pid::texOn),
      master     (proc.apvts, "08", "MASTER",      "mono lows + brickwall",     {}),
      inputKnob  (proc.apvts, pid::inputGain, "Input"),
      mixKnob    (proc.apvts, pid::mix, "Dry/Wet"),
      outputKnob (proc.apvts, pid::outputGain, "Output"),
      clipToggle (proc.apvts, pid::outClip, "CLIP"),
      autoToggle (proc.apvts, pid::autoLevel, "AUTO LEVEL")
{
    auto& s = proc.apvts;

    sampler.add<ui::Knob> (s, pid::crushRate, "Rate");
    sampler.add<ui::Knob> (s, pid::crushBits, "Bits");
    sampler.add<ui::Knob> (s, pid::crushPitch, "Pitch Trick");
    sampler.add<ui::Choice> (s, pid::crushDac, "DAC");
    sampler.add<ui::Toggle> (s, pid::crushAA, "ANTI-ALIAS");

    cassetteEq.add<ui::Knob> (s, pid::lowCut,  "Low Cut");
    cassetteEq.add<ui::Knob> (s, pid::highCut, "High Cut");
    cassetteEq.add<ui::Knob> (s, pid::boxFreq, "Box Freq");
    cassetteEq.add<ui::Knob> (s, pid::boxGain, "Box Gain");
    cassetteEq.add<ui::Knob> (s, pid::boxQ,    "Box Q");
    cassetteEq.add<ui::Knob> (s, pid::presence, "3.5k Dip");

    tapeSec.add<ui::Knob> (s, pid::tapeDrive, "Drive");
    tapeSec.add<ui::Knob> (s, pid::tapeBias,  "Bias");
    tapeSec.add<ui::Knob> (s, pid::wow,       "Wow");
    tapeSec.add<ui::Knob> (s, pid::flutter,   "Flutter");
    tapeSec.add<ui::Knob> (s, pid::dropouts,  "Dropouts");
    tapeSec.add<ui::Knob> (s, pid::hiss,      "Hiss");
    tapeSec.add<ui::Knob> (s, pid::hissDuck,  "Hiss Duck");

    dbxSec.add<ui::Knob> (s, pid::dbxBreath,  "Breath");
    dbxSec.add<ui::Knob> (s, pid::dbxRelease, "Release");
    dbxSec.add<ui::Knob> (s, pid::dbxTilt,    "Tilt");

    comp.add<ui::Knob>   (s, pid::compThresh,  "Threshold");
    comp.add<ui::Choice> (s, pid::compRatio,   "Ratio");
    comp.add<ui::Knob>   (s, pid::compAttack,  "Attack");
    comp.add<ui::Knob>   (s, pid::compRelease, "Release");
    comp.add<ui::Knob>   (s, pid::compMakeup,  "Makeup");
    comp.add<ui::Knob>   (s, pid::compMix,     "Mix");
    comp.add<ui::Choice> (s, pid::compMode,    "Mode");

    texture.add<ui::Knob> (s, pid::texHiss,    "Hiss");
    texture.add<ui::Knob> (s, pid::texDust,    "Dust");
    texture.add<ui::Knob> (s, pid::texDensity, "Density");
    texture.add<ui::Knob> (s, pid::texTone,    "Tone");
    texture.add<ui::Knob> (s, pid::texWidth,   "Width");
    texture.add<ui::Knob> (s, pid::texDuck,    "Duck");

    master.add<ui::Toggle> (s, pid::monoOn,       "MONO BASS");
    master.add<ui::Knob>   (s, pid::monoFreq,     "Mono Below");
    master.add<ui::Toggle> (s, pid::limitOn,      "LIMITER");
    master.add<ui::Knob>   (s, pid::limitDrive,   "Push");
    master.add<ui::Knob>   (s, pid::limitCeiling, "Ceiling");

    sub.add<ui::Knob> (s, pid::subFreq,  "Freq");
    sub.add<ui::Knob> (s, pid::subDrive, "Drive");
    sub.add<ui::Knob> (s, pid::subHard,  "Hardness");
    sub.add<ui::Knob> (s, pid::subLevel, "Level");

    for (auto* c : std::initializer_list<juce::Component*> { &sampler, &cassetteEq, &tapeSec, &dbxSec, &sub, &comp, &texture, &master,
                                                             &inputKnob, &mixKnob, &outputKnob, &clipToggle, &autoToggle,
                                                             &grMeter, &tapeMeter })
        addAndMakeVisible (c);

    refreshPresetBox();
    presetBox.onChange = [this]
    {
        if (const int idx = presetBox.getSelectedId() - 1; idx >= 0)
            processor.setCurrentProgram (idx);
    };
    addAndMakeVisible (presetBox);

    setSize (designWidth, designHeight);
    startTimerHz (30);
}

void MemphisDustPanel::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    for (int i = 0; i < processor.getNumPrograms(); ++i)
        presetBox.addItem (processor.getProgramName (i), i + 1);
    presetBox.setSelectedId (processor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void MemphisDustPanel::timerCallback()
{
    grMeter.setGr (processor.getGainReductionDb());
    grMeter.setLimiterGr (processor.getLimiterGrDb(),
                          processor.apvts.getRawParameterValue (pid::limitOn)->load() > 0.5f);
    tapeMeter.setPeak (processor.getTapeInputPeak());
    tapeMeter.setAutoLevelDb (processor.getAutoLevelDb());

    const float peak = processor.getOutputPeak();
    activity += ((peak > 0.001f ? 1.0f : 0.0f) - activity) * 0.08f;
    if (activity > 0.01f)
    {
        reelAngle += 0.09f * activity;
        repaint (reelArea);
    }

    if (presetBox.getSelectedId() != processor.getCurrentProgram() + 1)
        presetBox.setSelectedId (processor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void MemphisDustPanel::drawReel (juce::Graphics& g, juce::Point<float> c, float radius, float angle) const
{
    g.setColour (ui::colours::knobDark);
    g.fillEllipse (juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (c));
    g.setColour (ui::colours::cream.withAlpha (0.85f));
    g.drawEllipse (juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (c), 2.0f);
    g.fillEllipse (juce::Rectangle<float> (radius * 0.9f, radius * 0.9f).withCentre (c));

    juce::Path teeth;
    for (int i = 0; i < 6; ++i)
    {
        juce::Path t;
        t.addRectangle (-radius * 0.07f, -radius * 0.45f, radius * 0.14f, radius * 0.18f);
        teeth.addPath (t, juce::AffineTransform::rotation (angle + (float) i * juce::MathConstants<float>::pi / 3.0f));
    }
    g.setColour (ui::colours::knobDark);
    g.fillPath (teeth, juce::AffineTransform::translation (c));
    g.fillEllipse (juce::Rectangle<float> (radius * 0.28f, radius * 0.28f).withCentre (c));
}

void MemphisDustPanel::paint (juce::Graphics& g)
{
    using namespace ui;
    g.fillAll (colours::bg);

    // subtle horizontal "tape" grain
    g.setColour (juce::Colours::white.withAlpha (0.015f));
    for (int y = 0; y < getHeight(); y += 3)
        g.drawHorizontalLine (y, 0.0f, (float) getWidth());

    // Header
    g.setColour (colours::cream);
    g.setFont (juce::FontOptions (30.0f, juce::Font::bold));
    g.drawText ("MEMPHIS DUST", 22, 14, 320, 34, juce::Justification::centredLeft);
    g.setColour (colours::amber);
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("CASSETTE DRUM DESTROYER  //  4-TRACK  -  12-BIT  -  FET", 24, 48, 520, 16, juce::Justification::centredLeft);
    g.setColour (colours::textDim);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("PRESET", presetBox.getX() - 60, presetBox.getY(), 54, presetBox.getHeight(), juce::Justification::centredRight);

    // Signal-flow hint between sections
    g.setColour (colours::textDim);
    g.setFont (juce::FontOptions (10.5f));
    g.drawText ("FLOW:  IN > 08 mono > 01 > 02 > [ 04 encode > 03 tape > 04 decode ] > 05 > 06 > mix > 07 texture > out > clip > 08 limiter     "
                "hiss + dropouts live between dbx encode/decode, so they breathe like a real 424",
                22, sampler.getBottom() + 4, 1200, 14, juce::Justification::centredLeft);

    // Cassette window in footer
    auto win = reelArea.toFloat();
    g.setColour (colours::panel);
    g.fillRoundedRectangle (win, 10.0f);
    g.setColour (colours::outline);
    g.drawRoundedRectangle (win.reduced (0.5f), 10.0f, 1.0f);

    auto label = win.reduced (14.0f, 10.0f);
    g.setColour (colours::cream.withAlpha (0.92f));
    g.fillRoundedRectangle (label, 6.0f);
    g.setColour (colours::red);
    g.fillRect (label.getX(), label.getY() + 22.0f, label.getWidth(), 4.0f);
    g.setColour (colours::knobDark);
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("SIDE A  -  DRUMS  -  C-90", label.withHeight (20.0f).reduced (10.0f, 0.0f), juce::Justification::centredLeft);
    g.drawText ("TYPE I NORMAL", label.withHeight (20.0f).reduced (10.0f, 0.0f), juce::Justification::centredRight);

    auto tapeWindow = label.reduced (60.0f, 0.0f).withTrimmedTop (34.0f).withTrimmedBottom (8.0f);
    g.setColour (juce::Colour (0xff2a211b));
    g.fillRoundedRectangle (tapeWindow, tapeWindow.getHeight() * 0.5f);

    const float r = tapeWindow.getHeight() * 0.42f;
    const auto lc = juce::Point<float> (tapeWindow.getX() + tapeWindow.getHeight() * 0.5f + 6.0f, tapeWindow.getCentreY());
    const auto rc = juce::Point<float> (tapeWindow.getRight() - tapeWindow.getHeight() * 0.5f - 6.0f, tapeWindow.getCentreY());
    g.setColour (juce::Colour (0xff4a2f22));
    g.fillRect (juce::Rectangle<float> (lc.x, lc.y - r * 0.15f, rc.x - lc.x, r * 0.3f));
    drawReel (g, lc, r, reelAngle);
    drawReel (g, rc, r, reelAngle * 1.3f);
}

void MemphisDustPanel::resized()
{
    auto area = getLocalBounds().reduced (16, 0);

    auto header = area.removeFromTop (74);
    presetBox.setBounds (header.removeFromRight (240).withSizeKeepingCentre (240, 28));

    auto body = area.removeFromTop (450);
    const int gap = 10;
    const int w = (body.getWidth() - gap * 7) / 8;
    for (auto* s : { &sampler, &cassetteEq, &tapeSec, &dbxSec, &sub, &comp, &texture, &master })
    {
        s->setBounds (body.removeFromLeft (w));
        body.removeFromLeft (gap);
    }

    area.removeFromTop (26);
    auto footer = area.removeFromTop (150);
    reelArea = footer.removeFromLeft (440);

    footer.removeFromLeft (24);
    const int kw = 96;
    inputKnob.setBounds (footer.removeFromLeft (kw));
    mixKnob.setBounds (footer.removeFromLeft (kw));
    outputKnob.setBounds (footer.removeFromLeft (kw));

    footer.removeFromLeft (18);
    auto right = footer;
    auto toggles = right.removeFromTop (30).withTrimmedTop (6);
    clipToggle.setBounds (toggles.removeFromLeft (100));
    toggles.removeFromLeft (10);
    autoToggle.setBounds (toggles.removeFromLeft (120));
    right.removeFromTop (8);
    tapeMeter.setBounds (right.removeFromTop (54));
    grMeter.setBounds (right.removeFromTop (54));
}

//==============================================================================
MemphisDustEditor::MemphisDustEditor (MemphisDustProcessor& proc)
    : AudioProcessorEditor (&proc), processor (proc), panel (proc)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (panel);

    constexpr int W = MemphisDustPanel::designWidth, H = MemphisDustPanel::designHeight;
    setResizable (true, true);
    setResizeLimits (W / 2, H / 2, W * 3 / 2, H * 3 / 2);
    getConstrainer()->setFixedAspectRatio ((double) W / (double) H);

    // remember the window size with the session
    const int savedW = processor.apvts.state.getProperty ("uiWidth", (int) (W * 0.85));
    setSize (savedW, juce::roundToInt (savedW * (double) H / W));
}

MemphisDustEditor::~MemphisDustEditor() { setLookAndFeel (nullptr); }

void MemphisDustEditor::resized()
{
    const float scale = (float) getWidth() / (float) MemphisDustPanel::designWidth;
    panel.setTransform (juce::AffineTransform::scale (scale));
    panel.setBounds (0, 0, MemphisDustPanel::designWidth, MemphisDustPanel::designHeight);
    processor.apvts.state.setProperty ("uiWidth", getWidth(), nullptr);
}
