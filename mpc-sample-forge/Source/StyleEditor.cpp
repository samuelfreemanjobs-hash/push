#include "StyleEditor.h"
#include "BuiltInSynth.h"

namespace forge
{
namespace
{
    const juce::Colour bg (0xff16181d), panel (0xff20232a), accent (0xffe8483b), text (0xffe6e6e6), dim (0xff9aa0a6);
}

juce::File StyleEditor::userStyleFolder()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("MPCSampleForge").getChildFile ("Styles");
}

juce::Array<Style> StyleEditor::loadUserStyles (juce::StringArray& errors)
{
    juce::Array<Style> out;
    for (auto& f : userStyleFolder().findChildFiles (juce::File::findFiles, false, "*.json"))
    {
        Style st;
        juce::String err;
        if (loadStyleFile (f, st, err)) out.add (st);
        else errors.add (f.getFileName() + ": " + err);
    }
    return out;
}

juce::Slider& StyleEditor::makeRange (juce::Slider& s, double lo, double hi, double step, double a, double b, const juce::String& suffix)
{
    s.setSliderStyle (juce::Slider::TwoValueHorizontal);
    s.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0); // the label above shows the range
    s.setRange (lo, hi, step);
    s.setMinAndMaxValues (a, b, juce::dontSendNotification);
    s.setTextValueSuffix (suffix);
    s.setColour (juce::Slider::trackColourId, accent);
    addAndMakeVisible (s);
    return s;
}

StyleEditor::StyleEditor (const Style& b) : base (b)
{
    auto label = [this] (juce::Label& l, const juce::String& t) { l.setText (t, juce::dontSendNotification); l.setColour (juce::Label::textColourId, dim); addAndMakeVisible (l); };

    title.setText ("Style editor - based on \"" + base.name + "\"", juce::dontSendNotification);
    title.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    title.setColour (juce::Label::textColourId, accent);
    addAndMakeVisible (title);

    label (nameLabel, "Name");
    nameEditor.setText (base.name.endsWith ("(mine)") ? base.name : base.name + " (mine)");
    addAndMakeVisible (nameEditor);

    label (engineLabel, "Engine");
    auto ids = engineIds();
    for (int i = 0; i < ids.size(); ++i) engineBox.addItem (ids[i], i + 1);
    engineBox.addItem ("mixed", 100);
    engineBox.setText (base.engine, juce::dontSendNotification);
    addAndMakeVisible (engineBox);

    label (scaleLabel, "Scale");
    auto scales = allScaleNames();
    for (int i = 0; i < scales.size(); ++i) scaleBox.addItem (scales[i], i + 1);
    scaleBox.setText (base.scale, juce::dontSendNotification);
    addAndMakeVisible (scaleBox);

    label (bankLabel, "Scale bank");
    bankBox.addItem ("notes", 1); bankBox.addItem ("chords", 2); bankBox.addItem ("progressions", 3);
    bankBox.setText (base.bankType(), juce::dontSendNotification);
    addAndMakeVisible (bankBox);

    label (chordsLabel, "Chord types (ticked = used)");
    for (auto& q : allQualities())
    {
        auto* t = chordToggles.add (new juce::ToggleButton (q.suffix.isEmpty() ? juce::String ("maj") : q.suffix));
        t->setComponentID (q.id);
        for (auto& w : base.chords) if (w.value == q.id) t->setToggleState (true, juce::dontSendNotification);
        addAndMakeVisible (t);
    }
    label (voicingsLabel, "Voicings");
    for (auto& v : allVoicingIds())
    {
        auto* t = voicingToggles.add (new juce::ToggleButton (v));
        Voicing vv;
        parseVoicing (v, vv);
        for (auto& w : base.voicings) if (w.value == vv) t->setToggleState (true, juce::dontSendNotification);
        addAndMakeVisible (t);
    }

    auto sliderLabel = [this] (const juce::String& t) { auto* l = sliderLabels.add (new juce::Label ({}, t)); l->setColour (juce::Label::textColourId, dim); addAndMakeVisible (l); };
    sliderLabel ({}); makeRange (chordRange, 24, 96, 1, base.chordRootLow, base.chordRootHigh, "");
    sliderLabel ({}); makeRange (keyRange, 21, 108, 1, base.keyLow, base.keyHigh, "");
    sliderLabel ({}); makeRange (velocity, 1, 127, 1, base.velocityLow, base.velocityHigh, "");
    sliderLabel ({}); makeRange (hold, 0.05, 5.0, 0.05, base.holdLow, base.holdHigh, " s");
    auto notes = [] (const juce::Slider& sl)
    {
        return juce::String ((int) sl.getMinValue()) + "-" + juce::String ((int) sl.getMaxValue())
               + "  (" + midiNoteName ((int) sl.getMinValue()) + " to " + midiNoteName ((int) sl.getMaxValue()) + ")";
    };
    auto refresh = [this, notes]
    {
        sliderLabels[0]->setText ("Chord register (root): " + notes (chordRange), juce::dontSendNotification);
        sliderLabels[1]->setText ("Single-note register: " + notes (keyRange), juce::dontSendNotification);
        sliderLabels[2]->setText ("Velocity: " + juce::String ((int) velocity.getMinValue()) + "-" + juce::String ((int) velocity.getMaxValue()), juce::dontSendNotification);
        sliderLabels[3]->setText ("Hold: " + juce::String (hold.getMinValue(), 2) + " - " + juce::String (hold.getMaxValue(), 2) + " s", juce::dontSendNotification);
    };
    for (auto* sl : { &chordRange, &keyRange, &velocity, &hold }) sl->onValueChange = refresh;
    refresh();

    auto single = [this, &sliderLabel] (juce::Slider& s, const juce::String& t, double lo, double hi, double step, double v, const juce::String& suffix)
    {
        sliderLabel (t);
        s.setSliderStyle (juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::TextBoxRight, false, 70, 22);
        s.setRange (lo, hi, step);
        s.setValue (v, juce::dontSendNotification);
        s.setTextValueSuffix (suffix);
        s.setColour (juce::Slider::trackColourId, accent);
        addAndMakeVisible (s);
    };
    single (chordRatio, "Share of chords (vs single keys)", 0.0, 1.0, 0.05, base.chordRatio, "");
    single (strum, "Strum / roll (ms)", 0.0, 120.0, 1.0, base.strumMs, " ms");
    single (stabChance, "Chance a chord is a short stab", 0.0, 1.0, 0.05, base.stabChance, "");
    single (maxLength, "Max length (0 = natural)", 0.0, 6.0, 0.05, base.processing.maxLength, " s");

    saveButton.setColour (juce::TextButton::buttonColourId, accent);
    saveButton.onClick = [this]
    {
        Style st;
        juce::String err;
        if (! build (st, err)) { status.setText (err, juce::dontSendNotification); return; }
        userStyleFolder().createDirectory();
        auto file = userStyleFolder().getChildFile (st.id + ".json");
        if (! file.replaceWithText (juce::JSON::toString (st.toVar())))
        {
            status.setText ("Could not save " + file.getFullPathName(), juce::dontSendNotification);
            return;
        }
        status.setText ("Saved " + file.getFileName() + " - it loads every time the app starts.", juce::dontSendNotification);
        if (onSave) onSave (st);
    };
    applyButton.onClick = [this]
    {
        Style st;
        juce::String err;
        if (! build (st, err)) { status.setText (err, juce::dontSendNotification); return; }
        status.setText ("Using \"" + st.name + "\" for this session (not saved).", juce::dontSendNotification);
        if (onApply) onApply (st);
    };
    resetButton.onClick = [this]
    {
        // back to the base style's settings
        for (auto* t : chordToggles) { t->setToggleState (false, juce::dontSendNotification); for (auto& w : base.chords) if (w.value == t->getComponentID()) t->setToggleState (true, juce::dontSendNotification); }
        chordRange.setMinAndMaxValues (base.chordRootLow, base.chordRootHigh);
        keyRange.setMinAndMaxValues (base.keyLow, base.keyHigh);
        velocity.setMinAndMaxValues (base.velocityLow, base.velocityHigh);
        hold.setMinAndMaxValues (base.holdLow, base.holdHigh);
        chordRatio.setValue (base.chordRatio); strum.setValue (base.strumMs); stabChance.setValue (base.stabChance);
        maxLength.setValue (base.processing.maxLength);
        if (chordRange.onValueChange) chordRange.onValueChange();
        status.setText ("Reset to \"" + base.name + "\".", juce::dontSendNotification);
    };
    for (auto* button : { &saveButton, &applyButton, &resetButton }) addAndMakeVisible (button);

    status.setColour (juce::Label::textColourId, dim);
    status.setText ("Effects, recipe processing and pad notes come from the base style.", juce::dontSendNotification);
    addAndMakeVisible (status);

    setSize (900, 700);
}

bool StyleEditor::build (Style& out, juce::String& error) const
{
    auto v = base.toVar();
    auto* o = v.getDynamicObject();
    auto name = nameEditor.getText().trim();
    if (name.isEmpty()) { error = "Give the style a name."; return false; }
    o->setProperty ("name", name);
    o->setProperty ("id", "my-" + name.toLowerCase().retainCharacters ("abcdefghijklmnopqrstuvwxyz0123456789 -").replaceCharacter (' ', '-'));
    o->setProperty ("tag", name.retainCharacters ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
    // keep the recipe's compact tag (e.g. EPBell) when the name still starts with the base style's name
    o->setProperty ("short", name.startsWith (base.name) && base.shortTag.isNotEmpty()
                               ? base.shortTag
                               : name.retainCharacters ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789").substring (0, 8));
    o->setProperty ("engine", engineBox.getText());
    o->setProperty ("scale", scaleBox.getText());
    o->setProperty ("scaleBank", bankBox.getText());
    o->setProperty ("description", "My style, based on " + base.name + ".");

    auto* chords = new juce::DynamicObject();
    for (auto* t : chordToggles)
        if (t->getToggleState())
        {
            float weight = 2.0f; // keep the base style's emphasis where it had one
            for (auto& w : base.chords) if (w.value == t->getComponentID()) weight = w.weight;
            chords->setProperty (t->getComponentID(), weight);
        }
    if (chords->getProperties().isEmpty()) { error = "Tick at least one chord type."; delete chords; return false; }
    o->setProperty ("chords", juce::var (chords));

    auto* voicings = new juce::DynamicObject();
    for (auto* t : voicingToggles)
        if (t->getToggleState()) voicings->setProperty (t->getButtonText(), 2.0);
    if (voicings->getProperties().isEmpty()) { error = "Tick at least one voicing."; delete voicings; return false; }
    o->setProperty ("voicings", juce::var (voicings));

    auto range = [] (const juce::Slider& s) { juce::Array<juce::var> a { s.getMinValue(), s.getMaxValue() }; return juce::var (a); };
    o->setProperty ("chordRootRange", range (chordRange));
    o->setProperty ("keyRange", range (keyRange));
    o->setProperty ("velocity", range (velocity));
    o->setProperty ("hold", range (hold));
    o->setProperty ("chordRatio", chordRatio.getValue());
    o->setProperty ("strumMs", strum.getValue());
    o->setProperty ("stabChance", stabChance.getValue());
    o->setProperty ("scaleBankOnly", false);
    if (auto* p = o->getProperty ("processing").getDynamicObject()) p->setProperty ("maxLength", maxLength.getValue());

    if (! Style::fromVar (v, out, error)) return false;
    return true;
}

void StyleEditor::paint (juce::Graphics& g) { g.fillAll (bg); }

void StyleEditor::resized()
{
    auto r = getLocalBounds().reduced (14);
    title.setBounds (r.removeFromTop (28));
    r.removeFromTop (6);

    auto row = r.removeFromTop (26);
    nameLabel.setBounds (row.removeFromLeft (50)); nameEditor.setBounds (row.removeFromLeft (230)); row.removeFromLeft (14);
    engineLabel.setBounds (row.removeFromLeft (52)); engineBox.setBounds (row.removeFromLeft (130)); row.removeFromLeft (14);
    scaleLabel.setBounds (row.removeFromLeft (44)); scaleBox.setBounds (row.removeFromLeft (140)); row.removeFromLeft (14);
    bankLabel.setBounds (row.removeFromLeft (76)); bankBox.setBounds (row.removeFromLeft (120));
    r.removeFromTop (10);

    chordsLabel.setBounds (r.removeFromTop (20));
    const int cols = 8, cellW = r.getWidth() / cols, cellH = 22;
    auto grid = r.removeFromTop (((chordToggles.size() + cols - 1) / cols) * cellH);
    for (int i = 0; i < chordToggles.size(); ++i)
        chordToggles[i]->setBounds (grid.getX() + (i % cols) * cellW, grid.getY() + (i / cols) * cellH, cellW, cellH);
    r.removeFromTop (8);
    voicingsLabel.setBounds (r.removeFromTop (20));
    auto vrow = r.removeFromTop (cellH);
    for (int i = 0; i < voicingToggles.size(); ++i)
        voicingToggles[i]->setBounds (vrow.getX() + i * cellW, vrow.getY(), cellW, cellH);
    r.removeFromTop (10);

    auto bottom = r.removeFromBottom (30);
    saveButton.setBounds (bottom.removeFromRight (160)); bottom.removeFromRight (8);
    applyButton.setBounds (bottom.removeFromRight (100)); bottom.removeFromRight (8);
    resetButton.setBounds (bottom.removeFromRight (80)); bottom.removeFromRight (8);
    status.setBounds (bottom);
    r.removeFromBottom (8);

    juce::Slider* sliders[] = { &chordRange, &keyRange, &velocity, &hold, &chordRatio, &strum, &stabChance, &maxLength };
    auto colW = r.getWidth() / 2 - 10;
    for (int i = 0; i < 8; ++i)
    {
        auto col = i % 2, line = i / 2;
        auto cell = juce::Rectangle<int> (r.getX() + col * (colW + 20), r.getY() + line * 52, colW, 50);
        sliderLabels[i]->setBounds (cell.removeFromTop (20));
        sliders[i]->setBounds (cell.removeFromTop (26));
    }
}
}
