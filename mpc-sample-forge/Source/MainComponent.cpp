#include "MainComponent.h"
#include "BuiltInSynth.h"
#include "DiagnosticsPanel.h"
#include "ReviewPanel.h"

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
    styleBox.onChange = [this] { styleChanged(); };
    addAndMakeVisible (styleBox);
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

    registersBox.addItem ("Registers: just this one", 1);
    registersBox.addItem ("+ octave down", 2);
    registersBox.addItem ("+ octave up", 3);
    registersBox.addItem ("+ octave down & up", 4);
    registersBox.addItem ("+ 2 octaves down & up", 5);
    registersBox.setSelectedId (settings->getIntValue ("registers", 1), juce::dontSendNotification);
    registersBox.setTooltip ("Also render every kept sample in other registers (folders Octave_Down / Octave_Up). "
                             "With 16 Levels > Tune, load the register nearest the notes you play: big re-pitches sound grainy or dull.");
    registersBox.onChange = [this] { props.getUserSettings()->setValue ("registers", registersBox.getSelectedId()); };

    shortNamesToggle.setToggleState (settings->getBoolValue ("shortNames", true), juce::dontSendNotification);
    shortNamesToggle.setTooltip ("014_EPBell_C3 instead of JetLifeEPBell_C3_014_Key_C3 - easier to read on the MPC screen");
    shortNamesToggle.onClick = [this] { props.getUserSettings()->setValue ("shortNames", shortNamesToggle.getToggleState()); };
    for (auto* c : std::initializer_list<juce::Component*> { &levelBox, &registersBox, &shortNamesToggle })
        addAndMakeVisible (c);
    smartPickToggle.setToggleState (settings->getBoolValue ("smartPick", true), juce::dontSendNotification);
    smartPickToggle.setTooltip ("Renders 3x as many samples, scores them (pitch clarity, muddy voicings, harshness, clicks) "
                                "and keeps only the best, most varied ones. Takes about 3x longer.");
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

    // ---------------------------------------------------------- audio
    deviceManager.initialiseWithDefaultDevices (0, 2);
    deviceManager.addAudioCallback (&player);
    for (auto& m : juce::MidiInput::getAvailableDevices())
    {
        deviceManager.setMidiInputDeviceEnabled (m.identifier, true);
        deviceManager.addMidiInputDeviceCallback (m.identifier, &player);
    }

    styleBox.setSelectedItemIndex (juce::jmax (0, settings->getIntValue ("styleIndex", 0)), juce::sendNotificationSync);

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
    setSize (980, 760);
}

MainComponent::~MainComponent()
{
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
    styleFileButton.setBounds (r.removeFromRight (110)); r.removeFromRight (6);
    styleBox.setBounds (r);

    r = row (40);
    r.removeFromLeft (labelW);
    styleDesc.setBounds (r);

    r = row();
    keyLabel.setBounds (r.removeFromLeft (labelW));
    keyBox.setBounds (r.removeFromLeft (150)); r.removeFromLeft (6);
    scaleBox.setBounds (r.removeFromLeft (170)); r.removeFromLeft (20);
    countLabel.setBounds (r.removeFromLeft (70));
    countSlider.setBounds (r.removeFromLeft (150));

    r = row();
    seedLabel.setBounds (r.removeFromLeft (labelW));
    seedEditor.setBounds (r.removeFromLeft (110)); r.removeFromLeft (6);
    diceButton.setBounds (r.removeFromLeft (60)); r.removeFromLeft (20);
    prefixLabel.setBounds (r.removeFromLeft (60));
    prefixEditor.setBounds (r.removeFromLeft (200));

    r = row();
    audioLabel.setBounds (r.removeFromLeft (labelW));
    rateBox.setBounds (r.removeFromLeft (110)); r.removeFromLeft (6);
    bitsBox.setBounds (r.removeFromLeft (100)); r.removeFromLeft (12);
    levelBox.setBounds (r.removeFromLeft (210)); r.removeFromLeft (12);
    smartPickToggle.setBounds (r.removeFromLeft (220));

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
    generateButton.setBounds (r.removeFromRight (150)); r.removeFromRight (8);
    reviewButton.setBounds (r.removeFromRight (90)); r.removeFromRight (8);
    auditionButton.setBounds (r.removeFromRight (130)); r.removeFromRight (8);
    planButton.setBounds (r.removeFromRight (110)); r.removeFromRight (12);
    progressBar.setBounds (r);

    keyboard.setBounds (area.removeFromBottom (80));
    keyboard.setKeyWidth ((float) keyboard.getWidth() / 52.0f); // 88 keys = 52 white keys
    keyboard.setLowestVisibleKey (21);
    area.removeFromBottom (8);
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
    styleDesc.setText (styles[i].description + "   (scale: " + styles[i].scale + ")", juce::dontSendNotification);
    props.getUserSettings()->setValue ("styleIndex", i);
    lastPlan.clear();
    applyLiveEngine();
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
    if (smartPickToggle.getToggleState())
    {
        req.plan.count = juce::jmin (2000, wanted * 3);
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
    auto& spec = lastPlan[(size_t) index];
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
                                                             &engineBox, &rootBox, &lengthBox, &banksToggle, &smartPickToggle, &reviewButton,
                                                             &outButton, &planButton, &auditionButton, &keyboard })
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

    keyboardState.allNotesOff (0);
    player.setProcessor (nullptr); // the job owns the instrument while it renders
    setBusy (true);
    progress = 0;
    log ("==== Generating " + juce::String ((int) specs.size()) + " samples into " + req.outputFolder.getFullPathName());

    auto safe = juce::Component::SafePointer<MainComponent> (this);
    job = std::make_unique<BatchJob> (*instrument, req, std::move (specs));
    job->onProgress = [safe] (int done, int total, const juce::String& msg)
    {
        if (safe == nullptr) return;
        safe->progress = total > 0 ? (double) done / total : 0.0;
        safe->log (msg);
    };
    job->onFinished = [safe, folder = req.outputFolder] (bool ok, const juce::String& summary)
    {
        if (safe == nullptr) return;
        safe->jobFinished (ok, summary);
        if (folder.isDirectory())
        {
            safe->lastBatchFolder = folder;
            safe->openReview (folder); // straight into auditioning and picking keepers
        }
    };
    job->startThread();
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
    reviewWindow = std::make_unique<ToolWindow> ("Review & Keep - " + batchFolder.getFileName(),
                                                 new ReviewPanel (batchFolder, deviceManager),
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
