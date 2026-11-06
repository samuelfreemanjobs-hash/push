#include "PresetSweepPanel.h"

namespace forge
{
namespace
{
    const juce::Colour bg (0xff16181d), panel (0xff20232a), accent (0xffe8483b), text (0xffe6e6e6), dim (0xff9aa0a6);
}

PresetSweepPanel::PresetSweepPanel (const Info& i, juce::PropertiesFile* s) : info (i), settings (s)
{
    auto label = [this] (juce::Label& l, const juce::String& t)
    {
        l.setText (t, juce::dontSendNotification);
        l.setColour (juce::Label::textColourId, dim);
        addAndMakeVisible (l);
    };

    title.setText ("Preset Sweep - " + info.instrumentName, juce::dontSendNotification);
    title.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    title.setColour (juce::Label::textColourId, accent);
    addAndMakeVisible (title);
    label (intro, "Tries every preset with a test chord in the recipe \"" + info.recipeName + "\", throws out silent, pulsing, "
                  "off-pitch, harsh and duplicate ones, ranks the rest by how well they fit the recipe, then renders a "
                  "16-pad bank for each of the best. Key, format, Smart Pick and VST copies come from the main window.");
    intro.setJustificationType (juce::Justification::topLeft);
    intro.setMinimumHorizontalScale (1.0f);

    label (sourceLabel, "Presets");
    if (info.builtIn)
    {
        sourceBox.addItem ("Built-in engines (every engine x 3 timbres)", 1);
        sourceBox.addItem ("Timbre variations of the current engine", 3);
    }
    else
    {
        sourceBox.addItem ("The plugin's preset list" + (info.numPrograms > 1 ? " (" + juce::String (info.numPrograms) + ")"
                                                                               : juce::String (" (none listed)")), 1);
        sourceBox.addItem ("Preset files: .vstpreset / .forgesound" + (info.standardFolderPresets > 0
                               ? " (" + juce::String (info.standardFolderPresets) + " found)" : juce::String()), 2);
        sourceBox.addItem ("Variations of the current sound", 3);
    }
    auto savedSource = settings != nullptr ? settings->getIntValue ("sweepSource", 0) : 0;
    if (sourceBox.indexOfItemId (savedSource) >= 0) sourceBox.setSelectedId (savedSource, juce::dontSendNotification);
    else if (info.builtIn || info.numPrograms > 1) sourceBox.setSelectedId (1, juce::dontSendNotification);
    else sourceBox.setSelectedId (info.standardFolderPresets > 0 ? 2 : 3, juce::dontSendNotification);
    sourceBox.onChange = [this] { update(); };
    addAndMakeVisible (sourceBox);

    label (folderLabel, {});
    folder = juce::File (settings != nullptr ? settings->getValue ("sweepFolder") : juce::String());
    folderButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> ("Folder with .vstpreset / .forgesound files", folder);
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                              [this] (const juce::FileChooser& fc)
                              {
                                  if (fc.getResult() != juce::File()) { folder = fc.getResult(); update(); }
                              });
    };
    addAndMakeVisible (folderButton);

    auto slider = [this] (juce::Slider& sl, double lo, double hi, double step, double value)
    {
        sl.setRange (lo, hi, step);
        sl.setValue (value, juce::dontSendNotification);
        sl.setTextBoxStyle (juce::Slider::TextBoxRight, false, 56, 22);
        sl.setColour (juce::Slider::trackColourId, accent);
        sl.onValueChange = [this] { update(); };
        addAndMakeVisible (sl);
    };
    label (keepLabel, "Keep the best");
    slider (keepSlider, 1, 32, 1, settings != nullptr ? settings->getIntValue ("sweepKeep", 12) : 12);
    label (perLabel, "Samples each");
    for (auto n : { 16, 32, 48, 64 }) perBox.addItem (juce::String (n) + (n == 16 ? " (one bank)" : juce::String (" (") + juce::String (n / 16) + " banks)"), n);
    perBox.setSelectedId (settings != nullptr ? juce::jlimit (16, 64, settings->getIntValue ("sweepPer", 16)) : 16, juce::dontSendNotification);
    if (perBox.getSelectedId() == 0) perBox.setSelectedId (16, juce::dontSendNotification);
    perBox.onChange = [this] { update(); };
    addAndMakeVisible (perBox);

    label (maxLabel, "Try at most");
    maxBox.addItem ("All presets", 1);
    for (auto n : { 50, 100, 200, 400 }) maxBox.addItem (juce::String (n) + " (spread over the list)", n);
    maxBox.setSelectedId (settings != nullptr ? settings->getIntValue ("sweepMax", 1) : 1, juce::dontSendNotification);
    if (maxBox.getSelectedId() == 0) maxBox.setSelectedId (1, juce::dontSendNotification);
    maxBox.onChange = [this] { update(); };
    addAndMakeVisible (maxBox);

    label (includeLabel, "Only names with");
    includeEditor.setTextToShowWhenEmpty ("e.g. keys, rhodes, piano  (empty = all)", dim);
    includeEditor.setText (settings != nullptr ? settings->getValue ("sweepInclude") : juce::String(), false);
    includeEditor.onTextChange = [this] { update(); };
    addAndMakeVisible (includeEditor);

    label (variationsLabel, "Variations");
    slider (variationsSlider, 8, 200, 1, settings != nullptr ? settings->getIntValue ("sweepVariations", 48) : 48);
    label (strengthLabel, "How different");
    slider (strengthSlider, 0.05, 0.8, 0.01, settings != nullptr ? settings->getDoubleValue ("sweepStrength", 0.25) : 0.25);
    strengthSlider.setTooltip ("How far each varied parameter may move: 0.1 = subtle, 0.5 = wild");

    for (auto* t : { &skipNames, &skipPulsing, &keepProbes })
    {
        t->setToggleState (true, juce::dontSendNotification);
        addAndMakeVisible (t);
    }

    label (summary, {});
    summary.setColour (juce::Label::textColourId, text);
    startButton.setColour (juce::TextButton::buttonColourId, accent);
    startButton.onClick = [this] { save(); if (onStart) onStart (current()); };
    addAndMakeVisible (startButton);

    setSize (640, 470);
    update();
}

SweepOptions PresetSweepPanel::current() const
{
    SweepOptions o;
    auto id = sourceBox.getSelectedId();
    o.source = id == 2 ? SweepSource::PresetFolder : id == 3 ? SweepSource::Variations : SweepSource::Programs;
    o.folder = folder;
    o.keep = (int) keepSlider.getValue();
    o.perPreset = perBox.getSelectedId();
    o.maxPresets = maxBox.getSelectedId() > 1 ? maxBox.getSelectedId() : 0;
    o.include = includeEditor.getText().trim();
    o.variations = (int) variationsSlider.getValue();
    o.strength = (float) strengthSlider.getValue();
    o.skipByName = skipNames.getToggleState();
    o.skipRhythmic = skipPulsing.getToggleState();
    o.keepProbes = keepProbes.getToggleState();
    return o;
}

void PresetSweepPanel::update()
{
    const auto o = current();
    const bool files = o.source == SweepSource::PresetFolder, variations = o.source == SweepSource::Variations;
    folderLabel.setVisible (files);
    folderButton.setVisible (files);
    folderLabel.setText (folder.isDirectory() ? folder.getFullPathName()
                                              : juce::String ("Standard VST3 preset folders (or choose one)"), juce::dontSendNotification);
    for (auto* c : std::initializer_list<juce::Component*> { &variationsLabel, &variationsSlider, &strengthLabel, &strengthSlider })
        c->setVisible (variations && (c == &variationsLabel || c == &variationsSlider || ! info.builtIn));
    includeLabel.setEnabled (! variations);
    includeEditor.setEnabled (! variations);

    int tries = variations ? o.variations : info.builtIn ? 75 : files ? info.standardFolderPresets : info.numPrograms;
    if (o.maxPresets > 0 && tries > o.maxPresets) tries = o.maxPresets;
    juce::String what = tries > 0 ? "Tries about " + juce::String (tries) + " presets" : juce::String ("Tries every preset it finds");
    summary.setText (what + ", keeps up to " + juce::String (o.keep) + " and renders " + juce::String (o.perPreset)
                     + " samples of each = up to " + juce::String (o.keep * o.perPreset) + " samples, one folder per preset.",
                     juce::dontSendNotification);
    startButton.setEnabled (tries > 0 || files);
    resized();
}

void PresetSweepPanel::save() const
{
    if (settings == nullptr) return;
    auto o = current();
    settings->setValue ("sweepSource", sourceBox.getSelectedId());
    settings->setValue ("sweepFolder", folder.getFullPathName());
    settings->setValue ("sweepKeep", o.keep);
    settings->setValue ("sweepPer", o.perPreset);
    settings->setValue ("sweepMax", maxBox.getSelectedId());
    settings->setValue ("sweepInclude", o.include);
    settings->setValue ("sweepVariations", o.variations);
    settings->setValue ("sweepStrength", (double) o.strength);
}

void PresetSweepPanel::paint (juce::Graphics& g) { g.fillAll (bg); }

void PresetSweepPanel::resized()
{
    auto r = getLocalBounds().reduced (16, 12);
    title.setBounds (r.removeFromTop (30));
    intro.setBounds (r.removeFromTop (54));
    r.removeFromTop (8);
    const int labelW = 120;
    auto row = [&r] (int h = 28) { auto x = r.removeFromTop (h); r.removeFromTop (6); return x; };

    auto x = row();
    sourceLabel.setBounds (x.removeFromLeft (labelW));
    sourceBox.setBounds (x);
    if (folderButton.isVisible())
    {
        x = row();
        x.removeFromLeft (labelW);
        folderButton.setBounds (x.removeFromRight (100));
        folderLabel.setBounds (x);
    }
    if (variationsSlider.isVisible())
    {
        x = row();
        variationsLabel.setBounds (x.removeFromLeft (labelW));
        variationsSlider.setBounds (x);
        if (strengthSlider.isVisible())
        {
            x = row();
            strengthLabel.setBounds (x.removeFromLeft (labelW));
            strengthSlider.setBounds (x);
        }
    }
    x = row();
    keepLabel.setBounds (x.removeFromLeft (labelW));
    keepSlider.setBounds (x.removeFromLeft (260));
    x.removeFromLeft (16);
    perLabel.setBounds (x.removeFromLeft (100));
    perBox.setBounds (x);
    x = row();
    maxLabel.setBounds (x.removeFromLeft (labelW));
    maxBox.setBounds (x.removeFromLeft (260));
    x = row();
    includeLabel.setBounds (x.removeFromLeft (labelW));
    includeEditor.setBounds (x);
    for (auto* t : { &skipNames, &skipPulsing, &keepProbes })
    {
        x = row (24);
        x.removeFromLeft (labelW);
        t->setBounds (x);
    }
    auto bottom = r.removeFromBottom (34);
    startButton.setBounds (bottom.removeFromRight (140));
    summary.setBounds (r.removeFromBottom (40));
}
}
