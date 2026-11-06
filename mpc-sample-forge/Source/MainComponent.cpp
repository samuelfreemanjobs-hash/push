#include "MainComponent.h"
#include "ProgressionLibrary.h"
#include "BuiltInSynth.h"
#include "DiagnosticsPanel.h"
#include "ReviewPanel.h"
#include "StyleEditor.h"

namespace forge
{
namespace
{
    const juce::Colour bg (0xff16181d), panel (0xff20232a), accent (0xffe8483b), text (0xffe6e6e6), dim (0xff9aa0a6);

    class EditorWindow final : public juce::DocumentWindow
    {
    public:
        EditorWindow (juce::AudioProcessorEditor* editor, const juce::String& name, std::function<void()> closed)
            : DocumentWindow (name, bg, DocumentWindow::closeButton), onClosed (std::move (closed))
        {
            setUsingNativeTitleBar (true);
            setContentOwned (editor, true);
            setResizable (editor->isResizable(), false);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            if (onClosed) juce::MessageManager::callAsync (onClosed);
        }

    private:
        std::function<void()> onClosed;
    };

    enum { builtInId = 1, firstInstalledId = 100 };

    /** A resizable window that owns a panel and tells us when it is closed. */
    struct ToolWindow final : juce::DocumentWindow
    {
        ToolWindow (const juce::String& title, juce::Component* content, std::function<void()> onClose)
            : DocumentWindow (title, bg, DocumentWindow::closeButton), closed (std::move (onClose))
        {
            setUsingNativeTitleBar (true);
            setContentOwned (content, true);
            setResizable (true, false);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }
        void closeButtonPressed() override { if (closed) juce::MessageManager::callAsync (closed); }
        std::function<void()> closed;
    };
}

MainComponent::MainComponent()
{
    juce::PropertiesFile::Options o;
    o.applicationName = "MPCSampleForge";
    o.filenameSuffix = ".settings";
   #if JUCE_LINUX
    o.folderName = ".config/MPCSampleForge";  // keep it out of the visible home folder
   #else
    o.folderName = "MPCSampleForge";
   #endif
    o.osxLibrarySubFolder = "Application Support";
    props.setStorageParameters (o);
    auto* settings = props.getUserSettings();

    auto setupLabel = [this] (juce::Label& l, const juce::String& t)
    {
        l.setText (t, juce::dontSendNotification);
        l.setColour (juce::Label::textColourId, dim);
        addAndMakeVisible (l);
    };

    title.setText ("MPC SAMPLE FORGE", juce::dontSendNotification);
    title.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    title.setColour (juce::Label::textColourId, accent);
    addAndMakeVisible (title);
    diagnosticsButton.setTooltip ("System report, self-test and log file - use this when something goes wrong.");
    diagnosticsButton.onClick = [this] { openDiagnostics(); };
    addAndMakeVisible (diagnosticsButton);

    setupLabel (pluginLabel, "Instrument");
    setupLabel (styleLabel, "Style");
    setupLabel (keyLabel, "Key / Scale");
    setupLabel (countLabel, "Samples");
    setupLabel (seedLabel, "Seed");
    setupLabel (prefixLabel, "Prefix");
    setupLabel (audioLabel, "Format");
    setupLabel (outLabel, "Output");
    setupLabel (engineLabel, "Engine");
    setupLabel (rootLabel, "16 Levels root");
    styleDesc.setColour (juce::Label::textColourId, text);
    styleDesc.setJustificationType (juce::Justification::topLeft);
    addAndMakeVisible (styleDesc);
    outPath.setColour (juce::Label::textColourId, text);
    addAndMakeVisible (outPath);

    // ---------------------------------------------------------- instrument
    installedPlugins = host.findInstalledVST3();
    pluginBox.addItem ("Built-in Synth", builtInId);
    if (! installedPlugins.isEmpty())
    {
        pluginBox.addSeparator();
        for (int i = 0; i < installedPlugins.size(); ++i)
            pluginBox.addItem (juce::File (installedPlugins[i]).getFileNameWithoutExtension(), firstInstalledId + i);
    }
    pluginBox.onChange = [this] { choosePlugin (pluginBox.getSelectedId()); };
    addAndMakeVisible (pluginBox);

    browseButton.onClick = [this] { browseForPlugin(); };
    editButton.onClick = [this] { openEditor(); };
    saveSoundButton.onClick = [this] { saveSound(); };
    loadSoundButton.onClick = [this] { loadSound(); };
    for (auto* b : { &browseButton, &editButton, &saveSoundButton, &loadSoundButton })
        addAndMakeVisible (b);

    // ---------------------------------------------------------- style
    styles = builtInStyles();
    for (int i = 0; i < styles.size(); ++i)
        styleBox.addItem (styles[i].name, i + 1);
    {
        juce::StringArray errors;
        for (auto& mine : StyleEditor::loadUserStyles (errors))
        {
            styles.add (mine);
            styleBox.addItem (mine.name.endsWith ("(mine)") ? mine.name : mine.name + " (mine)", styles.size());
        }
        for (auto& e : errors) log ("Could not load your style " + e);
    }
    styleBox.onChange = [this] { styleChanged(); };
    addAndMakeVisible (styleBox);

    progressionBox.addItem ("All sets (--count / 16, by seed)", 1);
    for (auto& set : progressionLibrary ("trap"))
        progressionBox.addItem ("#" + juce::String (set.number) + "  " + set.keyName + ":  " + set.main.text, set.number + 1);
    progressionBox.setSelectedId (juce::jlimit (1, 21, settings->getIntValue ("progressionSet", 1)), juce::dontSendNotification);
    progressionBox.setTooltip ("Which progression set to render: one bank = main progression + 2 alternates + 808 roots. "
                               "The chord buttons below play the chosen set. Random keys = the set's original key.");
    progressionBox.onChange = [this]
    {
        props.getUserSettings()->setValue ("progressionSet", progressionBox.getSelectedId());
        lastPlan.clear();
        styleChanged();
    };
    addChildComponent (progressionBox);
    editStyleButton.setTooltip ("Change chord types, voicings, register, velocity, hold ... with checkboxes and sliders, "
                                "then save it as your own style");
    editStyleButton.onClick = [this] { openStyleEditor(); };
    addAndMakeVisible (editStyleButton);
    styleFileButton.onClick = [this] { loadStyleJson(); };
    addAndMakeVisible (styleFileButton);

    keyBox.addItem ("Random keys", 1);
    for (int pc = 0; pc < 12; ++pc)
        keyBox.addItem (pitchClassName (pc), pc + 2);
    keyBox.setSelectedId (1);
    addAndMakeVisible (keyBox);

    scaleBox.addItem ("Style's scale", 1);
    auto scales = allScaleNames();
    for (int i = 0; i < scales.size(); ++i)
        scaleBox.addItem (scales[i], i + 2);
    scaleBox.setSelectedId (1);
    addAndMakeVisible (scaleBox);

    bankBox.addItem ("Scale banks: off", 1);
    bankBox.addItem ("Scale banks: style's type", 2);
    bankBox.addItem ("Scale banks: notes", 3);
    bankBox.addItem ("Scale banks: chords", 4);
    bankBox.addItem ("Scale banks: progressions", 5);
    bankBox.setSelectedId (settings->getIntValue ("scaleBank", 1), juce::dontSendNotification);
    bankBox.setTooltip ("Each bank of 16 pads is one sound climbing your key's scale - single notes, or the diatonic chord "
                        "of each degree - so every pad is in tune without 16 Levels. Samples 64 = 4 banks.");
    bankBox.onChange = [this] { props.getUserSettings()->setValue ("scaleBank", bankBox.getSelectedId()); lastPlan.clear(); };
    addAndMakeVisible (bankBox);

    countSlider.setRange (1, 500, 1);
    countSlider.setValue (100);
    addAndMakeVisible (countSlider);

    seedEditor.setInputRestrictions (10, "0123456789");
    seedEditor.setText (juce::String (juce::Random::getSystemRandom().nextInt (999999)));
    addAndMakeVisible (seedEditor);
    diceButton.onClick = [this] { seedEditor.setText (juce::String (juce::Random::getSystemRandom().nextInt (999999))); };
    addAndMakeVisible (diceButton);

    prefixEditor.setTextToShowWhenEmpty ("(style name)", dim);
    addAndMakeVisible (prefixEditor);

    rateBox.addItem ("44.1 kHz", 44100);
    rateBox.addItem ("48 kHz", 48000);
    rateBox.setSelectedId (44100);
    bitsBox.addItem ("24-bit", 24);
    bitsBox.addItem ("16-bit", 16);
    bitsBox.setSelectedId (24);
    levelBox.addItem ("Level: loudness-matched", 1);
    levelBox.addItem ("Level: peak -1 dB", 2);
    levelBox.addItem ("Level: as played", 3);
    levelBox.setSelectedId (settings->getIntValue ("level", 1), juce::dontSendNotification);
    levelBox.setTooltip ("Loudness-matched: every file at the same perceived loudness, so pads play evenly. "
                         "Peak: every file peaks at -1 dB. As played: the instrument's own level.");
    levelBox.onChange = [this] { props.getUserSettings()->setValue ("level", levelBox.getSelectedId()); };

    registersBox.addItem ("Registers: off", 1);
    registersBox.addItem ("+ octave down", 2);
    registersBox.addItem ("+ octave up", 3);
    registersBox.addItem ("+ octave down & up", 4);
    registersBox.addItem ("+ 2 octaves down & up", 5);
    registersBox.setSelectedId (settings->getIntValue ("registers", 1), juce::dontSendNotification);
    registersBox.setTooltip ("Faster: leave this off and use \"Octave copies of keepers\" in Review. This renders every sample in other registers (Octave_Down / Octave_Up). "
                             "With 16 Levels > Tune, load the register nearest the notes you play: big re-pitches sound grainy or dull.");
    registersBox.onChange = [this] { props.getUserSettings()->setValue ("registers", registersBox.getSelectedId()); };

    shortNamesToggle.setToggleState (settings->getBoolValue ("shortNames", true), juce::dontSendNotification);
    shortNamesToggle.setTooltip ("014_EPBell_C3 instead of JetLifeEPBell_C3_014_Key_C3 - easier to read on the MPC screen");
    shortNamesToggle.onClick = [this] { props.getUserSettings()->setValue ("shortNames", shortNamesToggle.getToggleState()); };
    for (auto* c : std::initializer_list<juce::Component*> { &levelBox, &registersBox, &shortNamesToggle })
        addAndMakeVisible (c);

    varietyBox.addItem ("Variety: normal", 2);
    varietyBox.addItem ("Variety: strict", 3);
    varietyBox.addItem ("Variety: off", 1);
    varietyBox.setSelectedId (settings->getIntValue ("variety", 2), juce::dontSendNotification);
    varietyBox.setTooltip ("Normal: drop samples that sound nearly identical. Strict: drop anything too similar, even if the "
                           "batch ends up smaller. Scale banks are never thinned.");
    varietyBox.onChange = [this] { props.getUserSettings()->setValue ("variety", varietyBox.getSelectedId()); };

    for (int copies = 1; copies <= 4; ++copies)
        instancesBox.addItem ("VST copies: " + juce::String (copies), copies);
    instancesBox.setSelectedId (juce::jlimit (1, 4, settings->getIntValue ("instances", 1)), juce::dontSendNotification);
    instancesBox.setTooltip ("Load this many copies of your VST3 and render in parallel (uses more RAM - sample-heavy plugins "
                             "may not like it). The built-in synth always uses all CPU cores.");
    instancesBox.onChange = [this] { props.getUserSettings()->setValue ("instances", instancesBox.getSelectedId()); updateEstimate(); };

    sizeLabel.setText ("Batch", juce::dontSendNotification);
    sizeLabel.setColour (juce::Label::textColourId, dim);
    for (auto* b : { &size16, &size32, &size64 })
        b->onClick = [this, b] { countSlider.setValue (b->getButtonText().getIntValue()); };
    moreButton.setTooltip ("Render 16 more with a new seed and the same settings - quick rounds of fresh material");
    moreButton.onClick = [this]
    {
        if (job != nullptr) return;
        countSlider.setValue (16);
        seedEditor.setText (juce::String (juce::Random::getSystemRandom().nextInt (999999)));
        generateOrCancel();
    };
    estimateLabel.setColour (juce::Label::textColourId, dim);
    estimateLabel.setJustificationType (juce::Justification::centred);
    estimateLabel.setTooltip ("Estimated render time, learned from your previous batches with this instrument");
    for (auto* c : std::initializer_list<juce::Component*> { &varietyBox, &instancesBox, &sizeLabel, &size16, &size32, &size64,
                                                             &moreButton, &estimateLabel })
        addAndMakeVisible (c);

    // keep the estimate current
    for (auto* cb : { &styleBox, &registersBox, &bankBox, &pluginBox })
    {
        auto previous = cb->onChange;
        cb->onChange = [this, previous] { if (previous) previous(); updateEstimate(); };
    }
    countSlider.onValueChange = [this] { updateEstimate(); };
    smartPickToggle.onStateChange = [this] { updateEstimate(); };
    smartPickToggle.setToggleState (settings->getBoolValue ("smartPick", true), juce::dontSendNotification);
    smartPickToggle.setTooltip ("Renders 1.5x as many samples, scores them (pitch clarity, muddy voicings, harshness, clicks) "
                                "and keeps only the best, most varied ones.");
    smartPickToggle.onClick = [this] { props.getUserSettings()->setValue ("smartPick", smartPickToggle.getToggleState()); };
    for (auto* c : std::initializer_list<juce::Component*> { &rateBox, &bitsBox, &smartPickToggle })
        addAndMakeVisible (c);

    // ---------------------------------------------------------- one-shot / 16 Levels
    engineBox.addItem ("Style's engine", 1);
    {
        auto ids = engineIds();
        for (int i = 0; i < ids.size(); ++i)
        {
            Engine r;
            parseEngine (ids[i], r);
            engineBox.addItem (engineDisplayName (r) + " (" + ids[i] + ")", i + 2);
        }
        engineBox.addItem ("Mixed palette", 100);
    }
    engineBox.setSelectedId (1);
    engineBox.setTooltip ("Sound engine of the built-in synth. Ignored when a VST3 is loaded.");
    engineBox.onChange = [this] { applyLiveEngine(); lastPlan.clear(); };

    rootBox.addItem ("Off (style ranges)", 1);
    rootBox.addItem ("Recipe's own root", 2);
    for (int midi = 36; midi <= 84; midi += 12)
        rootBox.addItem (midiNoteName (midi), midi);
    rootBox.setSelectedId (1);

    lengthBox.addItem ("Recipe's end point", 1);
    for (auto ms : { 250, 500, 1000, 1500, 2000, 4000 })
        lengthBox.addItem ("Max " + (ms % 1000 == 0 ? juce::String (ms / 1000) : juce::String (ms / 1000.0, 2)) + " s", ms);
    lengthBox.addItem ("No limit", 2);
    lengthBox.setSelectedId (1);

    for (auto* c : std::initializer_list<juce::Component*> { &engineBox, &rootBox, &lengthBox, &banksToggle })
        addAndMakeVisible (c);

    outputFolder = juce::File (settings->getValue ("outputFolder",
        juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("MPC Sample Forge").getFullPathName()));
    outPath.setText (outputFolder.getFullPathName(), juce::dontSendNotification);
    outButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> ("Output folder", outputFolder);
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                              [this] (const juce::FileChooser& fc)
                              {
                                  if (fc.getResult() == juce::File()) return;
                                  outputFolder = fc.getResult();
                                  outPath.setText (outputFolder.getFullPathName(), juce::dontSendNotification);
                                  props.getUserSettings()->setValue ("outputFolder", outputFolder.getFullPathName());
                              });
    };
    addAndMakeVisible (outButton);

    planButton.onClick = [this] { previewPlan(); };
    auditionButton.onClick = [this]
    {
        if (lastPlan.empty()) previewPlan();
        if (! lastPlan.empty()) auditionSpec (juce::Random::getSystemRandom().nextInt ((int) lastPlan.size()));
    };
    auditionOctaveBox.addItem ("Oct: as planned", 1);
    auditionOctaveBox.addItem ("Oct: chord row's", 2);
    for (int oct = 0; oct <= 6; ++oct) auditionOctaveBox.addItem ("Oct " + juce::String (oct) + " (C" + juce::String (oct) + ")", oct + 10);
    auditionOctaveBox.setSelectedId (settings->getIntValue ("auditionOctave", 2), juce::dontSendNotification);
    if (auditionOctaveBox.getSelectedId() == 0) auditionOctaveBox.setSelectedId (2, juce::dontSendNotification);
    auditionOctaveBox.setTooltip ("Which octave Audition Random plays in: the lowest note of the sample lands in this octave "
                                  "(C3 = middle C on the MPC). \"Chord row's\" follows the octave selector by the chord buttons.");
    auditionOctaveBox.onChange = [this] { props.getUserSettings()->setValue ("auditionOctave", auditionOctaveBox.getSelectedId()); };
    addAndMakeVisible (auditionOctaveBox);
    generateButton.onClick = [this] { generateOrCancel(); };
    reviewButton.setTooltip ("Audition a batch, mark keepers (K) and rejects (X), export the keepers");
    reviewButton.onClick = [this] { if (lastBatchFolder.isDirectory()) openReview (lastBatchFolder); else chooseFolderToReview(); };
    addAndMakeVisible (reviewButton);
    generateButton.setColour (juce::TextButton::buttonColourId, accent);
    for (auto* b : { &planButton, &auditionButton, &generateButton })
        addAndMakeVisible (b);

    addAndMakeVisible (progressBar);

    logView.setMultiLine (true);
    logView.setReadOnly (true);
    logView.setScrollbarsShown (true);
    logView.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::plain));
    logView.setColour (juce::TextEditor::backgroundColourId, panel);
    addAndMakeVisible (logView);

    keyboard.setAvailableRange (21, 108);
    keyboardState.addListener (this);
    addAndMakeVisible (keyboard);

    // ---------------------------------------------------------- chord row
    chordRowLabel.setText ("Chords", juce::dontSendNotification);
    chordRowLabel.setColour (juce::Label::textColourId, dim);
    addAndMakeVisible (chordRowLabel);
    exportInfo.setFont (juce::FontOptions (13.0f));
    exportInfo.setTooltip ("Exactly what Export chord renders: the chord, its notes and the octave (C3 = middle C on the MPC). "
                           "The lowest note is also at the end of the file name.");
    addAndMakeVisible (exportInfo);
    for (int i = 0; i < 7; ++i)
    {
        auto* b = chordButtons.add (new juce::TextButton());
        b->setClickingTogglesState (false);
        b->onClick = [this, i] { playChord (i); };
        b->setTooltip ("Click to hear this chord through the current instrument (raw: the recipe's FX are added on export)");
        addAndMakeVisible (b);
    }
    for (int oct = 1; oct <= 5; ++oct) chordOctaveBox.addItem ("Oct " + juce::String (oct) + " (C" + juce::String (oct) + ")", oct);
    chordOctaveBox.setSelectedId (juce::jlimit (1, 5, settings->getIntValue ("chordOctave", 2)), juce::dontSendNotification);
    chordOctaveBox.setTooltip ("Which octave the chords are previewed and exported in (C3 = middle C on the MPC)");
    chordOctaveBox.onChange = [this] { props.getUserSettings()->setValue ("chordOctave", chordOctaveBox.getSelectedId()); updateChordRow(); };
    inversionBox.addItem ("Root position", 1);
    inversionBox.addItem ("1st inversion", 2);
    inversionBox.addItem ("2nd inversion", 3);
    inversionBox.addItem ("3rd inversion", 4);
    inversionBox.setSelectedId (1, juce::dontSendNotification);
    inversionBox.setTooltip ("Play / export the chord inverted: the lowest notes move up an octave");
    inversionBox.onChange = [this] { updateChordRow(); };
    exportChordButton.setTooltip ("Render the selected chord - shown in the EXPORT line above (this octave + inversion, with the recipe's processing) into 'Chord Picks'");
    exportChordButton.onClick = [this] { exportChords ({ selectedChord }); };
    exportRowButton.setTooltip ("Render all 7 chords of the key (this octave + inversion) into 'Chord Picks'");
    exportRowButton.onClick = [this] { exportChords ({ 0, 1, 2, 3, 4, 5, 6 }); };
    for (auto* c : std::initializer_list<juce::Component*> { &chordOctaveBox, &inversionBox, &exportChordButton, &exportRowButton })
        addAndMakeVisible (c);
    for (auto* cb : { &keyBox, &scaleBox })
    {
        auto previous = cb->onChange;
        cb->onChange = [this, previous] { if (previous) previous(); updateChordRow(); };
    }

    // ---------------------------------------------------------- audio
    deviceManager.initialiseWithDefaultDevices (0, 2);
    deviceManager.addAudioCallback (&player);
    for (auto& m : juce::MidiInput::getAvailableDevices())
    {
        deviceManager.setMidiInputDeviceEnabled (m.identifier, true);
        deviceManager.addMidiInputDeviceCallback (m.identifier, &player);
    }

    {
        // a saved style may have been deleted since: fall back to the first one
        auto saved = settings->getIntValue ("styleIndex", 0);
        styleBox.setSelectedItemIndex (juce::isPositiveAndBelow (saved, styles.size()) ? saved : 0, juce::sendNotificationSync);
    }

    auto lastPlugin = settings->getValue ("plugin");
    auto idx = installedPlugins.indexOf (lastPlugin);
    if (idx >= 0)                                  pluginBox.setSelectedId (firstInstalledId + idx, juce::dontSendNotification);
    else                                           pluginBox.setSelectedId (builtInId, juce::dontSendNotification);

    if (lastPlugin.isNotEmpty() && juce::File (lastPlugin).exists()) loadInstrument (lastPlugin);
    else                                                              useBuiltIn();

    log ("Pick a VST3 instrument, click 'Edit Sound' to dial in a patch, choose a style, then Generate.");
    log ("Play the on-screen keyboard (or a MIDI controller) to audition the sound.");
    log ("16 Levels kits: pick a recipe from the guide (Jet Life EP Bell ... Rhodes Bass) or a kit (West Coast, Memphis,");
    log ("Atlanta, Recipe Guide Palette), set '16 Levels root' to \"Recipe's own root\" and tick '16-pad banks'.");
    log ("Each batch includes _MPC_Setup.txt with the pad settings (mute group, tune, phrasing) for every recipe.");
    log ("Scale banks: pick a key + 'Scale banks' and every bank of 16 pads is one sound across the scale, in tune on every pad.");
    log ("Quick rounds: Batch 16, Generate, keep what you like in Review, then '+16 More'. Octave copies: make them for keepers in Review.");
    updateEstimate();
    updateChordRow();
    setSize (980, 800);
}

MainComponent::~MainComponent()
{
    styleEditorWindow.reset();
    reviewWindow.reset();
    diagnosticsWindow.reset(); // stops a running self-test and hands the instrument back
    if (job) job->stopThread (10000);
    job.reset();
    keyboardState.removeListener (this);
    closeEditor();
    for (auto& m : juce::MidiInput::getAvailableDevices())
        deviceManager.removeMidiInputDeviceCallback (m.identifier, &player);
    deviceManager.removeAudioCallback (&player);
    player.setProcessor (nullptr);
    instrument.reset();
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (bg);
    g.setColour (panel);
    g.fillRoundedRectangle (getLocalBounds().reduced (10).withTrimmedTop (44).withHeight (366).toFloat(), 6.0f);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced (20, 12);
    auto titleRow = area.removeFromTop (34);
    diagnosticsButton.setBounds (titleRow.removeFromRight (120).reduced (0, 3));
    title.setBounds (titleRow);
    area.removeFromTop (12);

    auto row = [&area] (int h = 28) { auto r = area.removeFromTop (h); area.removeFromTop (8); return r; };
    const int labelW = 100;

    auto r = row();
    pluginLabel.setBounds (r.removeFromLeft (labelW));
    loadSoundButton.setBounds (r.removeFromRight (100)); r.removeFromRight (6);
    saveSoundButton.setBounds (r.removeFromRight (100)); r.removeFromRight (6);
    editButton.setBounds (r.removeFromRight (100)); r.removeFromRight (6);
    browseButton.setBounds (r.removeFromRight (90)); r.removeFromRight (6);
    pluginBox.setBounds (r);

    r = row();
    styleLabel.setBounds (r.removeFromLeft (labelW));
    styleFileButton.setBounds (r.removeFromRight (100)); r.removeFromRight (6);
    editStyleButton.setBounds (r.removeFromRight (100)); r.removeFromRight (6);
    styleBox.setBounds (r);

    r = row (40);
    r.removeFromLeft (labelW);
    if (progressionBox.isVisible()) { progressionBox.setBounds (r.removeFromRight (340).withSizeKeepingCentre (340, 28)); r.removeFromRight (8); }
    styleDesc.setBounds (r);

    r = row();
    keyLabel.setBounds (r.removeFromLeft (labelW));
    keyBox.setBounds (r.removeFromLeft (150)); r.removeFromLeft (6);
    scaleBox.setBounds (r.removeFromLeft (170)); r.removeFromLeft (20);
    countLabel.setBounds (r.removeFromLeft (70));
    countSlider.setBounds (r.removeFromLeft (150)); r.removeFromLeft (12);
    bankBox.setBounds (r.removeFromLeft (220));

    r = row();
    seedLabel.setBounds (r.removeFromLeft (labelW));
    seedEditor.setBounds (r.removeFromLeft (110)); r.removeFromLeft (6);
    diceButton.setBounds (r.removeFromLeft (60)); r.removeFromLeft (20);
    prefixLabel.setBounds (r.removeFromLeft (60));
    prefixEditor.setBounds (r.removeFromLeft (200)); r.removeFromLeft (20);
    sizeLabel.setBounds (r.removeFromLeft (44));
    size16.setBounds (r.removeFromLeft (40)); r.removeFromLeft (4);
    size32.setBounds (r.removeFromLeft (40)); r.removeFromLeft (4);
    size64.setBounds (r.removeFromLeft (40));

    r = row();
    audioLabel.setBounds (r.removeFromLeft (labelW));
    rateBox.setBounds (r.removeFromLeft (100)); r.removeFromLeft (6);
    bitsBox.setBounds (r.removeFromLeft (90)); r.removeFromLeft (8);
    levelBox.setBounds (r.removeFromLeft (200)); r.removeFromLeft (8);
    smartPickToggle.setBounds (r.removeFromLeft (110)); r.removeFromLeft (4);
    varietyBox.setBounds (r.removeFromLeft (150)); r.removeFromLeft (8);
    instancesBox.setBounds (r.removeFromLeft (140));

    r = row();
    engineLabel.setBounds (r.removeFromLeft (labelW));
    engineBox.setBounds (r.removeFromLeft (180)); r.removeFromLeft (20);
    rootLabel.setBounds (r.removeFromLeft (110));
    rootBox.setBounds (r.removeFromLeft (150)); r.removeFromLeft (6);
    lengthBox.setBounds (r.removeFromLeft (130)); r.removeFromLeft (12);
    banksToggle.setBounds (r.removeFromLeft (140));

    r = row();
    outLabel.setBounds (r.removeFromLeft (labelW));
    outButton.setBounds (r.removeFromRight (100)); r.removeFromRight (6);
    shortNamesToggle.setBounds (r.removeFromRight (120)); r.removeFromRight (6);
    registersBox.setBounds (r.removeFromRight (200)); r.removeFromRight (10);
    outPath.setBounds (r);

    area.removeFromTop (14);
    r = row (34);
    generateButton.setBounds (r.removeFromRight (130)); r.removeFromRight (6);
    moreButton.setBounds (r.removeFromRight (84)); r.removeFromRight (6);
    reviewButton.setBounds (r.removeFromRight (84)); r.removeFromRight (6);
    auditionOctaveBox.setBounds (r.removeFromRight (140)); r.removeFromRight (4);
    auditionButton.setBounds (r.removeFromRight (124)); r.removeFromRight (6);
    planButton.setBounds (r.removeFromRight (100)); r.removeFromRight (6);
    estimateLabel.setBounds (r.removeFromRight (90)); r.removeFromRight (6);
    progressBar.setBounds (r);

    keyboard.setBounds (area.removeFromBottom (80));
    keyboard.setKeyWidth ((float) keyboard.getWidth() / 52.0f); // 88 keys = 52 white keys
    keyboard.setLowestVisibleKey (21);
    area.removeFromBottom (6);
    auto chordRow = area.removeFromBottom (30);
    area.removeFromBottom (2);
    exportInfo.setBounds (area.removeFromBottom (18).withTrimmedLeft (52));
    chordRowLabel.setBounds (chordRow.removeFromLeft (52));
    exportRowButton.setBounds (chordRow.removeFromRight (100)); chordRow.removeFromRight (4);
    exportChordButton.setBounds (chordRow.removeFromRight (100)); chordRow.removeFromRight (6);
    inversionBox.setBounds (chordRow.removeFromRight (120)); chordRow.removeFromRight (4);
    chordOctaveBox.setBounds (chordRow.removeFromRight (110)); chordRow.removeFromRight (6);
    auto bw = chordRow.getWidth() / 7;
    for (int i = 0; i < chordButtons.size(); ++i) chordButtons[i]->setBounds (chordRow.removeFromLeft (bw).reduced (2, 0));
    area.removeFromBottom (6);
    logView.setBounds (area);
}

void MainComponent::log (const juce::String& s)
{
    Log::write (s);
    logView.moveCaretToEnd();
    logView.insertTextAtCaret (s + "\n");
}

//==============================================================================
void MainComponent::choosePlugin (int id)
{
    if (id == builtInId) useBuiltIn();
    else if (id >= firstInstalledId) loadInstrument (installedPlugins[id - firstInstalledId]);
}

void MainComponent::browseForPlugin()
{
    chooser = std::make_unique<juce::FileChooser> ("Choose a VST3 instrument", juce::File(), "*.vst3");
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::canSelectDirectories,
                          [this] (const juce::FileChooser& fc)
                          {
                              auto f = fc.getResult();
                              if (f != juce::File()) loadInstrument (f.getFullPathName());
                          });
}

void MainComponent::setInstrument (std::unique_ptr<juce::AudioProcessor> p, const juce::String& path)
{
    closeEditor();
    player.setProcessor (nullptr);
    instrument = std::move (p);
    instrumentPath = path;
    lastPlan.clear();

    if (instrument != nullptr)
    {
        // make sure it has a stereo out for the audition device too
        auto layout = instrument->getBusesLayout();
        if (layout.outputBuses.size() > 0)
        {
            layout.outputBuses.getReference (0) = juce::AudioChannelSet::stereo();
            if (instrument->checkBusesLayoutSupported (layout)) instrument->setBusesLayout (layout);
        }
        player.setProcessor (instrument.get());
        editButton.setEnabled (instrument->hasEditor());
        log ("Loaded: " + instrument->getName());
    }

    props.getUserSettings()->setValue ("plugin", path);
}

void MainComponent::useBuiltIn()
{
    setInstrument (std::make_unique<BuiltInSynth>(), {});
    pluginBox.setSelectedId (builtInId, juce::dontSendNotification);
    applyLiveEngine();
}

void MainComponent::loadInstrument (const juce::String& path)
{
    juce::String error;
    double sr = 44100;
    int bs = 512;
    if (auto* dev = deviceManager.getCurrentAudioDevice())
    {
        sr = dev->getCurrentSampleRate();
        bs = dev->getCurrentBufferSizeSamples();
    }

    log ("Loading " + path + " ...");
    auto plugin = host.load (path, sr, bs, error);

    if (plugin == nullptr)
    {
        log ("ERROR: " + error);
        juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, "Could not load plugin", error);
        return;
    }

    setInstrument (std::move (plugin), path);

    auto idx = installedPlugins.indexOf (path);
    if (idx >= 0) pluginBox.setSelectedId (firstInstalledId + idx, juce::dontSendNotification);
    else          pluginBox.setText (juce::File (path).getFileNameWithoutExtension(), juce::dontSendNotification);
}

void MainComponent::openEditor()
{
    if (instrument == nullptr || ! instrument->hasEditor()) return;
    if (editorWindow != nullptr) { editorWindow->toFront (true); return; }

    if (auto* ed = instrument->createEditorIfNeeded())
        editorWindow = std::make_unique<EditorWindow> (ed, instrument->getName(),
                                                       [safe = juce::Component::SafePointer<MainComponent> (this)]
                                                       { if (safe) safe->closeEditor(); });
}

void MainComponent::closeEditor()
{
    editorWindow.reset(); // deletes the editor before the plugin goes away
}

void MainComponent::saveSound()
{
    if (instrument == nullptr) return;
    chooser = std::make_unique<juce::FileChooser> ("Save sound", outputFolder.getChildFile (safeFileName (instrument->getName()) + ".forgesound"),
                                                   "*.forgesound");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
                          {
                              auto f = fc.getResult();
                              if (f == juce::File() || instrument == nullptr) return;
                              f = f.withFileExtension (".forgesound");
                              if (captureState (*instrument, instrumentPath).save (f))
                                  log ("Saved sound to " + f.getFullPathName() + "  (use with --sound for automation)");
                              else
                                  log ("ERROR: could not save " + f.getFullPathName());
                          });
}

void MainComponent::loadSound()
{
    chooser = std::make_unique<juce::FileChooser> ("Load sound", outputFolder, "*.forgesound");
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this] (const juce::FileChooser& fc)
                          {
                              auto f = fc.getResult();
                              if (f == juce::File()) return;
                              SoundState s;
                              juce::String err;
                              if (! s.load (f, err)) { log ("ERROR: " + err); return; }

                              if (s.pluginPath.isEmpty()) useBuiltIn();
                              else if (s.pluginPath != instrumentPath) loadInstrument (s.pluginPath);

                              if (instrument != nullptr && s.state.getSize() > 0)
                              {
                                  instrument->setStateInformation (s.state.getData(), (int) s.state.getSize());
                                  log ("Restored sound for " + s.pluginName);
                              }
                          });
}

//==============================================================================
void MainComponent::styleChanged()
{
    auto i = styleBox.getSelectedItemIndex();
    if (! juce::isPositiveAndBelow (i, styles.size())) return;
    const bool library = styles[i].progressionLibrary.isNotEmpty();
    styleDesc.setText (styles[i].description + (library ? juce::String() : "   (scale: " + styles[i].scale + ")"), juce::dontSendNotification);
    if (library)
        if (auto* set = selectedProgressionSet())
            styleDesc.setText ("#" + juce::String (set->number) + " " + set->keyName + ": " + set->main.text + " / "
                               + set->alt1.text + " / " + set->alt2.text + ".  " + set->tip, juce::dontSendNotification);
    if (progressionBox.isVisible() != library) { progressionBox.setVisible (library); resized(); }
    props.getUserSettings()->setValue ("styleIndex", i);
    lastPlan.clear();
    applyLiveEngine();
    updateChordRow();
}

void MainComponent::loadStyleJson()
{
    chooser = std::make_unique<juce::FileChooser> ("Load style", juce::File(), "*.json");
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this] (const juce::FileChooser& fc)
                          {
                              auto f = fc.getResult();
                              if (f == juce::File()) return;
                              Style s;
                              juce::String err;
                              if (! loadStyleFile (f, s, err)) { log ("ERROR: " + err); return; }
                              styles.add (s);
                              styleBox.addItem (s.name + " (custom)", styles.size());
                              styleBox.setSelectedId (styles.size());
                              log ("Loaded style '" + s.name + "' from " + f.getFileName());
                          });
}

bool MainComponent::buildRequest (BatchRequest& req, std::vector<SampleSpec>& specs)
{
    auto i = styleBox.getSelectedItemIndex();
    if (! juce::isPositiveAndBelow (i, styles.size()) || instrument == nullptr) return false;

    req.style = styles[i];
    req.plan.count = (int) countSlider.getValue();
    req.plan.key = keyBox.getSelectedId() <= 1 ? juce::String ("random") : keyBox.getText();
    req.plan.scale = scaleBox.getSelectedId() <= 1 ? juce::String() : scaleBox.getText();
    req.plan.seed = seedEditor.getText().getLargeIntValue();
    req.plan.prefix = prefixEditor.getText().trim();
    req.render.sampleRate = rateBox.getSelectedId();
    req.render.bitDepth = bitsBox.getSelectedId();
    req.render.normalise = levelBox.getSelectedId() != 3;
    req.render.loudnessMatch = levelBox.getSelectedId() == 1;
    req.plan.shortNames = shortNamesToggle.getToggleState();
    req.plan.progressionNumber = req.style.progressionLibrary.isNotEmpty() ? progressionBox.getSelectedId() - 1 : 0;
    req.variety = varietyBox.getSelectedId() - 1;
    req.plan.scaleBank = bankBox.getSelectedId() > 1;
    req.plan.bankType = bankBox.getSelectedId() == 3 ? "notes" : bankBox.getSelectedId() == 4 ? "chords"
                      : bankBox.getSelectedId() == 5 ? "progressions" : "";
    switch (registersBox.getSelectedId())
    {
        case 2: req.registers = { -12 }; break;
        case 3: req.registers = { 12 }; break;
        case 4: req.registers = { -12, 12 }; break;
        case 5: req.registers = { -24, -12, 12, 24 }; break;
        default: break;
    }
    req.instrumentName = instrument->getName();
    req.plan.engine = selectedEngine();
    req.plan.builtIn = dynamic_cast<BuiltInSynth*> (instrument.get()) != nullptr;
    auto rootId = rootBox.getSelectedId();
    req.plan.fixedRoot = rootId == 1 ? -1 : rootId == 2 ? PlanOptions::styleRoot : rootId;
    req.splitBanks = banksToggle.getToggleState();
    auto lengthId = lengthBox.getSelectedId();
    req.render.trimToSec = lengthId == 1 ? -1.0 : lengthId == 2 ? 0.0 : lengthId / 1000.0;

    auto sub = safeFileName (req.plan.prefix.isNotEmpty() ? req.plan.prefix : req.style.name);
    if (req.plan.fixedRoot >= 0) sub += "_" + midiNoteName (req.plan.fixedRoot);
    else if (req.plan.fixedRoot == PlanOptions::styleRoot) sub += "_16Levels";
    else if (req.plan.key != "random") sub += "_" + req.plan.key;
    req.outputFolder = outputFolder.getChildFile (sub).getNonexistentSibling (false);

    juce::String error;
    const int wanted = req.plan.count;
    const bool banks = req.plan.scaleBank || req.style.scaleBankOnly;
    if (banks)
    {
        req.splitBanks = true; // one sound per 16-pad bank
        if (smartPickToggle.getToggleState() || ! req.registers.empty())
            log ("Scale banks: Smart Pick and Registers are skipped - each bank is a complete 16-pad set across the scale.");
        req.registers.clear();
    }
    if (! banks && smartPickToggle.getToggleState())
    {
        req.plan.count = juce::jmin (2000, (int) std::ceil (wanted * 1.5));
        req.smartPickTarget = wanted;
    }
    specs = planSamples (req.style, req.plan, error);
    if (req.smartPickTarget > 0)
    {
        if ((int) specs.size() <= wanted)
        {
            log ("Smart Pick skipped: only " + juce::String ((int) specs.size()) + " distinct samples are possible with these settings.");
            req.smartPickTarget = 0;
        }
        error = {};
    }
    req.plan.count = wanted;
    if (error.isNotEmpty()) log ("WARNING: " + error);
    return ! specs.empty();
}

void MainComponent::previewPlan()
{
    BatchRequest req;
    if (! buildRequest (req, lastPlan)) return;
    log ("---- " + juce::String ((int) lastPlan.size()) + (req.smartPickTarget > 0 ? " candidates planned (Smart Pick keeps the best "
                                                                                      + juce::String (req.smartPickTarget) + ")" : juce::String (" samples planned"))
         + " (" + req.style.name + ", seed " + juce::String (req.plan.seed) + ") ----");
    for (auto& s : lastPlan) log (describeSpec (s));
}

void MainComponent::auditionSpec (int index)
{
    if (! juce::isPositiveAndBelow (index, (int) lastPlan.size()) || job != nullptr) return;
    auto spec = lastPlan[(size_t) index];

    // play it in the chosen octave: the lowest note lands in that octave (60 = C3)
    auto octaveId = auditionOctaveBox.getSelectedId();
    int octave = octaveId == 2 ? chordOctaveBox.getSelectedId() : octaveId >= 10 ? octaveId - 10 : -99;
    if (octave > -99 && ! spec.notes.empty())
    {
        int lowest = 127;
        for (auto& n : spec.notes) lowest = std::min (lowest, n.note);
        const int base = 12 * (octave + 2);
        const int shift = -12 * (int) std::floor ((lowest - base) / 12.0);
        SampleSpec moved;
        if (shift != 0 && transposeSpec (spec, shift, moved)) spec = moved;
    }
    log ("> " + describeSpec (spec));

    if (auto* b = dynamic_cast<BuiltInSynth*> (instrument.get()))
    {
        Engine r;
        if (parseEngine (spec.engine, r)) b->setEngine (r);
        b->setVariation (spec.variation);
    }

    auto now = juce::Time::getMillisecondCounterHiRes() * 0.001;
    auto& collector = player.getMidiMessageCollector();
    for (auto& n : spec.notes)
    {
        auto on = juce::MidiMessage::noteOn (1, n.note, (juce::uint8) n.velocity);
        on.setTimeStamp (now);
        collector.addMessageToQueue (on);
        // schedule note-offs on the message thread
        juce::Timer::callAfterDelay ((int) ((n.startSec + n.lengthSec) * 1000.0),
                                     [safe = juce::Component::SafePointer<MainComponent> (this), note = n.note]
                                     {
                                         if (safe == nullptr) return;
                                         auto off = juce::MidiMessage::noteOff (1, note);
                                         off.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001);
                                         safe->player.getMidiMessageCollector().addMessageToQueue (off);
                                     });
    }
}

void MainComponent::setBusy (bool busy)
{
    for (auto* c : std::initializer_list<juce::Component*> { &pluginBox, &browseButton, &loadSoundButton, &styleBox,
                                                             &styleFileButton, &keyBox, &scaleBox, &countSlider, &seedEditor,
                                                             &diceButton, &prefixEditor, &rateBox, &bitsBox, &levelBox, &registersBox, &shortNamesToggle,
                                                             &engineBox, &rootBox, &lengthBox, &banksToggle, &smartPickToggle, &reviewButton, &bankBox,
                                                             &exportChordButton, &exportRowButton, &editStyleButton,
                                                             &varietyBox, &instancesBox, &size16, &size32, &size64, &moreButton,
                                                             &outButton, &planButton, &auditionButton, &auditionOctaveBox, &keyboard })
        c->setEnabled (! busy);
    generateButton.setButtonText (busy ? "Cancel" : "Generate");
}

void MainComponent::generateOrCancel()
{
    if (job != nullptr)
    {
        job->signalThreadShouldExit();
        log ("Cancelling ...");
        return;
    }

    BatchRequest req;
    std::vector<SampleSpec> specs;
    if (! buildRequest (req, specs)) return;

    startJob (std::move (req), std::move (specs), true);
}

void MainComponent::startJob (BatchRequest req, std::vector<SampleSpec> specs, bool openReviewAfter)
{
    keyboardState.allNotesOff (0);
    player.setProcessor (nullptr); // the job owns the instrument while it renders

    // parallel VST3 copies (the built-in synth makes its own)
    extraInstances.clear();
    if (dynamic_cast<BuiltInSynth*> (instrument.get()) == nullptr && instrumentPath.isNotEmpty() && instancesBox.getSelectedId() > 1)
    {
        juce::MemoryBlock state;
        instrument->getStateInformation (state);
        for (int k = 1; k < instancesBox.getSelectedId(); ++k)
        {
            juce::String err;
            auto copy = host.load (instrumentPath, req.render.sampleRate, req.render.blockSize, err);
            if (copy == nullptr) { log ("Parallel copy " + juce::String (k + 1) + " failed (" + err + ") - continuing with fewer."); break; }
            copy->setStateInformation (state.getData(), (int) state.getSize());
            extraInstances.push_back (std::move (copy));
        }
        for (auto& e : extraInstances) req.extraInstances.push_back (e.get());
        if (! extraInstances.empty()) log ("Rendering with " + juce::String ((int) extraInstances.size() + 1) + " copies of " + instrument->getName());
    }

    setBusy (true);
    progress = 0;
    log ("==== " + juce::String (req.registersOnly ? "Octave copies of " : "Generating ") + juce::String ((int) specs.size())
         + " samples into " + req.outputFolder.getFullPathName());

    auto safe = juce::Component::SafePointer<MainComponent> (this);
    job = std::make_unique<BatchJob> (*instrument, req, std::move (specs));
    job->onProgress = [safe] (int done, int total, const juce::String& msg)
    {
        if (safe == nullptr) return;
        safe->progress = total > 0 ? (double) done / total : 0.0;
        safe->log (msg);
    };
    job->onFinished = [safe, folder = req.outputFolder, openReviewAfter, name = req.instrumentName] (bool ok, const juce::String& summary)
    {
        if (safe == nullptr) return;
        // learn this instrument's speed for the next estimate
        if (safe->job != nullptr && safe->job->elapsedSeconds() > 0.3 && safe->job->renderedAudioSeconds() > 1.0)
            safe->props.getUserSettings()->setValue ("speed_" + name, safe->job->renderedAudioSeconds() / safe->job->elapsedSeconds());
        safe->extraInstances.clear();
        safe->jobFinished (ok, summary);
        if (folder.isDirectory())
        {
            safe->lastBatchFolder = folder;
            if (openReviewAfter) safe->openReview (folder); // straight into auditioning and picking keepers
            else if (safe->reviewWindow != nullptr)
                if (auto* panel = dynamic_cast<ReviewPanel*> (safe->reviewWindow->getContentComponent())) panel->reload();
        }
        safe->updateEstimate();
    };
    job->startThread();
}

double MainComponent::expectedSpeed() const
{
    // x real time, measured on earlier batches; first guesses until then
    auto builtIn = dynamic_cast<BuiltInSynth*> (instrument.get()) != nullptr;
    auto name = instrument != nullptr ? instrument->getName() : juce::String();
    auto fallback = builtIn ? 60.0 * juce::jmin (8, juce::SystemStats::getNumCpus()) : 8.0 * instancesBox.getSelectedId();
    return juce::jmax (0.5, (double) const_cast<juce::ApplicationProperties&> (props).getUserSettings()->getDoubleValue ("speed_" + name, fallback));
}

void MainComponent::updateEstimate()
{
    auto i = styleBox.getSelectedItemIndex();
    if (! juce::isPositiveAndBelow (i, styles.size()) || instrument == nullptr) { estimateLabel.setText ({}, juce::dontSendNotification); return; }
    const auto st = styles[i];
    auto banks = bankBox.getSelectedId() > 1 || st.scaleBankOnly;
    auto count = (double) countSlider.getValue();
    if (banks) count = std::ceil (count / 16.0) * 16.0;

    // seconds of audio per sample: the recipe's end point, else the hold plus a short tail
    auto perSample = st.processing.maxLength > 0 ? st.processing.maxLength + st.processing.fxTailMs / 1000.0
                                                 : (st.holdLow + st.holdHigh) * 0.5 + 0.8;
    perSample += 0.1; // silence between samples
    auto factor = (! banks && smartPickToggle.getToggleState()) ? 1.5 : 1.0;
    auto regs = banks ? 0 : (registersBox.getSelectedId() == 1 ? 0 : registersBox.getSelectedId() == 5 ? 4 : registersBox.getSelectedId() == 4 ? 2 : 1);
    auto seconds = count * perSample * (factor + regs) / expectedSpeed();
    if (dynamic_cast<BuiltInSynth*> (instrument.get()) == nullptr) seconds += 1.5; // plugin warm-up

    estimateLabel.setText (seconds < 60 ? "~" + juce::String (juce::jmax (1, juce::roundToInt (seconds))) + " s"
                                        : "~" + juce::String (seconds / 60.0, 1) + " min", juce::dontSendNotification);
}

void MainComponent::makeRegistersForKeepers (const juce::File& batchFolder, const juce::Array<juce::File>& keepers)
{
    if (job != nullptr) { log ("Busy - wait for the current job to finish."); return; }

    auto batch = juce::JSON::parse (batchFolder.getChildFile ("_batch.json"));
    auto specsJson = juce::JSON::parse (batchFolder.getChildFile ("_specs.json"));
    if (! specsJson.isArray())
    {
        juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, "Can't make octave copies",
            "This batch was made by an older version (no _specs.json). Generate it again to use this.");
        return;
    }
    auto madeWith = batch["instrument"].toString();
    if (instrument == nullptr || (madeWith.isNotEmpty() && madeWith != instrument->getName()))
    {
        juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, "Load the same instrument first",
            "These samples were made with \"" + madeWith + "\". Load it (and its sound) so the octave copies match, then try again.");
        return;
    }

    BatchRequest req;
    req.registersOnly = true;
    req.outputFolder = batchFolder;
    req.instrumentName = instrument->getName();
    req.render.sampleRate = (double) batch.getProperty ("sampleRate", 44100.0);
    req.render.bitDepth = (int) batch.getProperty ("bitDepth", 24);
    req.render.loudnessMatch = (bool) batch.getProperty ("loudnessMatch", false);
    req.registers = { -12, 12 };
    switch (registersBox.getSelectedId())
    {
        case 2: req.registers = { -12 }; break;
        case 3: req.registers = { 12 }; break;
        case 5: req.registers = { -24, -12, 12, 24 }; break;
        default: break;
    }

    std::vector<SampleSpec> specs;
    for (auto& keeper : keepers)
    {
        auto rel = keeper.getRelativePathFrom (batchFolder).replaceCharacter ('\\', '/');
        for (auto& v : *specsJson.getArray())
        {
            SampleSpec sp;
            if ((int) v.getProperty ("register", 0) == 0 && v["file"].toString() == rel && specFromVar (v, sp))
            {
                specs.push_back (sp);
                req.existingFiles.push_back (keeper);
                break;
            }
        }
    }
    if (specs.empty()) { log ("None of the keepers were found in _specs.json."); return; }
    startJob (std::move (req), std::move (specs), false);
}

void MainComponent::jobFinished (bool ok, const juce::String& summary)
{
    job.reset();
    progress = ok ? 1.0 : progress;
    log (summary);
    setBusy (false);
    if (instrument != nullptr) player.setProcessor (instrument.get());
    applyLiveEngine();
    seedEditor.setText (juce::String (juce::Random::getSystemRandom().nextInt (999999))); // next batch is different
}

void MainComponent::openDiagnostics()
{
    if (diagnosticsWindow != nullptr) { diagnosticsWindow->toFront (true); return; }

    DiagnosticsPanel::Hooks hooks;
    auto safe = juce::Component::SafePointer<MainComponent> (this);
    hooks.report = [safe]
    {
        return safe == nullptr ? juce::String()
                               : systemReport (safe->host, &safe->deviceManager, safe->instrument.get(), safe->instrumentPath);
    };
    hooks.acquire = [safe]() -> juce::AudioProcessor*
    {
        if (safe == nullptr || safe->job != nullptr) return nullptr; // never test while a batch renders
        safe->keyboardState.allNotesOff (0);
        safe->player.setProcessor (nullptr);
        safe->setBusy (true);
        safe->generateButton.setEnabled (false);
        return safe->instrument.get();
    };
    hooks.release = [safe]
    {
        if (safe == nullptr) return;
        safe->setBusy (false);
        safe->generateButton.setEnabled (true);
        if (safe->instrument != nullptr) safe->player.setProcessor (safe->instrument.get());
        safe->applyLiveEngine();
    };

    diagnosticsWindow = std::make_unique<ToolWindow> ("MPC Sample Forge - Diagnostics", new DiagnosticsPanel (hooks),
                                                      [safe] { if (safe != nullptr) safe->diagnosticsWindow.reset(); });
}

void MainComponent::openReview (const juce::File& batchFolder)
{
    if (! batchFolder.isDirectory()) return;
    reviewWindow.reset(); // one review at a time
    auto safe = juce::Component::SafePointer<MainComponent> (this);
    auto* panel = new ReviewPanel (batchFolder, deviceManager);
    panel->onMakeRegisters = [safe] (const juce::File& f, const juce::Array<juce::File>& keepers)
    {
        if (safe != nullptr) safe->makeRegistersForKeepers (f, keepers);
    };
    panel->onMoreLikeThis = [safe] (const juce::File& f, const juce::File& sample)
    {
        if (safe != nullptr) safe->moreLikeThis (f, sample);
    };
    reviewWindow = std::make_unique<ToolWindow> ("Review & Keep - " + batchFolder.getFileName(), panel,
                                                 [safe] { if (safe != nullptr) safe->reviewWindow.reset(); });
    reviewWindow->toFront (true);
}

void MainComponent::chooseFolderToReview()
{
    auto start = lastBatchFolder.isDirectory() ? lastBatchFolder : outputFolder;
    chooser = std::make_unique<juce::FileChooser> ("Choose a batch folder to review", start);
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                          [this] (const juce::FileChooser& fc)
                          {
                              auto f = fc.getResult();
                              if (f.isDirectory()) openReview (f);
                          });
}

juce::String MainComponent::selectedEngine() const
{
    auto id = engineBox.getSelectedId();
    if (id == 100) return "mixed";
    if (id >= 2) return engineIds()[id - 2];
    return {};
}

void MainComponent::applyLiveEngine()
{
    // What you play on the keyboard should sound like what will be rendered.
    auto* b = dynamic_cast<BuiltInSynth*> (instrument.get());
    if (b == nullptr) return;

    auto name = selectedEngine();
    auto i = styleBox.getSelectedItemIndex();
    if (name.isEmpty() && juce::isPositiveAndBelow (i, styles.size())) name = styles[i].engine;

    Engine r = Engine::EPiano;
    if (name == "mixed") r = Engine::Bell;
    else parseEngine (name, r);
    b->setEngine (r);
}


//==============================================================================
int MainComponent::currentTonic() const
{
    return keyBox.getSelectedId() <= 1 ? 0 : keyBox.getSelectedId() - 2; // "Random keys" previews in C
}

std::vector<int> MainComponent::currentScale() const
{
    std::vector<int> scale;
    auto i = styleBox.getSelectedItemIndex();
    auto name = scaleBox.getSelectedId() > 1 ? scaleBox.getText()
              : juce::isPositiveAndBelow (i, styles.size()) ? styles[i].scale : juce::String ("minor");
    if (! getScale (name, scale)) getScale ("minor", scale);
    return scale;
}

const ProgressionSet* MainComponent::selectedProgressionSet (bool orFirst) const
{
    auto i = styleBox.getSelectedItemIndex();
    if (! juce::isPositiveAndBelow (i, styles.size()) || styles[i].progressionLibrary.isEmpty()) return nullptr;
    const auto& lib = progressionLibrary (styles[i].progressionLibrary);
    auto n = progressionBox.getSelectedId() - 1;
    if (n < 1) n = orFirst ? 1 : 0;
    return juce::isPositiveAndNotGreaterThan (n, (int) lib.size()) && n > 0 ? &lib[(size_t) n - 1] : nullptr;
}

void MainComponent::updateChordRow()
{
    struct ShowInfo { MainComponent& m; ~ShowInfo() { m.updateExportInfo(); } } showInfo { *this };
    auto i = styleBox.getSelectedItemIndex();
    if (! juce::isPositiveAndBelow (i, styles.size())) return;
    const auto st = styles[i];
    if (st.isKit()) { currentChords.clear(); for (auto* b : chordButtons) { b->setButtonText ("-"); b->setEnabled (false); } return; }

    if (auto* set = st.progressionLibrary.isNotEmpty() ? selectedProgressionSet (true) : nullptr)
    {
        // the chosen set's chords (main, then the alternates' new ones), in the chosen key
        const int tonic = keyBox.getSelectedId() <= 1 ? set->tonicPc : keyBox.getSelectedId() - 2;
        const int shift = tonic - set->tonicPc;
        currentChords.clear();
        for (auto* p : { &set->main, &set->alt1, &set->alt2 })
            for (auto& named : p->chords)
            {
                NamedChord c = named;
                c.rootPc = ((c.rootPc + shift) % 12 + 12) % 12;
                auto name = pitchClassName (c.rootPc) + (c.quality == "min" ? "m" : c.quality == "5" ? "5" : "");
                if (currentChords.size() >= 7 || std::any_of (currentChords.begin(), currentChords.end(), [&] (auto& d) { return d.name == name; }))
                    continue;
                DiatonicChord d;
                d.degree = (int) currentChords.size();
                d.rootPc = c.rootPc;
                d.roman = romanNumeralFor (c, tonic);
                d.name = name;
                if (auto* q = findQuality (c.quality)) d.intervals = q->intervals;
                currentChords.push_back (d);
            }
        for (int b = 0; b < chordButtons.size(); ++b)
        {
            auto* button = chordButtons[b];
            auto has = b < (int) currentChords.size();
            button->setEnabled (has);
            button->setButtonText (has ? currentChords[(size_t) b].roman + " " + currentChords[(size_t) b].name : juce::String ("-"));
            button->setColour (juce::TextButton::buttonColourId, b == selectedChord ? accent.withAlpha (0.6f) : panel.brighter (0.1f));
        }
        chordRowLabel.setText ("#" + juce::String (set->number), juce::dontSendNotification);
        chordRowLabel.setTooltip ("Chords of progression set #" + juce::String (set->number) + " in " + pitchClassName (tonic)
                                  + (set->minor ? " minor" : " major"));
        return;
    }

    currentChords = diatonicChords (st, currentTonic(), currentScale());
    for (int b = 0; b < chordButtons.size(); ++b)
    {
        auto* button = chordButtons[b];
        auto has = b < (int) currentChords.size();
        button->setEnabled (has);
        button->setButtonText (has ? currentChords[(size_t) b].roman + " " + currentChords[(size_t) b].name : juce::String ("-"));
        button->setColour (juce::TextButton::buttonColourId, b == selectedChord ? accent.withAlpha (0.6f) : panel.brighter (0.1f));
    }
    chordRowLabel.setText (pitchClassName (currentTonic()), juce::dontSendNotification);
    chordRowLabel.setTooltip ("Chords of " + pitchClassName (currentTonic()) + " " + (scaleBox.getSelectedId() > 1 ? scaleBox.getText() : st.scale)
                              + (keyBox.getSelectedId() <= 1 ? " (pick a Key to change it)" : ""));
}

void MainComponent::updateExportInfo()
{
    auto spec = chordSpec (selectedChord);
    if (spec.notes.empty()) { exportInfo.setText ({}, juce::dontSendNotification); return; }
    const int ps = spec.processing.pitchShift;
    juce::StringArray notes;
    for (auto& n : spec.notes) notes.add (midiNoteName (n.note + ps));
    const int octave = chordOctaveBox.getSelectedId();
    const auto first = chordSpec (0), last = chordSpec ((int) currentChords.size() - 1);
    exportInfo.setText ("EXPORT  Octave " + juce::String (octave) + ":  " + spec.label + " " + inversionBox.getText().toLowerCase()
                        + " = " + notes.joinIntoString (" ") + "      All " + juce::String ((int) currentChords.size()) + ": roots "
                        + midiNoteName (first.rootNote) + " to " + midiNoteName (last.rootNote)
                        + "      (C3 = middle C)", juce::dontSendNotification);
    exportInfo.setColour (juce::Label::textColourId, accent.brighter (0.4f));
}

SampleSpec MainComponent::chordSpec (int index) const
{
    SampleSpec s;
    auto i = styleBox.getSelectedItemIndex();
    if (! juce::isPositiveAndBelow (index, (int) currentChords.size()) || ! juce::isPositiveAndBelow (i, styles.size())) return s;
    const auto st = styles[i];
    auto& c = currentChords[(size_t) index];
    auto octave = chordOctaveBox.getSelectedId();
    auto inversion = juce::jmin (inversionBox.getSelectedId() - 1, (int) c.intervals.size() - 1);
    auto root = c.rootPc + 12 * (octave + 2);                // sounding root (60 = C3)
    auto sounding = invertChord (root, c.intervals, inversion);

    s.kind = "Chord";
    s.label = c.roman + "-" + c.name;
    s.voicing = inversion == 0 ? "root" : "inv" + juce::String (inversion);
    s.rootNote = root;
    s.styleId = st.id; s.tag = st.tag; s.shortTag = st.shortTag;
    s.processing = st.processing;
    auto engine = selectedEngine();
    s.engine = engine.isNotEmpty() && engine != "mixed" ? engine : (st.engine == "mixed" ? juce::String ("epiano") : st.engine);
    s.variation = juce::Random::getSystemRandom().nextInt64();
    auto velocity = (st.velocityLow + st.velocityHigh) / 2;
    auto hold = (st.holdLow + st.holdHigh) * 0.5;
    for (auto n : sounding) s.notes.push_back ({ n - st.processing.pitchShift, velocity, 0.0, hold }); // played so it sounds right after varispeed
    auto tag = st.shortTag.isNotEmpty() ? st.shortTag : st.tag;
    s.stemHead = {};
    s.stemTail = tag + "_" + s.label + "_" + (inversion == 0 ? juce::String ("root") : "inv" + juce::String (inversion)) + "_" + midiNoteName (sounding.front());
    s.index = index + 1;
    s.fileStem = safeFileName (juce::String (index + 1).paddedLeft ('0', 2) + "_" + s.stemTail);
    return s;
}

void MainComponent::playChord (int index)
{
    selectedChord = index;
    updateChordRow();
    if (job != nullptr) return;
    auto spec = chordSpec (index);
    if (spec.notes.empty()) return;
    applyLiveEngine();
    auto& collector = player.getMidiMessageCollector();
    auto now = juce::Time::getMillisecondCounterHiRes() * 0.001;
    for (auto& n : spec.notes)
    {
        auto on = juce::MidiMessage::noteOn (1, n.note + spec.processing.pitchShift, (juce::uint8) n.velocity); // preview = sounding pitch
        on.setTimeStamp (now);
        collector.addMessageToQueue (on);
        juce::Timer::callAfterDelay ((int) (n.lengthSec * 1000.0),
            [safe = juce::Component::SafePointer<MainComponent> (this), note = on.getNoteNumber()]
            {
                if (safe == nullptr) return;
                auto off = juce::MidiMessage::noteOff (1, note);
                off.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001);
                safe->player.getMidiMessageCollector().addMessageToQueue (off);
            });
    }
    log ("Chord " + spec.label + " (" + spec.voicing + ", " + midiNoteName (spec.notes.front().note + spec.processing.pitchShift) + " up)");
}

void MainComponent::exportChords (const std::vector<int>& indices)
{
    if (job != nullptr || instrument == nullptr) return;
    auto i = styleBox.getSelectedItemIndex();
    if (! juce::isPositiveAndBelow (i, styles.size()) || currentChords.empty()) return;

    std::vector<SampleSpec> specs;
    for (auto idx : indices) if (idx < (int) currentChords.size()) specs.push_back (chordSpec (idx));
    if (specs.empty()) return;

    BatchRequest req;
    req.style = styles[i];
    req.appendOnly = true;
    req.variety = 0;
    req.instrumentName = instrument->getName();
    req.render.sampleRate = rateBox.getSelectedId();
    req.render.bitDepth = bitsBox.getSelectedId();
    req.render.normalise = levelBox.getSelectedId() != 3;
    req.render.loudnessMatch = false; // single picks: keep their natural peak level
    auto scaleName = scaleBox.getSelectedId() > 1 ? scaleBox.getText() : req.style.scale;
    req.outputFolder = outputFolder.getChildFile ("Chord Picks")
                           .getChildFile (safeFileName (req.style.name) + "_" + pitchClassName (currentTonic()) + "_" + scaleName);
    {
        juce::StringArray what;
        for (auto& sp : specs)
        {
            juce::StringArray notes;
            for (auto& n : sp.notes) notes.add (midiNoteName (n.note + sp.processing.pitchShift));
            what.add (sp.label + " (" + notes.joinIntoString (" ") + ")");
        }
        log ("Exporting in Octave " + juce::String (chordOctaveBox.getSelectedId()) + ", " + inversionBox.getText().toLowerCase()
             + ": " + what.joinIntoString (", "));
    }
    startJob (std::move (req), std::move (specs), false);
}

void MainComponent::moreLikeThis (const juce::File& batchFolder, const juce::File& sample)
{
    if (job != nullptr) { log ("Busy - wait for the current job to finish."); return; }
    auto batch = juce::JSON::parse (batchFolder.getChildFile ("_batch.json"));
    auto specsJson = juce::JSON::parse (batchFolder.getChildFile ("_specs.json"));
    auto rel = sample.getRelativePathFrom (batchFolder).replaceCharacter ('\\', '/');
    SampleSpec base;
    bool found = false;
    if (specsJson.isArray())
        for (auto& v : *specsJson.getArray())
            if (v["file"].toString() == rel && specFromVar (v, base)) { found = true; break; }
    if (! found)
    {
        juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, "Can't make variations",
            "This sample isn't in the batch's _specs.json (made by an older version?). Generate the batch again to use this.");
        return;
    }
    auto madeWith = batch["instrument"].toString();
    if (instrument == nullptr || (madeWith.isNotEmpty() && madeWith != instrument->getName()))
    {
        juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, "Load the same instrument first",
            "This sample was made with \"" + madeWith + "\". Load it (and its sound) so the variations match.");
        return;
    }

    // 16 variations: new timbre (built-in), voicing / inversion shifts, velocity and length
    juce::Random rng (juce::Time::currentTimeMillis());
    auto baseName = base.fileStem;
    if (baseName.upToFirstOccurrenceOf ("_", false, false).containsOnly ("0123456789"))
        baseName = baseName.fromFirstOccurrenceOf ("_", false, false);
    std::vector<SampleSpec> specs;
    for (int k = 0; k < 16; ++k)
    {
        auto v = base;
        v.variation = rng.nextInt64();
        auto lengthScale = 0.85 + 0.3 * rng.nextDouble();
        std::vector<int> notes;
        for (auto& n : v.notes) notes.push_back (n.note);
        std::sort (notes.begin(), notes.end());
        if (notes.size() > 1)
        {
            auto alt = notes;
            switch (k % 5)
            {
                case 1: alt.front() += 12; break;                                // up an inversion
                case 2: alt.back() -= 12; break;                                 // down an inversion
                case 3: if (alt.size() >= 4) alt[alt.size() - 2] -= 12; break;   // drop-2
                case 4: for (size_t j = 1; j < alt.size(); j += 2) alt[j] += 12; break; // open
                default: break;
            }
            std::sort (alt.begin(), alt.end());
            if (alt.front() >= 21 && alt.back() <= 108 && lowIntervalViolations (alt) == 0) notes = alt;
        }
        std::vector<NoteEvent> events;
        for (size_t j = 0; j < notes.size(); ++j)
        {
            auto src = v.notes[juce::jmin (j, v.notes.size() - 1)];
            src.note = notes[j];
            src.velocity = juce::jlimit (1, 127, src.velocity + rng.nextInt (25) - 12);
            src.lengthSec *= lengthScale;
            events.push_back (src);
        }
        v.notes = events;
        v.pad = 0;
        v.stemHead = {};
        v.stemTail = baseName + "_like";
        specs.push_back (v);
    }
    renumberSamples (specs);

    BatchRequest req;
    if (auto* st = findBuiltInStyle (base.styleId)) req.style = *st;
    else if (juce::isPositiveAndBelow (styleBox.getSelectedItemIndex(), styles.size())) req.style = styles[styleBox.getSelectedItemIndex()];
    req.instrumentName = instrument->getName();
    req.render.sampleRate = (double) batch.getProperty ("sampleRate", 44100.0);
    req.render.bitDepth = (int) batch.getProperty ("bitDepth", 24);
    req.render.loudnessMatch = (bool) batch.getProperty ("loudnessMatch", false);
    req.variety = juce::jmax (1, varietyBox.getSelectedId() - 1); // never 16 copies of the same thing
    req.outputFolder = batchFolder.getSiblingFile (batchFolder.getFileName() + "_more_" + safeFileName (baseName).substring (0, 24))
                           .getNonexistentSibling (false);
    log ("More like " + sample.getFileName() + " ...");
    startJob (std::move (req), std::move (specs), true);
}

void MainComponent::addOrReplaceStyle (const Style& st, bool select)
{
    int index = -1;
    for (int i = 0; i < styles.size(); ++i) if (styles[i].id == st.id) index = i;
    if (index < 0)
    {
        styles.add (st);
        index = styles.size() - 1;
        styleBox.addItem (st.name.endsWith ("(mine)") ? st.name : st.name + " (mine)", index + 1);
    }
    else
    {
        styles.set (index, st);
        styleBox.changeItemText (index + 1, st.name.endsWith ("(mine)") ? st.name : st.name + " (mine)");
    }
    if (select) styleBox.setSelectedId (index + 1, juce::sendNotificationSync);
}

void MainComponent::openStyleEditor()
{
    auto i = styleBox.getSelectedItemIndex();
    if (! juce::isPositiveAndBelow (i, styles.size())) return;
    if (styles[i].isKit()) { log ("Kits mix other styles - pick one of its recipes to edit."); return; }
    styleEditorWindow.reset();
    auto* editor = new StyleEditor (styles[i]);
    auto safe = juce::Component::SafePointer<MainComponent> (this);
    editor->onSave = [safe] (const Style& st) { if (safe != nullptr) { safe->addOrReplaceStyle (st, true); safe->log ("Saved your style \"" + st.name + "\""); } };
    editor->onApply = [safe] (const Style& st) { if (safe != nullptr) safe->addOrReplaceStyle (st, true); };
    styleEditorWindow = std::make_unique<ToolWindow> ("Edit Style", editor, [safe] { if (safe != nullptr) safe->styleEditorWindow.reset(); });
}

//==============================================================================
void MainComponent::handleNoteOn (juce::MidiKeyboardState*, int ch, int note, float vel)
{
    auto m = juce::MidiMessage::noteOn (ch, note, vel);
    m.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001);
    player.getMidiMessageCollector().addMessageToQueue (m);
}

void MainComponent::handleNoteOff (juce::MidiKeyboardState*, int ch, int note, float vel)
{
    auto m = juce::MidiMessage::noteOff (ch, note, vel);
    m.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001);
    player.getMidiMessageCollector().addMessageToQueue (m);
}
}
