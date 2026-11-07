#include "PluginEditor.h"

using namespace kf;

namespace
{
    const juce::Colour bgColour      { 0xff121419 };
    const juce::Colour panelColour   { 0xff1b1e25 };
    const juce::Colour outlineColour { 0xff2c303b };
    const juce::Colour textColour    { 0xffd8dce6 };
    const juce::Colour dimText       { 0xff8a91a3 };
    const juce::Colour accentColour  { 0xffff7a3d };

    juce::Font uiFont (float size, bool bold = false)
    {
        return juce::Font (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
    }

    constexpr int knobW = 66, knobH = 86, choiceW = 112, choiceH = 44;
}

//==============================================================================
// Look and feel
//==============================================================================
KitForgeLookAndFeel::KitForgeLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, bgColour);
    setColour (juce::Label::textColourId, textColour);
    setColour (juce::Slider::textBoxTextColourId, dimText);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxHighlightColourId, accentColour.withAlpha (0.4f));
    setColour (juce::Slider::rotarySliderFillColourId, accentColour);
    setColour (juce::Slider::rotarySliderOutlineColourId, outlineColour);
    setColour (juce::TextButton::buttonColourId, juce::Colour (0xff262a34));
    setColour (juce::TextButton::buttonOnColourId, accentColour);
    setColour (juce::TextButton::textColourOffId, textColour);
    setColour (juce::TextButton::textColourOnId, juce::Colours::black);
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff262a34));
    setColour (juce::ComboBox::outlineColourId, outlineColour);
    setColour (juce::ComboBox::textColourId, textColour);
    setColour (juce::ComboBox::arrowColourId, dimText);
    setColour (juce::PopupMenu::backgroundColourId, panelColour);
    setColour (juce::PopupMenu::textColourId, textColour);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, accentColour);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::black);
    setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff262a34));
    setColour (juce::TextEditor::textColourId, textColour);
    setColour (juce::TextEditor::highlightColourId, accentColour.withAlpha (0.4f));
    setColour (juce::CaretComponent::caretColourId, accentColour);
}

void KitForgeLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                            float startAngle, float endAngle, juce::Slider& s)
{
    const auto bounds = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (4.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float lineW = 3.5f;
    const float arcR = radius - lineW * 0.5f;
    const auto fill = s.findColour (juce::Slider::rotarySliderFillColourId);

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (outlineColour);
    g.strokePath (track, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const bool bipolar = s.getProperties().getWithDefault ("bipolar", false);
    const float valueAngle = startAngle + pos * (endAngle - startAngle);
    const float fromAngle = bipolar ? (startAngle + endAngle) * 0.5f : startAngle;

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f,
                         juce::jmin (fromAngle, valueAngle), juce::jmax (fromAngle, valueAngle), true);
    g.setColour (fill);
    g.strokePath (value, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Knob body
    const float bodyR = radius - lineW - 3.0f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a3f4c), centre.x, centre.y - bodyR,
                                             juce::Colour (0xff22252d), centre.x, centre.y + bodyR, false));
    g.fillEllipse (juce::Rectangle<float> (bodyR * 2, bodyR * 2).withCentre (centre));

    // Pointer
    juce::Path pointer;
    pointer.addRoundedRectangle (-1.5f, -bodyR + 2.0f, 3.0f, bodyR * 0.55f, 1.5f);
    g.setColour (s.isEnabled() ? juce::Colours::white.withAlpha (0.9f) : dimText);
    g.fillPath (pointer, juce::AffineTransform::rotation (valueAngle).translated (centre));
}

void KitForgeLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour& bg,
                                                bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    auto c = b.getToggleState() ? b.findColour (juce::TextButton::buttonOnColourId) : bg;
    if (down) c = c.brighter (0.2f);
    else if (highlighted) c = c.brighter (0.08f);

    g.setColour (c);
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (b.getToggleState() ? c.brighter (0.3f) : outlineColour);
    g.drawRoundedRectangle (r, 4.0f, 1.0f);
}

juce::Font KitForgeLookAndFeel::getTextButtonFont (juce::TextButton&, int h)
{
    return uiFont (juce::jmin (13.0f, (float) h * 0.5f), true);
}

juce::Font KitForgeLookAndFeel::getLabelFont (juce::Label& l)
{
    return l.getFont();
}

juce::Font KitForgeLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return uiFont (13.0f);
}

void KitForgeLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int,
                                        juce::ComboBox& box)
{
    auto r = juce::Rectangle<int> (width, height).toFloat().reduced (0.5f);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (r, 4.0f, 1.0f);

    juce::Path arrow;
    const float ax = (float) width - 14.0f, ay = (float) height * 0.5f;
    arrow.addTriangle (ax - 4.0f, ay - 2.0f, ax + 4.0f, ay - 2.0f, ax, ay + 3.0f);
    g.setColour (box.findColour (juce::ComboBox::arrowColourId));
    g.fillPath (arrow);
}

//==============================================================================
// Knob / Choice
//==============================================================================
Knob::Knob (const juce::String& name, bool bipolar)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, knobW, 15);
    slider.setRotaryParameters (juce::degreesToRadians (225.0f), juce::degreesToRadians (495.0f), true);
    slider.getProperties().set ("bipolar", bipolar);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxTextColourId, dimText);
    addAndMakeVisible (slider);

    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (uiFont (11.5f, true));
    label.setColour (juce::Label::textColourId, dimText);
    label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (label);
}

void Knob::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromTop (15));
    slider.setBounds (r);
}

void Knob::attach (juce::AudioProcessorValueTreeState& apvts, const juce::String& id)
{
    attachment.reset();
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, id, slider);
}

void Knob::setColour (juce::Colour c)
{
    slider.setColour (juce::Slider::rotarySliderFillColourId, c);
}

Choice::Choice (const juce::String& name)
{
    addAndMakeVisible (box);
    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centredLeft);
    label.setFont (uiFont (11.5f, true));
    label.setColour (juce::Label::textColourId, dimText);
    addAndMakeVisible (label);
}

void Choice::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromTop (16));
    box.setBounds (r.removeFromTop (24));
}

void Choice::attach (juce::AudioProcessorValueTreeState& apvts, const juce::String& id)
{
    attachment.reset();
    box.clear (juce::dontSendNotification);
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (id)))
        box.addItemList (p->choices, 1);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, id, box);
}

//==============================================================================
// Track header
//==============================================================================
TrackHeader::TrackHeader (KitForgeProcessor& p, int t) : proc (p), track (t) {}

void TrackHeader::update()
{
    const int count = proc.getTriggerCount (track);
    if (count != lastTriggerCount)
    {
        lastTriggerCount = count;
        flash = 1.0f;
    }
    else
    {
        flash *= 0.75f;
    }
    repaint();
}

void TrackHeader::paint (juce::Graphics& g)
{
    const auto colour = trackColour (track);
    auto r = getLocalBounds().toFloat().reduced (2.0f, 3.0f);

    g.setColour (selected ? colour.withAlpha (0.18f) : juce::Colour (0xff1f222a));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (selected ? colour : outlineColour);
    g.drawRoundedRectangle (r, 4.0f, 1.0f);

    // colour strip
    g.setColour (colour);
    g.fillRoundedRectangle (r.withWidth (4.0f), 2.0f);

    // trigger LED
    auto led = juce::Rectangle<float> (8.0f, 8.0f).withCentre ({ r.getRight() - 12.0f, r.getCentreY() });
    g.setColour (colour.withAlpha (0.15f + 0.85f * flash));
    g.fillEllipse (led);

    const int typeIndex = (int) getParamValue (proc.apvts, voiceParamID (track, Type));
    g.setColour (dimText);
    g.setFont (uiFont (11.0f, true));
    g.drawText (juce::String (track + 1), r.withTrimmedLeft (10.0f).withWidth (16.0f), juce::Justification::centredLeft);
    g.setColour (textColour);
    g.setFont (uiFont (13.5f, true));
    g.drawText (voiceTypeNames()[typeIndex].toUpperCase(), r.withTrimmedLeft (28.0f).withTrimmedRight (22.0f),
                juce::Justification::centredLeft);
}

void TrackHeader::mouseDown (const juce::MouseEvent&)
{
    proc.previewTrack (track);
    if (onSelect)
        onSelect (track);
}

//==============================================================================
// Voice panel
//==============================================================================
VoicePanel::VoicePanel (KitForgeProcessor& p) : proc (p)
{
    for (auto* c : std::initializer_list<juce::Component*> {
             &type, &forgeButton, &previewButton,
             &wave, &pitch, &pEnv, &pDecay, &tDecay, &tLevel, &fm, &ratio,
             &noiseType, &filterMode, &nLevel, &nDecay, &cutoff, &reso, &burst,
             &click, &attack, &drive, &level, &pan, &choke })
        addAndMakeVisible (c);

    forgeButton.setTooltip ("Generate a new sound for this track based on its drum type");
    forgeButton.onClick = [this]
    {
        const auto t = (VoiceType) (int) getParamValue (proc.apvts, voiceParamID (track, Type));
        auto voice = KitFactory::forgeVoice (t, rng);
        voice[Pan] = getParamValue (proc.apvts, voiceParamID (track, Pan));
        voice[Choke] = getParamValue (proc.apvts, voiceParamID (track, Choke));
        KitFactory::applyVoice (proc.apvts, track, voice);
        proc.previewTrack (track);
        if (onVoiceForged) onVoiceForged();
    };
    previewButton.onClick = [this] { proc.previewTrack (track); };

    setTrack (0);
}

void VoicePanel::setTrack (int t)
{
    track = t;
    auto& a = proc.apvts;

    type.attach (a, voiceParamID (t, Type));
    wave.attach (a, voiceParamID (t, Wave));
    noiseType.attach (a, voiceParamID (t, NoiseType));
    filterMode.attach (a, voiceParamID (t, FilterMode));

    const std::pair<Knob*, VP> knobs[] = {
        { &pitch, Pitch }, { &pEnv, PEnvAmt }, { &pDecay, PEnvDecay }, { &tDecay, ToneDecay },
        { &tLevel, ToneLevel }, { &fm, FmAmt }, { &ratio, FmRatio },
        { &nLevel, NoiseLevel }, { &nDecay, NoiseDecay }, { &cutoff, Cutoff }, { &reso, Reso }, { &burst, Burst },
        { &click, Click }, { &attack, Attack }, { &drive, Drive }, { &level, Level }, { &pan, Pan }, { &choke, Choke },
    };

    for (auto& [knob, vp] : knobs)
    {
        knob->attach (a, voiceParamID (t, vp));
        knob->setColour (trackColour (t));
    }

    forgeButton.setColour (juce::TextButton::buttonColourId, trackColour (t).withMultipliedBrightness (0.55f));
    repaint();
}

void VoicePanel::paint (juce::Graphics& g)
{
    const auto colour = trackColour (track);

    g.setColour (panelColour);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 8.0f);
    g.setColour (colour.withAlpha (0.6f));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f), 8.0f, 1.0f);

    // Title
    const int typeIndex = (int) getParamValue (proc.apvts, voiceParamID (track, Type));
    g.setColour (colour);
    g.setFont (uiFont (20.0f, true));
    g.drawText ("TRACK " + juce::String (track + 1), headerArea.withWidth (120), juce::Justification::centredLeft);
    g.setColour (textColour);
    g.setFont (uiFont (14.0f));
    g.drawText ("Synth engine  /  " + voiceTypeNames()[typeIndex],
                headerArea.withTrimmedLeft (120).withWidth (260), juce::Justification::centredLeft);

    auto section = [&] (juce::Rectangle<int> area, const juce::String& title)
    {
        g.setColour (juce::Colour (0xff171a20));
        g.fillRoundedRectangle (area.toFloat(), 6.0f);
        g.setColour (outlineColour);
        g.drawRoundedRectangle (area.toFloat().reduced (0.5f), 6.0f, 1.0f);
        g.setColour (colour.withAlpha (0.85f));
        g.setFont (uiFont (11.0f, true));
        g.drawText (title, area.reduced (10, 4).removeFromTop (14), juce::Justification::centredLeft);
    };

    section (toneArea, "TONE OSCILLATOR");
    section (noiseArea, "NOISE / METAL");
    section (outArea, "SHAPE / OUTPUT");
}

void VoicePanel::resized()
{
    auto r = getLocalBounds().reduced (14, 10);

    headerArea = r.removeFromTop (44);
    {
        auto h = headerArea;
        previewButton.setBounds (h.removeFromRight (70).withSizeKeepingCentre (70, 30));
        h.removeFromRight (8);
        forgeButton.setBounds (h.removeFromRight (130).withSizeKeepingCentre (130, 30));
        h.removeFromRight (12);
        type.setBounds (h.removeFromRight (choiceW + 20).withTrimmedTop (2));
    }
    r.removeFromTop (6);

    auto layoutSection = [] (juce::Rectangle<int> area, std::initializer_list<juce::Component*> choices,
                             std::initializer_list<Knob*> knobs)
    {
        auto inner = area.reduced (10, 6).withTrimmedTop (16);
        for (auto* c : choices)
        {
            auto col = inner.removeFromLeft (choiceW);
            c->setBounds (col.withHeight (choiceH).withY (col.getY() + 4));
            inner.removeFromLeft (8);
        }
        auto knobCol = inner;
        const int n = (int) knobs.size();
        const int step = juce::jmin (knobW + 8, knobCol.getWidth() / juce::jmax (1, n));
        for (auto* k : knobs)
            k->setBounds (knobCol.removeFromLeft (step).withSizeKeepingCentre (knobW, knobH));
    };

    const int rowH = knobH + 30;
    toneArea = r.removeFromTop (rowH);
    r.removeFromTop (8);
    auto row2 = r.removeFromTop (rowH);
    noiseArea = row2.removeFromLeft ((int) (row2.getWidth() * 0.54f));
    row2.removeFromLeft (8);
    outArea = row2;

    layoutSection (toneArea, { &wave }, { &pitch, &pEnv, &pDecay, &tDecay, &tLevel, &fm, &ratio });
    layoutSection (noiseArea, { &noiseType, &filterMode }, { &nLevel, &nDecay, &cutoff, &reso, &burst });
    layoutSection (outArea, {}, { &click, &attack, &drive, &level, &pan, &choke });
}

//==============================================================================
// Editor
//==============================================================================
KitForgeEditor::KitForgeEditor (KitForgeProcessor& p)
    : AudioProcessorEditor (&p), proc (p), grid (p), voicePanel (p)
{
    setLookAndFeel (&lnf);

    // ---- Transport ----
    playButton.setClickingTogglesState (false);
    playButton.onClick = [this] { proc.setInternalPlaying (! proc.isInternalPlaying()); };
    addAndMakeVisible (playButton);

    for (auto* k : { &tempo, &swing, &length, &master })
        addAndMakeVisible (k);
    tempo.attach (proc.apvts, ids::tempo);
    swing.attach (proc.apvts, ids::swing);
    length.attach (proc.apvts, ids::length);
    master.attach (proc.apvts, ids::master);

    // ---- Kit factory ----
    kitBox.setTextWhenNothingSelected ("Factory kits...");
    kitBox.addItemList (KitFactory::factoryKitNames(), 1);
    kitBox.onChange = [this]
    {
        const int idx = kitBox.getSelectedItemIndex();
        if (idx >= 0)
        {
            proc.loadKit (KitFactory::factoryKit (idx));
            refreshKitName();
        }
        kitBox.setSelectedId (0, juce::dontSendNotification);
    };
    addAndMakeVisible (kitBox);

    kitName.setEditable (false, true, false);
    kitName.setFont (uiFont (18.0f, true));
    kitName.setColour (juce::Label::textColourId, accentColour);
    kitName.setTooltip ("Double-click to rename the kit");
    kitName.onTextChange = [this] { proc.setKitName (kitName.getText()); };
    addAndMakeVisible (kitName);

    forgeKitButton.setColour (juce::TextButton::buttonColourId, accentColour.withMultipliedBrightness (0.6f));
    forgeKitButton.onClick = [this]
    {
        proc.loadKit (KitFactory::forgeKit (rng));
        refreshKitName();
    };
    mutateButton.onClick = [this]
    {
        KitFactory::mutateKit (proc.apvts, rng, 0.5f);
        if (! proc.getKitName().endsWith ("*"))
            proc.setKitName (proc.getKitName() + "*");
        refreshKitName();
    };
    saveButton.onClick = [this] { saveKit(); };
    loadButton.onClick = [this] { loadKitFromFile(); };
    for (auto* b : { &forgeKitButton, &mutateButton, &saveButton, &loadButton })
        addAndMakeVisible (b);

    // ---- Patterns ----
    for (int i = 0; i < numPatterns; ++i)
    {
        auto& b = patternButtons[(size_t) i];
        b.setButtonText (juce::String::charToString ((juce::juce_wchar) ('A' + i)));
        b.onClick = [this, i] { setParamValue (proc.apvts, ids::pattern, (float) i); };
        addAndMakeVisible (b);
    }

    copyButton.onClick = [this]
    {
        const int pat = proc.getCurrentPattern();
        for (int t = 0; t < numTracks; ++t)
            for (int s = 0; s < numSteps; ++s)
                clipboard[(size_t) (t * numSteps + s)] = proc.patterns.get (pat, t, s);
        hasClipboard = true;
    };
    pasteButton.onClick = [this]
    {
        if (! hasClipboard) return;
        const int pat = proc.getCurrentPattern();
        for (int t = 0; t < numTracks; ++t)
            for (int s = 0; s < numSteps; ++s)
                proc.patterns.set (pat, t, s, clipboard[(size_t) (t * numSteps + s)]);
        grid.repaint();
    };
    clearButton.onClick = [this] { proc.patterns.clear (proc.getCurrentPattern()); grid.repaint(); };
    generateButton.onClick = [this]
    {
        KitFactory::generatePattern (proc.patterns, proc.getCurrentPattern(), proc.apvts, rng);
        grid.repaint();
    };
    for (auto* b : { &copyButton, &pasteButton, &clearButton, &generateButton })
        addAndMakeVisible (b);

    // ---- Tracks ----
    for (int t = 0; t < numTracks; ++t)
    {
        headers[(size_t) t] = std::make_unique<TrackHeader> (proc, t);
        headers[(size_t) t]->onSelect = [this] (int tr) { selectTrack (tr); };
        addAndMakeVisible (*headers[(size_t) t]);

        auto& m = muteButtons[(size_t) t];
        auto& s = soloButtons[(size_t) t];
        m.setButtonText ("M");
        s.setButtonText ("S");
        m.setClickingTogglesState (true);
        s.setClickingTogglesState (true);
        m.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffe0b23a));
        s.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff47c0ff));
        addAndMakeVisible (m);
        addAndMakeVisible (s);
        buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            proc.apvts, voiceParamID (t, Mute), m));
        buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            proc.apvts, voiceParamID (t, Solo), s));
    }

    grid.onTrackClicked = [this] (int t) { selectTrack (t); };
    addAndMakeVisible (grid);

    voicePanel.onVoiceForged = [this]
    {
        if (! proc.getKitName().endsWith ("*"))
            proc.setKitName (proc.getKitName() + "*");
        refreshKitName();
    };
    addAndMakeVisible (voicePanel);

    selectTrack (0);
    refreshKitName();

    setSize (1180, 800);
    startTimerHz (30);
}

KitForgeEditor::~KitForgeEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void KitForgeEditor::selectTrack (int t)
{
    if (voicePanel.getTrack() != t)
        voicePanel.setTrack (t);
    grid.setSelectedTrack (t);
    for (int i = 0; i < numTracks; ++i)
    {
        headers[(size_t) i]->selected = (i == t);
        headers[(size_t) i]->repaint();
    }
}

void KitForgeEditor::refreshKitName()
{
    kitName.setText (proc.getKitName(), juce::dontSendNotification);
}

void KitForgeEditor::saveKit()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("KitForge Kits");
    dir.createDirectory();
    const auto name = proc.getKitName().trimCharactersAtEnd ("*");

    chooser = std::make_unique<juce::FileChooser> ("Save kit", dir.getChildFile (juce::File::createLegalFileName (name) + ".kitforge"),
                                                   "*.kitforge");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this, name] (const juce::FileChooser& fc)
                          {
                              auto file = fc.getResult();
                              if (file == juce::File()) return;
                              file = file.withFileExtension ("kitforge");

                              auto kitNameToSave = name;
                              if (kitNameToSave.isEmpty()) kitNameToSave = file.getFileNameWithoutExtension();

                              auto kit = KitFactory::captureKit (proc.apvts, kitNameToSave);
                              if (auto xml = KitFactory::kitToXml (kit); xml->writeTo (file))
                              {
                                  proc.setKitName (kitNameToSave);
                                  refreshKitName();
                              }
                          });
}

void KitForgeEditor::loadKitFromFile()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("KitForge Kits");
    chooser = std::make_unique<juce::FileChooser> ("Load kit", dir, "*.kitforge");
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this] (const juce::FileChooser& fc)
                          {
                              const auto file = fc.getResult();
                              if (! file.existsAsFile()) return;

                              Kit kit;
                              if (auto xml = juce::parseXML (file); xml != nullptr && KitFactory::kitFromXml (*xml, kit))
                              {
                                  proc.loadKit (kit);
                                  refreshKitName();
                              }
                              else
                              {
                                  juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon,
                                                                          "KitForge", "That file isn't a KitForge kit.");
                              }
                          });
}

void KitForgeEditor::timerCallback()
{
    const bool host = proc.isHostPlaying();
    const bool internal = proc.isInternalPlaying();
    playButton.setToggleState (host || internal, juce::dontSendNotification);
    playButton.setButtonText (host ? "HOST" : (internal ? "STOP" : "PLAY"));

    const int pat = proc.getCurrentPattern();
    for (int i = 0; i < numPatterns; ++i)
        patternButtons[(size_t) i].setToggleState (i == pat, juce::dontSendNotification);

    for (auto& h : headers)
        h->update();

    grid.repaint();
}

void KitForgeEditor::paint (juce::Graphics& g)
{
    g.fillAll (bgColour);

    // Header
    g.setColour (panelColour);
    g.fillRect (headerArea);
    g.setColour (outlineColour);
    g.drawHorizontalLine (headerArea.getBottom(), 0.0f, (float) getWidth());

    auto logo = headerArea.reduced (18, 0).removeFromLeft (190);
    g.setColour (accentColour);
    g.setFont (uiFont (30.0f, true));
    g.drawText ("KITFORGE", logo.removeFromTop (54).withTrimmedTop (12), juce::Justification::centredLeft);
    g.setColour (dimText);
    g.setFont (uiFont (11.0f, true));
    g.drawText ("DRUM SYNTH  +  SEQUENCER", logo.withTrimmedTop (-6).removeFromTop (16), juce::Justification::centredLeft);

    // Kit factory box
    g.setColour (juce::Colour (0xff171a20));
    g.fillRoundedRectangle (factoryArea.toFloat(), 6.0f);
    g.setColour (outlineColour);
    g.drawRoundedRectangle (factoryArea.toFloat().reduced (0.5f), 6.0f, 1.0f);
    g.setColour (accentColour.withAlpha (0.9f));
    g.setFont (uiFont (11.0f, true));
    g.drawText ("KIT FACTORY", factoryArea.reduced (10, 5).removeFromTop (14), juce::Justification::centredLeft);

    // Pattern strip label
    g.setColour (dimText);
    g.setFont (uiFont (11.0f, true));
    g.drawText ("PATTERN", patternArea.withWidth (70), juce::Justification::centredLeft);

    // Step numbers above the grid
    const auto gb = grid.getBounds();
    const float cw = (float) gb.getWidth() / (float) numSteps;
    g.setFont (uiFont (10.5f, true));
    for (int s = 0; s < numSteps; ++s)
    {
        g.setColour (s % 4 == 0 ? textColour : dimText);
        g.drawText (juce::String (s + 1), juce::Rectangle<float> (gb.getX() + s * cw, (float) gb.getY() - 16.0f, cw, 14.0f),
                    juce::Justification::centred);
    }

    // Help text
    g.setColour (dimText.withAlpha (0.7f));
    g.setFont (uiFont (11.0f));
    g.drawText ("Click: toggle / drag to paint   Right-click: cycle velocity   Alt-drag: fine velocity   MIDI C1-G1 plays tracks 1-8",
                patternArea.withTrimmedLeft (patternArea.getWidth() - 620), juce::Justification::centredRight);
}

void KitForgeEditor::resized()
{
    auto r = getLocalBounds();

    // ---- Header ----
    headerArea = r.removeFromTop (106);
    {
        auto h = headerArea.reduced (12, 8);
        h.removeFromLeft (206);

        playButton.setBounds (h.removeFromLeft (84).withSizeKeepingCentre (84, 44));
        h.removeFromLeft (12);
        for (auto* k : { &tempo, &swing, &length, &master })
            k->setBounds (h.removeFromLeft (knobW + 4).withSizeKeepingCentre (knobW, knobH));

        h.removeFromLeft (16);
        factoryArea = h;
        auto f = h.reduced (10, 6).withTrimmedTop (16);
        auto top = f.removeFromTop (30);
        kitName.setBounds (top.removeFromLeft (230));
        top.removeFromLeft (8);
        kitBox.setBounds (top.withSizeKeepingCentre (top.getWidth(), 26));
        f.removeFromTop (8);
        auto bottom = f.removeFromTop (30);
        const int bw = (bottom.getWidth() - 3 * 8) / 4;
        for (auto* b : { &forgeKitButton, &mutateButton, &saveButton, &loadButton })
        {
            b->setBounds (bottom.removeFromLeft (bw));
            bottom.removeFromLeft (8);
        }
    }

    r.reduce (12, 0);
    r.removeFromTop (10);

    // ---- Pattern strip ----
    patternArea = r.removeFromTop (30);
    {
        auto p = patternArea;
        p.removeFromLeft (70);
        for (auto& b : patternButtons)
        {
            b.setBounds (p.removeFromLeft (34));
            p.removeFromLeft (4);
        }
        p.removeFromLeft (16);
        for (auto* b : { &copyButton, &pasteButton, &clearButton, &generateButton })
        {
            b->setBounds (p.removeFromLeft (b == &generateButton ? 90 : 64));
            p.removeFromLeft (4);
        }
    }

    r.removeFromTop (24);

    // ---- Tracks + grid ----
    const int rowH = 34;
    gridArea = r.removeFromTop (rowH * numTracks);
    {
        auto g = gridArea;
        auto headerCol = g.removeFromLeft (170);
        auto muteCol = g.removeFromLeft (32);
        auto soloCol = g.removeFromLeft (32);
        g.removeFromLeft (8);
        grid.setBounds (g);

        for (int t = 0; t < numTracks; ++t)
        {
            headers[(size_t) t]->setBounds (headerCol.removeFromTop (rowH));
            muteButtons[(size_t) t].setBounds (muteCol.removeFromTop (rowH).reduced (2, 5));
            soloButtons[(size_t) t].setBounds (soloCol.removeFromTop (rowH).reduced (2, 5));
        }
    }

    r.removeFromTop (14);
    voicePanel.setBounds (r.removeFromTop (r.getHeight() - 12));
}
