#include "PluginEditor.h"

using namespace phonk;
using juce::Graphics; using juce::Rectangle;

//==============================================================================
PhonkLook::PhonkLook()
{
    setColour (juce::Slider::textBoxTextColourId, ui::text);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, ui::dim);
    setColour (juce::ComboBox::backgroundColourId, ui::bg);
    setColour (juce::ComboBox::textColourId, ui::text);
    setColour (juce::ComboBox::outlineColourId, ui::edge);
    setColour (juce::ComboBox::arrowColourId, ui::amber);
    setColour (juce::PopupMenu::backgroundColourId, ui::panel);
    setColour (juce::PopupMenu::textColourId, ui::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, ui::accent);
}

void PhonkLook::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&)
{
    const auto r = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (6.f);
    const float d = juce::jmin (r.getWidth(), r.getHeight()), rad = d * 0.5f;
    const auto c = r.getCentre();
    const float ang = a0 + pos * (a1 - a0);
    juce::Path track, val;
    track.addCentredArc (c.x, c.y, rad, rad, 0.f, a0, a1, true);
    val.addCentredArc (c.x, c.y, rad, rad, 0.f, a0, ang, true);
    g.setColour (ui::edge);  g.strokePath (track, juce::PathStrokeType (3.f));
    g.setColour (ui::accent); g.strokePath (val, juce::PathStrokeType (3.f));
    g.setColour (ui::bg);    g.fillEllipse (c.x - rad * 0.72f, c.y - rad * 0.72f, rad * 1.44f, rad * 1.44f);
    g.setColour (ui::amber);
    g.drawLine (c.x + std::sin (ang) * rad * 0.25f, c.y - std::cos (ang) * rad * 0.25f,
                c.x + std::sin (ang) * rad * 0.68f, c.y - std::cos (ang) * rad * 0.68f, 2.f);
}

void PhonkLook::drawButtonBackground (Graphics& g, juce::Button& b, const juce::Colour&, bool hover, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (down ? ui::accent : hover ? ui::edge.brighter (0.2f) : ui::edge);
    g.fillRoundedRectangle (r, 4.f);
}

void PhonkLook::drawToggleButton (Graphics& g, juce::ToggleButton& b, bool hover, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState();
    g.setColour (on ? ui::accent : hover ? ui::edge.brighter (0.2f) : ui::edge);
    g.fillRoundedRectangle (r, 4.f);
    g.setColour (on ? juce::Colours::white : ui::text);
    g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

//==============================================================================
class PhonkEditor::Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState& s, const char* id, const juce::String& name)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 14);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
        slider.setDoubleClickReturnValue (true, s.getParameter (id)->convertFrom0to1 (s.getParameter (id)->getDefaultValue()));
        addAndMakeVisible (slider);
        label.setText (name, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font (juce::FontOptions (11.f)));
        addAndMakeVisible (label);
        attach = std::make_unique<juce::SliderParameterAttachment> (*s.getParameter (id), slider);
    }
    void resized() override
    {
        auto r = getLocalBounds();
        label.setBounds (r.removeFromBottom (14));
        slider.setBounds (r);
    }
    juce::Slider slider;
    juce::Label label;
private:
    std::unique_ptr<juce::SliderParameterAttachment> attach;
};

//==============================================================================
class PhonkEditor::PadGrid : public juce::Component, private juce::Timer
{
public:
    explicit PadGrid (PhonkProcessor& p) : proc (p) { startTimerHz (30); }
    ~PadGrid() override { stopTimer(); }

    void paint (Graphics& g) override
    {
        for (int pad = 0; pad < kNumPads; ++pad)
        {
            const auto r = cell (pad).toFloat().reduced (3.f);
            const auto col = ui::voiceColour (pad / kVariations);
            const float fl = proc.padFlash (pad);
            g.setColour (ui::bg.interpolatedWith (col, 0.15f + 0.85f * fl));
            g.fillRoundedRectangle (r, 5.f);
            g.setColour (col.withAlpha (0.55f));
            g.drawRoundedRectangle (r, 5.f, 1.2f);
            g.setColour (fl > 0.5f ? juce::Colours::black : ui::text);
            g.setFont (juce::Font (juce::FontOptions (11.f, juce::Font::bold)));
            g.drawText (juce::String (pad % kVariations + 1), r.toNearestInt().reduced (6, 4), juce::Justification::bottomLeft);
            g.setColour (ui::dim);
            g.setFont (juce::Font (juce::FontOptions (10.f)));
            g.drawText (juce::MidiMessage::getMidiNoteName (kFirstMidiNote + pad, true, true, 3),
                        r.toNearestInt().reduced (6, 4), juce::Justification::topRight);
        }
        static const char* names[] = { "KICK", "SNARE", "C.HAT", "O.HAT", "COWBELL" };
        g.setFont (juce::Font (juce::FontOptions (10.f, juce::Font::bold)));
        for (int v = 0; v < NumVoices; ++v)
        {
            g.setColour (ui::voiceColour (v));
            g.drawText (names[v], v * colW, 0, colW, headerH, juce::Justification::centred);
        }
    }
    void mouseDown (const juce::MouseEvent& e) override { press (e); }
    void mouseUp (const juce::MouseEvent&) override { release(); }

private:
    static constexpr int headerH = 18;
    int colW = 64, rowH = 40;
    void resized() override { colW = getWidth() / NumVoices; rowH = (getHeight() - headerH) / kVariations; }
    Rectangle<int> cell (int pad) const { return { (pad / kVariations) * colW, headerH + (pad % kVariations) * rowH, colW, rowH }; }
    int padAt (juce::Point<int> p) const
    {
        for (int i = 0; i < kNumPads; ++i) if (cell (i).contains (p)) return i;
        return -1;
    }
    void press (const juce::MouseEvent& e)
    {
        held = padAt (e.getPosition());
        if (held >= 0) proc.keyboardState.noteOn (1, kFirstMidiNote + held, 0.85f);
    }
    void release()
    {
        if (held >= 0) proc.keyboardState.noteOff (1, kFirstMidiNote + held, 0.f);
        held = -1;
    }
    void timerCallback() override { proc.decayFlashes (0.12f); repaint(); }

    PhonkProcessor& proc;
    int held = -1;
};

//==============================================================================
class PhonkEditor::PatternView : public juce::Component, private juce::Timer
{
public:
    explicit PatternView (PhonkProcessor& p) : proc (p) { startTimerHz (30); }
    ~PatternView() override { stopTimer(); }

    void paint (Graphics& g) override
    {
        const auto& pat = proc.uiPattern();
        const int total = pat.totalSteps();
        const auto area = getLocalBounds().toFloat().reduced (4.f);
        const float labelW = 0.f, laneH = area.getHeight() / NumVoices, stepW = (area.getWidth() - labelW) / total;
        const double swing = proc.apvts.getRawParameterValue (ids::swing)->load();

        for (int s = 0; s < total; ++s)   // grid
        {
            g.setColour (s % 16 == 0 ? ui::dim : (s % 4 == 0 ? ui::edge.brighter (0.3f) : ui::edge.withAlpha (0.45f)));
            g.fillRect (area.getX() + s * stepW, area.getY(), s % 16 == 0 ? 2.f : 1.f, area.getHeight());
        }
        for (const auto& e : pat.events)
        {
            const double t = std::fmod (eventTime (e, swing), (double) total);
            const float x = area.getX() + (float) t * stepW, y = area.getY() + e.voice * laneH;
            g.setColour (ui::voiceColour (e.voice).withAlpha (0.35f + 0.65f * e.velocity));
            g.fillRoundedRectangle (x + 1.f, y + 3.f, juce::jmax (3.f, stepW - 3.f), laneH - 6.f, 2.f);
        }
        const float cur = proc.currentStep();
        if (cur >= 0.f)
        {
            g.setColour (juce::Colours::white.withAlpha (0.85f));
            g.fillRect (area.getX() + cur * stepW, area.getY(), 2.f, area.getHeight());
        }
    }
private:
    void timerCallback() override { repaint(); }
    PhonkProcessor& proc;
};

//==============================================================================
class PhonkEditor::DragHandle : public juce::Component
{
public:
    explicit DragHandle (PhonkProcessor& p) : proc (p) { setMouseCursor (juce::MouseCursor::DraggingHandCursor); }
    void paint (Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (ui::amber.withAlpha (0.18f)); g.fillRoundedRectangle (r, 4.f);
        g.setColour (ui::amber);                   g.drawRoundedRectangle (r, 4.f, 1.2f);
        g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
        g.drawText ("DRAG LOOP TO DAW", getLocalBounds(), juce::Justification::centred);
    }
    void mouseDown (const juce::MouseEvent&) override { file = proc.renderLoopFile(); dragging = false; }
    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (dragging || ! file.existsAsFile() || e.getDistanceFromDragStart() < 6) return;
        dragging = true;
        juce::DragAndDropContainer::performExternalDragDropOfFiles ({ file.getFullPathName() }, false, this,
                                                                    [safe = juce::Component::SafePointer<DragHandle> (this)] { if (safe != nullptr) safe->dragging = false; });
    }
private:
    PhonkProcessor& proc;
    juce::File file;
    bool dragging = false;
};

//==============================================================================
PhonkEditor::PhonkEditor (PhonkProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&look);

    pads = std::make_unique<PadGrid> (p);              addAndMakeVisible (*pads);
    patternView = std::make_unique<PatternView> (p);   addAndMakeVisible (*patternView);
    dragHandle = std::make_unique<DragHandle> (p);     addAndMakeVisible (*dragHandle);

    auto addKnob = [&] (const char* id, const char* name)
    {
        knobs.push_back (std::make_unique<Knob> (p.apvts, id, name));
        addAndMakeVisible (*knobs.back());
    };
    // order matters: resized() lays these out in panel groups
    for (auto [id, n] : std::initializer_list<std::pair<const char*, const char*>> {
             { ids::bpm, "BPM" }, { ids::swing, "Swing" }, { ids::variation, "Variation" }, { ids::rolls, "Hat Rolls" }, { ids::cowbell, "Cowbell" },
             { ids::kickTune, "Kick Tune" }, { ids::kickDecay, "Kick Decay" }, { ids::snareTune, "Snare Tune" }, { ids::snareDecay, "Snare Decay" },
             { ids::hatDecay, "Hat Decay" }, { ids::cowTune, "Cow Tune" }, { ids::grit, "Grit" }, { ids::bitDepth, "Bits" },
             { ids::hiss, "Hiss" }, { ids::tone, "Tone" },
             { ids::busDrive, "Drive" }, { ids::busHiss, "Hiss" }, { ids::busTone, "LPF" }, { ids::wow, "Wow" }, { ids::master, "Master" },
             { ids::lvlKick, "Kick" }, { ids::lvlSnare, "Snare" }, { ids::lvlHat, "Hats" }, { ids::lvlCow, "Cowbell" } })
        addKnob (id, n);

    for (auto* b : { &exportButton, &newPatternButton, &rerollButton }) addAndMakeVisible (*b);
    for (auto* b : { &playButton, &hostButton }) addAndMakeVisible (*b);
    barsBox.addItemList ({ "1 bar", "2 bars", "4 bars" }, 1);
    addAndMakeVisible (barsBox);
    barsAttach = std::make_unique<juce::ComboBoxParameterAttachment> (*p.apvts.getParameter (ids::bars), barsBox);
    playAttach = std::make_unique<juce::ButtonParameterAttachment> (*p.apvts.getParameter (ids::run), playButton);
    hostAttach = std::make_unique<juce::ButtonParameterAttachment> (*p.apvts.getParameter (ids::hostSync), hostButton);

    newPatternButton.onClick = [this] { proc.newPattern(); };
    rerollButton.onClick = [this] { proc.rerollKit(); };
    exportButton.onClick = [this] { chooseExportFolder(); };
    startTimerHz (4);
    setSize (940, 700);   // last: resized() needs every child to exist
}

PhonkEditor::~PhonkEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void PhonkEditor::timerCallback()
{
    if (status.isNotEmpty() && juce::Time::getMillisecondCounter() - statusSetAt > 8000) { status.clear(); repaint(); }
}

void PhonkEditor::chooseExportFolder()
{
    if (proc.isExporting()) return;
    chooser = std::make_unique<juce::FileChooser> ("Choose where to create the Memphis_Phonk_MPC_Kit folder",
                                                   juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "", true);
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                          [safe = juce::Component::SafePointer<PhonkEditor> (this)] (const juce::FileChooser& fc)
    {
        if (safe == nullptr || fc.getResult() == juce::File()) return;
        const auto dest = fc.getResult().getChildFile ("Memphis_Phonk_MPC_Kit");
        safe->setStatus ("Rendering kit and loops...");
        if (! safe->proc.exportKitTo (dest, [safe, dest] (int n)
        {
            if (safe == nullptr) return;
            safe->setStatus (n > 0 ? "Exported " + juce::String (n) + " WAVs to " + dest.getFullPathName()
                                   : "Export failed - check folder permissions");
            if (n > 0) dest.revealToUser();
        }))
            safe->setStatus ("An export is already running");
    });
}

//==============================================================================
void PhonkEditor::paint (Graphics& g)
{
    g.fillAll (ui::bg);
    g.setColour (ui::accent);
    g.setFont (juce::Font (juce::FontOptions (28.f, juce::Font::bold)));
    g.drawText ("PHONK FACTORY", 20, 10, 320, 36, juce::Justification::centredLeft);
    g.setColour (ui::dim);
    g.setFont (juce::Font (juce::FontOptions (11.f)));
    g.drawText ("MEMPHIS DRUM KIT & LOOP GENERATOR", 22, 40, 320, 14, juce::Justification::centredLeft);

    for (auto& pn : panels)
    {
        g.setColour (ui::panel);  g.fillRoundedRectangle (pn.bounds.toFloat(), 8.f);
        g.setColour (ui::edge);   g.drawRoundedRectangle (pn.bounds.toFloat().reduced (0.5f), 8.f, 1.f);
        g.setColour (ui::amber);
        g.setFont (juce::Font (juce::FontOptions (11.f, juce::Font::bold)));
        g.drawText (pn.title, pn.bounds.getX() + 12, pn.bounds.getY() + 5, 200, 14, juce::Justification::centredLeft);
    }
    if (status.isNotEmpty())
    {
        g.setColour (ui::text);
        g.setFont (juce::Font (juce::FontOptions (12.f)));
        g.drawText (status, 20, getHeight() - 22, getWidth() - 40, 18, juce::Justification::centredLeft);
    }
}

void PhonkEditor::resized()
{
    if (dragHandle == nullptr || knobs.size() < 24) return;
    auto area = getLocalBounds().reduced (16, 10);
    panels.clear();

    auto header = area.removeFromTop (46);
    dragHandle->setBounds (header.removeFromRight (170).reduced (0, 8));
    header.removeFromRight (8);
    exportButton.setBounds (header.removeFromRight (110).reduced (0, 8));
    area.removeFromTop (8);

    auto top = area.removeFromTop (214);
    auto padArea = top.removeFromLeft (340);
    panels.push_back ({ "PADS  (MIDI C1-G2)", padArea });
    pads->setBounds (padArea.reduced (10, 22));
    top.removeFromLeft (8);
    panels.push_back ({ "PATTERN", top });
    patternView->setBounds (top.reduced (10, 22));
    area.removeFromTop (8);

    const int kw = 74, kh = 100;
    size_t ki = 0;
    auto row = [&] (const juce::String& title, int count, int extraRight, std::function<void (Rectangle<int>)> placeExtra)
    {
        auto r = area.removeFromTop (kh + 26);
        panels.push_back ({ title, r });
        auto inner = r.reduced (10, 0).withTrimmedTop (22);
        for (int i = 0; i < count; ++i) knobs[ki++]->setBounds (inner.removeFromLeft (kw));
        if (extraRight > 0) { inner.removeFromLeft (8); placeExtra (inner); }
        area.removeFromTop (8);
    };
    row ("SEQUENCER", 5, 1, [&] (Rectangle<int> r)
    {
        auto col = r.removeFromLeft (110).withTrimmedTop (4);
        playButton.setBounds (col.removeFromTop (28));  col.removeFromTop (6);
        hostButton.setBounds (col.removeFromTop (28));
        r.removeFromLeft (8);
        auto col2 = r.removeFromLeft (110).withTrimmedTop (4);
        barsBox.setBounds (col2.removeFromTop (28));    col2.removeFromTop (6);
        newPatternButton.setBounds (col2.removeFromTop (28));
    });
    row ("KIT  -  re-renders the one-shots", 10, 1, [&] (Rectangle<int> r)
    {
        rerollButton.setBounds (r.removeFromLeft (100).withTrimmedTop (30).withHeight (28));
    });
    {   // tape bus + mix share a row
        auto r = area.removeFromTop (kh + 26);
        auto busArea = r.removeFromLeft (kw * 5 + 20);
        panels.push_back ({ "TAPE BUS", busArea });
        auto bi = busArea.reduced (10, 0).withTrimmedTop (22);
        for (int i = 0; i < 5; ++i) knobs[ki++]->setBounds (bi.removeFromLeft (kw));
        r.removeFromLeft (8);
        auto mixArea = r.removeFromLeft (kw * 4 + 20);
        panels.push_back ({ "MIX", mixArea });
        auto mi = mixArea.reduced (10, 0).withTrimmedTop (22);
        for (int i = 0; i < 4; ++i) knobs[ki++]->setBounds (mi.removeFromLeft (kw));
    }
}
