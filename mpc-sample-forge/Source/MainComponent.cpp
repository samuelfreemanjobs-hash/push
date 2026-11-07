#include "MainComponent.h"
#include "PresetSweepPanel.h"
#include "QueuePanel.h"
#include "RecipeCardPanel.h"
#include "ProgressionPanel.h"
#include <map>
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

static const std::vector<std::pair<const char*, const char*>>& gridTypes()
{
    static const std::vector<std::pair<const char*, const char*>> t {
        { "maj", "Maj" }, { "min", "m" }, { "7", "7" }, { "maj7", "M7" }, { "m7", "m7" }, { "6", "6" },
        { "m6", "m6" }, { "9", "9" }, { "maj9", "M9" }, { "m9", "m9" }, { "m11", "m11" }, { "13", "13" },
        { "sus2", "sus2" }, { "sus4", "sus4" }, { "7sus4", "7sus4" }, { "add9", "add9" }, { "dim", "dim" }, { "dim7", "dim7" },
        { "m7b5", "m7b5" }, { "aug", "aug" }, { "69", "6/9" }, { "7s9", "7#9" }, { "5", "5 (pow)" }, { "note", "Note" } };
    return t;
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
    queueButton.setTooltip ("The render queue: batches that render one after another while you're away");
    queueButton.onClick = [this] { openQueue(); };
    addAndMakeVisible (queueButton);
    cardsButton.setTooltip ("Paste a recipe book page (e.g. your daily ChatGPT installment) and render it with its exact notes and file names");
    cardsButton.onClick = [this] { openRecipeCards(); };
    addAndMakeVisible (cardsButton);
    progressionsButton.setTooltip ("Build progressions from chord blocks, or let a genre preset write one - with keys, bass, pad, lead and drums - and export MIDI and loops");
    progressionsButton.setColour (juce::TextButton::buttonColourId, accent.darker (0.5f));
    progressionsButton.onClick = [this] { openProgressions(); };
    addAndMakeVisible (progressionsButton);
    addQueueButton.setTooltip ("Save everything set up here (recipe, key, count, instrument + sound, format, takes ...) as a batch "
                               "in the render queue, to render later with the others");
    addQueueButton.onClick = [this] { addToQueue(); };
    addAndMakeVisible (addQueueButton);
    {
        juce::String err;
        queue = loadQueue (defaultQueueFile(), err);
        if (err.isNotEmpty()) log ("Queue: " + err);
        updateQueueButton();
    }
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
    sweepButton.setTooltip ("Try every preset of this instrument with the selected recipe, keep the best ones and render a bank of each");
    sweepButton.onClick = [this] { openPresetSweep(); };
    sweepButton.setColour (juce::TextButton::buttonColourId, accent.darker (0.5f));
    addAndMakeVisible (sweepButton);
    for (auto* b : { &browseButton, &editButton, &saveSoundButton, &loadSoundButton })
        addAndMakeVisible (b);

    // ---------------------------------------------------------- style
    {
        // grouped under headings so a recipe is easy to find; the order of `styles` matches the menu
        static const std::vector<std::pair<const char*, const char*>> groups {
            { "Genres (a mix of chords and notes)", "neo-soul lofi trap drill rnb gospel house jazz boom-bap afrobeats pop cinematic ambient synthwave" },
            { "Kits (several recipes in one batch)", "westcoast-kit memphis-kit atlanta-kit guide-palette one-shot-palette" },
            { "Progressions & chord sets", "trap-progressions soul-chords ninth-chords screw-rnb gospel-rnb-90s smooth-jazz-chords rnb-80s-chords vaporwave chillwave phonkwave" },
            { "Keys, electric piano & organ", "wiz-ep gfunk-ep jetlife-ep-bell wurli-keys clav-funk zaytoven-organ memphis-organ organ-stabs" },
            { "Bells & music box", "bell-tines phonk-cowbell horror-musicbox" },
            { "Strings, brass & horns", "philly-strings soul-horns ross-lux-strings ross-lux-brass jeezy-strings jeezy-brass trap-brass trap-brass-stabs trap-pizz pizz-strings luxury-harp smooth-jazz-sax" },
            { "Synths & leads", "njs-stabs gucci-synth shawty-redd-synth juicyj-strip-synth lead-synth gfunk-lead talkbox-lead detuned-synth plucks polyester-pluck" },
            { "Pads, choirs & textures", "smooth-jazz-pad sade-pad choir screwed-choir mellotron-flute" },
            { "Bass & 808", "soul-bass quiet-storm-bass neo-soul-bass upright-bass njs-bass 808-bass bassline moog-bass rhodes-bass log-drum" },
            { "Scale banks (every pad in key)", "scale-notes trap-scale" } };
        auto all = builtInStyles();
        juce::StringArray used;
        for (auto& [heading, ids] : groups)
        {
            bool headed = false;
            for (auto& id : juce::StringArray::fromTokens (ids, " ", ""))
                for (auto& st : all)
                    if (st.id == id && ! used.contains (id))
                    {
                        if (! headed) { styleBox.addSectionHeading (heading); headed = true; }
                        styles.add (st);
                        styleBox.addItem (st.name, styles.size());
                        used.add (id);
                    }
        }
        bool otherHeaded = false;
        for (auto& st : all)
            if (! used.contains (st.id))
            {
                if (! otherHeaded) { styleBox.addSectionHeading ("Other"); otherHeaded = true; }
                styles.add (st);
                styleBox.addItem (st.name, styles.size());
            }
    }
    {
        juce::StringArray errors;
        auto mineList = StyleEditor::loadUserStyles (errors);
        if (! mineList.isEmpty()) styleBox.addSectionHeading ("Your styles");
        for (auto& mine : mineList)
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
    takesBox.addItem ("1 take", 1);
    for (int t = 2; t <= 4; ++t) takesBox.addItem (juce::String (t) + " takes (round robin)", t);
    takesBox.setSelectedId (juce::jlimit (1, 4, settings->getIntValue ("takes", 1)), juce::dontSendNotification);
    takesBox.setTooltip ("Round robins: also render 2-4 slightly different takes of every sample (velocity, timing, length) "
                         "into Round_Robin/Take_2 ... - same pad numbers, so repeated hits don't sound machine-gunned");
    takesBox.onChange = [this] { props.getUserSettings()->setValue ("takes", takesBox.getSelectedId()); updateEstimate(); };
    for (auto* c : std::initializer_list<juce::Component*> { &levelBox, &registersBox, &takesBox, &shortNamesToggle })
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
    for (int oct = 0; oct <= 6; ++oct) chordOctaveBox.addItem ("Oct " + juce::String (oct) + " (C" + juce::String (oct) + ")", oct + 1);
    chordOctaveBox.setSelectedId (juce::jlimit (0, 6, settings->getIntValue ("chordOctave", 2)) + 1, juce::dontSendNotification);
    chordOctaveBox.setTooltip ("Which octave the chords are previewed and exported in (C3 = middle C on the MPC)");
    chordOctaveBox.onChange = [this] { props.getUserSettings()->setValue ("chordOctave", chordOctaveBox.getSelectedId() - 1); updateChordRow(); };
    inversionBox.addItem ("Root position", 1);
    inversionBox.addItem ("1st inversion", 2);
    inversionBox.addItem ("2nd inversion", 3);
    inversionBox.addItem ("3rd inversion", 4);
    inversionBox.setSelectedId (1, juce::dontSendNotification);
    inversionBox.setTooltip ("Play / export the chord inverted: the lowest notes move up an octave");
    inversionBox.onChange = [this] { updateChordRow(); };
    exportChordButton.setTooltip ("Render the selected chord - shown in the EXPORT line above (this octave + inversion, with the recipe's processing) into 'Chord Picks'");
    exportChordButton.onClick = [this] { exportSpecs ({ specForChord (currentGridChord(), 1) }, "the selected chord"); };
    exportRowButton.setTooltip ("Render all 7 chords of the key (this octave + inversion) into 'Chord Picks'");
    exportRowButton.onClick = [this] { exportChords ({ 0, 1, 2, 3, 4, 5, 6 }); };
    for (auto* c : std::initializer_list<juce::Component*> { &chordOctaveBox, &inversionBox, &exportChordButton, &exportRowButton })
        addAndMakeVisible (c);
    for (auto* cb : { &keyBox, &scaleBox })
    {
        auto previous = cb->onChange;
        cb->onChange = [this, previous] { if (previous) previous(); updateChordRow(); };
    }

    // ---------------------------------------------------------- chord grid (DR-5 style) + palette
    gridRootLabel.setText ("Root", juce::dontSendNotification);
    gridTypeLabel.setText ("Type", juce::dontSendNotification);
    for (auto* l : { &gridRootLabel, &gridTypeLabel }) { l->setColour (juce::Label::textColourId, dim); addAndMakeVisible (l); }
    for (int r = 0; r < 12; ++r)
    {
        auto* b = rootButtons.add (new juce::TextButton (pitchClassName (r)));
        b->setTooltip ("Root " + pitchClassName (r) + ": plays it with the selected type (brighter = in your key)");
        b->onClick = [this, r] { gridRoot = r; playGridChord(); };
        addAndMakeVisible (b);
    }
    for (size_t t = 0; t < gridTypes().size(); ++t)
    {
        auto id = juce::String (gridTypes()[t].first);
        auto* b = typeButtons.add (new juce::TextButton (gridTypes()[t].second));
        b->setTooltip (id == "note" ? juce::String ("A single note on the root") : "Chord type " + juce::String (gridTypes()[t].second)
                                                                                + " (brighter = fits your key on this root)");
        b->onClick = [this, id] { gridQuality = id; playGridChord(); };
        addAndMakeVisible (b);
    }
    voicingBox.addItem ("Close", 1); voicingBox.addItem ("Drop 2", 2); voicingBox.addItem ("Open", 3);
    voicingBox.addItem ("Spread", 4); voicingBox.addItem ("Rootless", 5); voicingBox.addItem ("Shell", 6);
    voicingBox.setSelectedId (juce::jlimit (1, 6, settings->getIntValue ("gridVoicing", 1)), juce::dontSendNotification);
    voicingBox.setTooltip ("How the chord's notes are spread (the lowest note stays in the selected octave)");
    voicingBox.onChange = [this] { props.getUserSettings()->setValue ("gridVoicing", voicingBox.getSelectedId()); updateExportInfo(); };
    addPaletteButton.setTooltip ("Add the selected chord - exactly as you hear it (octave, voicing, inversion) - to the palette");
    addPaletteButton.onClick = [this] { addToPalette(); };
    undoPaletteButton.onClick = [this] { if (! palette.empty()) { log ("- Palette: " + palette.back().describe()); palette.pop_back(); savePalette(); } };
    clearPaletteButton.onClick = [this] { palette.clear(); savePalette(); log ("Palette cleared"); };
    exportPaletteButton.setTooltip ("Render every chord in the palette (with the recipe's sound and FX) into 'Chord Picks'");
    exportPaletteButton.setColour (juce::TextButton::buttonColourId, accent.darker (0.4f));
    exportPaletteButton.onClick = [this] { exportSpecs (paletteSpecs(), "the palette"); };
    usePaletteToggle.setTooltip ("Generate renders exactly the palette - nothing random (count, Smart Pick and Variety are ignored)");
    usePaletteToggle.setToggleState (settings->getBoolValue ("usePalette", false), juce::dontSendNotification);
    usePaletteToggle.onClick = [this] { props.getUserSettings()->setValue ("usePalette", usePaletteToggle.getToggleState()); planSignature = {}; };
    exactPreviewToggle.setTooltip ("On: chord clicks and Audition are rendered exactly like the batch (recipe FX, length, envelope) - "
                                   "what you hear is what you get. Off: played live on the instrument (instant, no FX).");
    exactPreviewToggle.setToggleState (settings->getBoolValue ("exactPreview", true), juce::dontSendNotification);
    exactPreviewToggle.onClick = [this] { props.getUserSettings()->setValue ("exactPreview", exactPreviewToggle.getToggleState()); };
    paletteLabel.setColour (juce::Label::textColourId, text);
    paletteLabel.setMinimumHorizontalScale (0.6f);
    for (auto* c : std::initializer_list<juce::Component*> { &voicingBox, &addPaletteButton, &undoPaletteButton, &clearPaletteButton,
                                                             &exportPaletteButton, &usePaletteToggle, &exactPreviewToggle, &paletteLabel })
        addAndMakeVisible (c);
    for (auto& item : juce::StringArray::fromTokens (settings->getValue ("palette"), ";", ""))
    {
        PaletteChord c;
        if (PaletteChord::fromString (item, c)) palette.push_back (c);
    }

    // ---------------------------------------------------------- octaves for Generate
    octaveRangeBox.addItem ("Octaves: recipe's", 1);
    for (int o = 0; o <= 6; ++o) octaveRangeBox.addItem ("Octave " + juce::String (o) + " (C" + juce::String (o) + "-B" + juce::String (o) + ")", 10 + o);
    for (int o = 0; o <= 5; ++o) octaveRangeBox.addItem ("Octaves " + juce::String (o) + "-" + juce::String (o + 1), 100 + o * 10 + o + 1);
    octaveRangeBox.setSelectedId (settings->getIntValue ("octaveRange", 1), juce::dontSendNotification);
    if (octaveRangeBox.getSelectedId() == 0) octaveRangeBox.setSelectedId (1, juce::dontSendNotification);
    octaveRangeBox.setTooltip ("Which octave(s) Generate makes notes and chords in: the lowest note of every sample lands there "
                               "(C3 = middle C on the MPC). 'Recipe's' = each recipe's own range.");
    octaveRangeBox.onChange = [this] { props.getUserSettings()->setValue ("octaveRange", octaveRangeBox.getSelectedId()); };
    addAndMakeVisible (octaveRangeBox);

    // ---------------------------------------------------------- what Generate will make
    nextGenerateLabel.setColour (juce::Label::textColourId, juce::Colour (0xffffd166));
    nextGenerateLabel.setColour (juce::Label::backgroundColourId, panel);
    nextGenerateLabel.setJustificationType (juce::Justification::topLeft);
    nextGenerateLabel.setMinimumHorizontalScale (0.8f);
    nextGenerateLabel.setFont (juce::FontOptions (12.5f));
    nextGenerateLabel.setTooltip ("Exactly what the Generate button will render with the current settings. Audition Random plays samples from this list.");
    addAndMakeVisible (nextGenerateLabel);
    planWatcher = std::make_unique<juce::TimedCallback> ([this] { updatePlanSummary(); });
    planWatcher->startTimer (400);
    auditionOctaveBox.setVisible (false); // replaced by the Octaves menu, which Generate and Audition share

    // ---------------------------------------------------------- audio
    deviceManager.initialiseWithDefaultDevices (0, 2);
    deviceManager.addAudioCallback (&player);
    deviceManager.addAudioCallback (&previewPlayer);  // exact previews play alongside the live instrument
    for (auto& m : juce::MidiInput::getAvailableDevices())
    {
        deviceManager.setMidiInputDeviceEnabled (m.identifier, true);
        deviceManager.addMidiInputDeviceCallback (m.identifier, &player);
    }

    {
        // a saved style may have been deleted since: fall back to the first one
        auto savedId = settings->getValue ("styleId");
        int saved = 0;
        for (int k = 0; k < styles.size(); ++k) if (styles[k].id == savedId) { saved = k; break; }
        styleBox.setSelectedItemIndex (saved, juce::sendNotificationSync);
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
    setSize (980, 920);
}

MainComponent::~MainComponent()
{
    planWatcher.reset();
    previewPlayer.setSource (nullptr);
    deviceManager.removeAudioCallback (&previewPlayer);
    progressionsWindow.reset();
    cardsWindow.reset();
    queueWindow.reset();
    queueRunning = false;
    sweepWindow.reset();
    sweep.reset();          // before the instrument it uses
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
    titleRow.removeFromRight (8);
    queueButton.setBounds (titleRow.removeFromRight (120).reduced (0, 3));
    titleRow.removeFromRight (8);
    cardsButton.setBounds (titleRow.removeFromRight (130).reduced (0, 3));
    titleRow.removeFromRight (8);
    progressionsButton.setBounds (titleRow.removeFromRight (130).reduced (0, 3));
    title.setBounds (titleRow);
    area.removeFromTop (12);

    auto row = [&area] (int h = 28) { auto r = area.removeFromTop (h); area.removeFromTop (8); return r; };
    const int labelW = 100;

    auto r = row();
    pluginLabel.setBounds (r.removeFromLeft (labelW));
    sweepButton.setBounds (r.removeFromRight (116)); r.removeFromRight (6);
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
    engineBox.setBounds (r.removeFromLeft (160)); r.removeFromLeft (10);
    rootLabel.setBounds (r.removeFromLeft (100));
    rootBox.setBounds (r.removeFromLeft (150)); r.removeFromLeft (6);
    lengthBox.setBounds (r.removeFromLeft (120)); r.removeFromLeft (8);
    banksToggle.setBounds (r.removeFromLeft (124)); r.removeFromLeft (6);
    octaveRangeBox.setBounds (r);

    r = row();
    outLabel.setBounds (r.removeFromLeft (labelW));
    outButton.setBounds (r.removeFromRight (100)); r.removeFromRight (6);
    shortNamesToggle.setBounds (r.removeFromRight (120)); r.removeFromRight (6);
    takesBox.setBounds (r.removeFromRight (176)); r.removeFromRight (6);
    registersBox.setBounds (r.removeFromRight (176)); r.removeFromRight (10);
    outPath.setBounds (r);

    area.removeFromTop (14);
    r = row (34);
    generateButton.setBounds (r.removeFromRight (130)); r.removeFromRight (6);
    addQueueButton.setBounds (r.removeFromRight (106)); r.removeFromRight (6);
    moreButton.setBounds (r.removeFromRight (80)); r.removeFromRight (6);
    reviewButton.setBounds (r.removeFromRight (78)); r.removeFromRight (6);
    auditionButton.setBounds (r.removeFromRight (124)); r.removeFromRight (6);
    planButton.setBounds (r.removeFromRight (100)); r.removeFromRight (6);
    estimateLabel.setVisible (false); // the estimate shows inside the progress bar while idle
    progressBar.setBounds (r);

    keyboard.setBounds (area.removeFromBottom (64));
    keyboard.setKeyWidth ((float) keyboard.getWidth() / 52.0f); // 88 keys = 52 white keys
    keyboard.setLowestVisibleKey (21);
    area.removeFromBottom (6);

    // palette row
    auto paletteRow = area.removeFromBottom (28);
    area.removeFromBottom (4);
    addPaletteButton.setBounds (paletteRow.removeFromLeft (90)); paletteRow.removeFromLeft (4);
    undoPaletteButton.setBounds (paletteRow.removeFromLeft (56)); paletteRow.removeFromLeft (4);
    clearPaletteButton.setBounds (paletteRow.removeFromLeft (56)); paletteRow.removeFromLeft (8);
    exportPaletteButton.setBounds (paletteRow.removeFromRight (120)); paletteRow.removeFromRight (6);
    usePaletteToggle.setBounds (paletteRow.removeFromRight (172)); paletteRow.removeFromRight (6);
    paletteLabel.setBounds (paletteRow);

    // chord types: two rows of 12
    auto typeRow2 = area.removeFromBottom (24);
    area.removeFromBottom (3);
    auto typeRow1 = area.removeFromBottom (24);
    area.removeFromBottom (3);
    gridTypeLabel.setBounds (typeRow1.removeFromLeft (52));
    typeRow2.removeFromLeft (52);
    for (int t = 0; t < typeButtons.size(); ++t)
    {
        auto& rowArea = t < 12 ? typeRow1 : typeRow2;
        auto w = rowArea.getWidth() / (12 - (t % 12));
        typeButtons[t]->setBounds (rowArea.removeFromLeft (w).reduced (1, 0));
    }

    // roots + voicing + preview mode
    auto rootRow = area.removeFromBottom (24);
    area.removeFromBottom (4);
    gridRootLabel.setBounds (rootRow.removeFromLeft (52));
    exactPreviewToggle.setBounds (rootRow.removeFromRight (112)); rootRow.removeFromRight (4);
    voicingBox.setBounds (rootRow.removeFromRight (110)); rootRow.removeFromRight (8);
    for (int k = 0; k < rootButtons.size(); ++k)
    {
        auto w = rootRow.getWidth() / (12 - k);
        rootButtons[k]->setBounds (rootRow.removeFromLeft (w).reduced (1, 0));
    }

    // the key's 7 chords + octave / inversion / export
    auto chordRow = area.removeFromBottom (28);
    area.removeFromBottom (2);
    exportInfo.setBounds (area.removeFromBottom (18).withTrimmedLeft (52));
    chordRowLabel.setBounds (chordRow.removeFromLeft (52));
    exportRowButton.setBounds (chordRow.removeFromRight (100)); chordRow.removeFromRight (4);
    exportChordButton.setBounds (chordRow.removeFromRight (100)); chordRow.removeFromRight (6);
    inversionBox.setBounds (chordRow.removeFromRight (120)); chordRow.removeFromRight (4);
    chordOctaveBox.setBounds (chordRow.removeFromRight (110)); chordRow.removeFromRight (6);
    auto bw = chordRow.getWidth() / 7;
    for (int i = 0; i < chordButtons.size(); ++i) chordButtons[i]->setBounds (chordRow.removeFromLeft (bw).reduced (2, 0));
    area.removeFromBottom (4);
    nextGenerateLabel.setBounds (area.removeFromBottom (44));
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
    props.getUserSettings()->setValue ("styleId", styles[i].id);
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

bool MainComponent::buildRequest (BatchRequest& req, std::vector<SampleSpec>& specs, bool quiet)
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
    req.roundRobins = juce::jlimit (1, 4, takesBox.getSelectedId());
    {
        auto o = octaveRangeBox.getSelectedId();
        if (o >= 100) { req.plan.octaveLow = (o - 100) / 10; req.plan.octaveHigh = (o - 100) % 10; }
        else if (o >= 10) req.plan.octaveLow = req.plan.octaveHigh = o - 10;
    }
    req.plan.usePalette = usePaletteToggle.getToggleState() && ! palette.empty();
    if (req.plan.usePalette) req.plan.palette = palette;
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

    juce::StringArray notes;
    const bool ok = planBatch (req, smartPickToggle.getToggleState(), specs, notes);
    if (! quiet) for (auto& n : notes) log (n);
    return ok;
}

static juce::String chordTypeOf (const SampleSpec& s)
{
    auto label = s.label;
    if (s.kind == "KeyStab") return "triad stab";
    if (label.contains ("-") && label.upToFirstOccurrenceOf ("-", false, false).containsOnly ("iIvVb#"))
        label = label.fromFirstOccurrenceOf ("-", false, false);                       // "ii-Dm9" -> "Dm9"
    if (label.contains ("-")) label = label.upToFirstOccurrenceOf ("-", false, false); // palette "Cm9-C2" -> "Cm9"
    auto rootLen = label.length() > 1 && (label[1] == 'b' || label[1] == '#') ? 2 : 1;
    auto type = label.substring (rootLen);
    return type.isEmpty() ? juce::String ("maj") : type;
}

juce::String MainComponent::describePlan (const BatchRequest& req, const std::vector<SampleSpec>& specs) const
{
    if (specs.empty()) return "NEXT GENERATE: nothing can be planned with these settings.";
    int chords = 0, singles = 0, lowest = 127, highest = 0;
    std::map<juce::String, int> types;
    for (auto& s : specs)
    {
        int low = 127;
        for (auto& n : s.notes) low = std::min (low, n.note + s.processing.pitchShift);
        lowest = std::min (lowest, low); highest = std::max (highest, low);
        if (s.kind == "Chord" || s.kind == "Stab") { ++chords; ++types[chordTypeOf (s)]; }
        else ++singles;
    }
    std::vector<std::pair<int, juce::String>> sorted;
    for (auto& [t, n] : types) sorted.push_back ({ n, t });
    std::sort (sorted.begin(), sorted.end(), [] (auto& a, auto& b) { return a.first > b.first; });
    juce::StringArray top;
    for (size_t k = 0; k < sorted.size() && k < 6; ++k) top.add (sorted[k].second + " x" + juce::String (sorted[k].first));
    if (sorted.size() > 6) top.add ("+" + juce::String ((int) sorted.size() - 6) + " more types");

    const bool picking = req.smartPickTarget > 0;
    const int count = picking ? req.smartPickTarget : (int) specs.size();
    auto scale = req.plan.scale.isNotEmpty() ? req.plan.scale : req.style.scale;
    juce::String t = "NEXT GENERATE:  ";
    if (req.plan.usePalette) t << "your palette, exactly as picked - " << (int) specs.size() << " samples";
    else t << count << " samples of " << req.style.name;
    if (! req.plan.usePalette)
        t << (req.plan.fixedRoot >= 0 ? "  (16 Levels, all on " + midiNoteName (req.plan.fixedRoot) + ")"
              : req.plan.key == "random" ? juce::String (" in random keys") : " in " + req.plan.key + " " + scale);
    t << "  |  lowest notes " << midiNoteName (lowest) << " to " << midiNoteName (highest)
      << "  |  " << (req.plan.builtIn ? "Built-in " + (req.plan.engine.isNotEmpty() ? req.plan.engine : req.style.engine) : req.instrumentName)
      << ", seed " << juce::String (req.plan.seed);
    if (req.roundRobins > 1) t << "  |  x" << req.roundRobins << " takes";
    if (! req.registers.empty()) t << "  |  + octave copies";
    t << "\n" << (picking ? "Smart Pick renders " + juce::String ((int) specs.size()) + " candidates - " : juce::String())
      << chords << " chords" << (top.isEmpty() ? juce::String() : " (" + top.joinIntoString (", ") + ")")
      << " + " << singles << " single notes" << (picking ? " - and keeps the best " + juce::String (req.smartPickTarget) : juce::String())
      << ".  Audition Random plays from this exact list.";
    return t;
}

void MainComponent::updatePlanSummary()
{
    if (job != nullptr || sweep != nullptr || instrument == nullptr) return;
    BatchRequest req;
    std::vector<SampleSpec> specs;
    if (! buildRequest (req, specs, true)) { nextGenerateLabel.setText ("NEXT GENERATE: nothing can be planned with these settings.", juce::dontSendNotification); return; }
    juce::String signature = req.style.id + "|" + juce::String (req.smartPickTarget) + "|" + req.instrumentName + "|" + juce::String (req.roundRobins)
                             + "|" + juce::String ((int) req.registers.size());
    for (auto& sp : specs)
    {
        signature << sp.fileStem << sp.engine;
        for (auto& n : sp.notes) signature << n.note << ",";
    }
    if (signature == planSignature) return;
    planSignature = signature;
    lastPlan = specs;   // Audition plays from exactly what Generate will make
    nextGenerateLabel.setText (describePlan (req, specs), juce::dontSendNotification);
    updateEstimate();
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
    // a sample from the exact list Generate will render (see the NEXT GENERATE line)
    previewSpec (lastPlan[(size_t) index], "#" + juce::String (index + 1) + " of " + juce::String ((int) lastPlan.size()) + ": "
                                            + lastPlan[(size_t) index].fileStem);
}

void MainComponent::setBusy (bool busy)
{
    for (auto* c : std::initializer_list<juce::Component*> { &pluginBox, &browseButton, &loadSoundButton, &sweepButton, &styleBox,
                                                             &styleFileButton, &keyBox, &scaleBox, &countSlider, &seedEditor,
                                                             &diceButton, &prefixEditor, &rateBox, &bitsBox, &levelBox, &registersBox, &takesBox, &shortNamesToggle,
                                                             &engineBox, &rootBox, &lengthBox, &banksToggle, &smartPickToggle, &reviewButton, &bankBox, &addQueueButton, &cardsButton,
                                                             &exportChordButton, &exportRowButton, &editStyleButton,
                                                             &varietyBox, &instancesBox, &size16, &size32, &size64, &moreButton,
                                                             &outButton, &planButton, &auditionButton, &auditionOctaveBox, &keyboard })
        c->setEnabled (! busy);
    generateButton.setButtonText (busy ? "Cancel" : "Generate");
    if (busy) progressBar.setTextToDisplay ({}); // percentage while rendering
    else updateEstimate();
}

void MainComponent::generateOrCancel()
{
    if (job != nullptr)
    {
        job->signalThreadShouldExit();
        if (queueRunning) { queueStopRequested = true; log ("Cancelling - the queue stops after this batch ..."); }
        else log ("Cancelling ...");
        return;
    }
    if (sweep != nullptr)
    {
        sweep->cancel();
        log ("Cancelling the preset sweep ...");
        return;
    }

    BatchRequest req;
    std::vector<SampleSpec> specs;
    if (! buildRequest (req, specs)) return;
    log (describePlan (req, specs).upToFirstOccurrenceOf ("\n", false, false).replace ("NEXT GENERATE:", "GENERATING:"));

    startJob (std::move (req), std::move (specs), true);
}

void MainComponent::makeExtraInstances (BatchRequest& req)
{
    // parallel VST3 copies (the built-in synth makes its own)
    extraInstances.clear();
    const int copies = queueRunning ? queueCopies : instancesBox.getSelectedId();
    if (dynamic_cast<BuiltInSynth*> (instrument.get()) == nullptr && instrumentPath.isNotEmpty() && copies > 1)
    {
        juce::MemoryBlock state;
        instrument->getStateInformation (state);
        for (int k = 1; k < copies; ++k)
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

}

void MainComponent::startJob (BatchRequest req, std::vector<SampleSpec> specs, bool openReviewAfter)
{
    keyboardState.allNotesOff (0);
    player.setProcessor (nullptr); // the job owns the instrument while it renders

    makeExtraInstances (req);

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
        if (safe->queueRunning) { safe->queueItemFinished (ok, summary, folder); return; }
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
    auto takes = takesBox.getSelectedId() - 1;
    auto seconds = count * perSample * (factor + regs + takes) / expectedSpeed();
    if (dynamic_cast<BuiltInSynth*> (instrument.get()) == nullptr) seconds += 1.5; // plugin warm-up

    estimateLabel.setText (seconds < 60 ? "~" + juce::String (juce::jmax (1, juce::roundToInt (seconds))) + " s"
                                        : "~" + juce::String (seconds / 60.0, 1) + " min", juce::dontSendNotification);
    if (job == nullptr && sweep == nullptr)
    {
        progressBar.setTextToDisplay (estimateLabel.getText());
        progressBar.setTooltip ("Estimated render time, learned from your previous batches with this instrument");
    }
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

// The chord grid (Boss DR-5 style): pick a root, pick a type, hear it - exactly as it will be
// exported. The 7 chord buttons above are shortcuts to the key's own chords.
PaletteChord MainComponent::currentGridChord() const
{
    PaletteChord c;
    c.rootPc = gridRoot;
    c.quality = gridQuality;
    c.octave = chordOctaveBox.getSelectedId() - 1;
    c.inversion = c.isNote() ? 0 : juce::jmax (0, inversionBox.getSelectedId() - 1);
    c.voicing = c.isNote() ? juce::String ("close") : voicingBox.getText().isEmpty() ? juce::String ("close") : voicingIdFromBox();
    if (auto* q = findQuality (c.quality)) c.inversion = juce::jmin (c.inversion, (int) q->intervals.size() - 1);
    return c;
}

juce::String MainComponent::voicingIdFromBox() const
{
    static const char* ids[] = { "close", "drop2", "open", "spread", "rootless", "shell" };
    return ids[juce::jlimit (0, 5, voicingBox.getSelectedId() - 1)];
}

SampleSpec MainComponent::specForChord (const PaletteChord& c, int number) const
{
    auto i = styleBox.getSelectedItemIndex();
    if (! juce::isPositiveAndBelow (i, styles.size())) return {};
    // planned exactly as Generate plans a palette: same sound, same notes, same processing
    PlanOptions o;
    o.usePalette = true;
    o.palette = { c };
    o.seed = seedEditor.getText().getLargeIntValue();          // the same timbre for preview and export
    o.engine = selectedEngine();
    o.shortNames = true;
    juce::String err;
    auto specs = planSamples (styles[i], o, err);
    if (specs.empty()) return {};
    auto s = specs.front();
    auto tag = styles[i].shortTag.isNotEmpty() ? styles[i].shortTag : styles[i].tag;
    s.stemHead = {};
    s.stemTail = tag + "_" + s.label + (c.isNote() ? juce::String() : "_" + s.voicing);
    s.index = number;
    s.fileStem = safeFileName (juce::String (number).paddedLeft ('0', 2) + "_" + s.stemTail);
    return s;
}

SampleSpec MainComponent::chordSpec (int index) const
{
    if (! juce::isPositiveAndBelow (index, (int) currentChords.size())) return {};
    return specForChord (diatonicAsPalette (currentChords[(size_t) index]), index + 1);
}

PaletteChord MainComponent::diatonicAsPalette (const DiatonicChord& d) const
{
    PaletteChord c = currentGridChord();
    c.rootPc = d.rootPc;
    c.quality = "maj";
    for (auto& q : allQualities())
        if (q.intervals == d.intervals) { c.quality = q.id; break; }
    if (findQuality (c.quality) != nullptr && findQuality (c.quality)->intervals != d.intervals)
        c.quality = d.intervals.size() > 1 && d.intervals[1] == 3 ? "min" : "maj";
    if (auto* q = findQuality (c.quality)) c.inversion = juce::jmin (c.inversion, (int) q->intervals.size() - 1);
    return c;
}

void MainComponent::updateExportInfo()
{
    auto c = currentGridChord();
    auto spec = specForChord (c, 1);
    if (spec.notes.empty()) { exportInfo.setText ({}, juce::dontSendNotification); return; }
    const int ps = spec.processing.pitchShift;
    juce::StringArray notes;
    for (auto& n : spec.notes) notes.add (midiNoteName (n.note + ps));
    exportInfo.setText ("SELECTED  " + c.name() + (c.isNote() ? juce::String() : "  " + voicingBox.getText() + ", " + inversionBox.getText().toLowerCase())
                        + "  =  " + notes.joinIntoString (" ") + "   (lowest note in Octave " + juce::String (c.octave) + "; C3 = middle C)"
                        + "      Palette: " + juce::String ((int) palette.size()), juce::dontSendNotification);
    exportInfo.setColour (juce::Label::textColourId, accent.brighter (0.4f));

    // the grid shows what is selected, and which roots / types fit the key
    const auto tonic = currentTonic();
    const auto scale = currentScale();
    const bool keyChosen = keyBox.getSelectedId() > 1;
    for (int r = 0; r < rootButtons.size(); ++r)
    {
        const bool inKey = std::find (scale.begin(), scale.end(), ((r - tonic) % 12 + 12) % 12) != scale.end();
        rootButtons[r]->setColour (juce::TextButton::buttonColourId,
                                   r == gridRoot ? accent.withAlpha (0.75f) : (keyChosen && inKey) ? panel.brighter (0.35f) : panel.brighter (0.08f));
    }
    for (int t = 0; t < typeButtons.size(); ++t)
    {
        const auto id = juce::String (gridTypes()[(size_t) t].first);
        bool fits = false;
        if (id == "note") fits = std::find (scale.begin(), scale.end(), ((gridRoot - tonic) % 12 + 12) % 12) != scale.end();
        else if (auto* q = findQuality (id)) fits = chordFitsScale (gridRoot, q->intervals, tonic, scale);
        typeButtons[t]->setColour (juce::TextButton::buttonColourId,
                                   id == gridQuality ? accent.withAlpha (0.75f) : (keyChosen && fits) ? panel.brighter (0.35f) : panel.brighter (0.08f));
    }
    juce::StringArray chips;
    for (auto& p : palette) chips.add (p.describe());
    paletteLabel.setText (palette.empty() ? juce::String ("Palette is empty: pick a root + type, then + Palette. Export Palette renders them; tick 'Generate uses palette' to make Generate render exactly these.")
                                          : "Palette (" + juce::String ((int) palette.size()) + "):  " + chips.joinIntoString ("  |  "),
                          juce::dontSendNotification);
}

void MainComponent::playGridChord()
{
    selectedChord = -1;
    for (size_t k = 0; k < currentChords.size(); ++k)
    {
        auto d = diatonicAsPalette (currentChords[k]);
        if (d.rootPc == gridRoot && d.quality == gridQuality) selectedChord = (int) k;
    }
    updateChordRow();
    auto c = currentGridChord();
    previewSpec (specForChord (c, 1), "Chord " + c.describe());
}

void MainComponent::playChord (int index)
{
    if (! juce::isPositiveAndBelow (index, (int) currentChords.size())) return;
    auto d = diatonicAsPalette (currentChords[(size_t) index]);
    gridRoot = d.rootPc;
    gridQuality = d.quality;
    playGridChord();
}

/** Plays a sample: rendered exactly as Generate / Export renders it (recipe FX, trim, envelope),
    or - with "Hear FX" off - played live on the instrument (instant, but without the recipe's processing). */
void MainComponent::previewSpec (SampleSpec spec, const juce::String& what)
{
    if (spec.notes.empty() || job != nullptr || sweep != nullptr || instrument == nullptr) return;
    const int ps = spec.processing.pitchShift;
    juce::StringArray notes;
    for (auto& n : spec.notes) notes.add (midiNoteName (n.note + ps));

    if (! exactPreviewToggle.getToggleState())
    {
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
            auto on = juce::MidiMessage::noteOn (1, n.note + ps, (juce::uint8) n.velocity);
            on.setTimeStamp (now + n.startSec);
            collector.addMessageToQueue (on);
            juce::Timer::callAfterDelay ((int) ((n.startSec + n.lengthSec) * 1000.0),
                                         [safe = juce::Component::SafePointer<MainComponent> (this), note = n.note + ps]
                                         {
                                             if (safe == nullptr) return;
                                             auto off = juce::MidiMessage::noteOff (1, note);
                                             off.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001);
                                             safe->player.getMidiMessageCollector().addMessageToQueue (off);
                                         });
        }
        log ("> " + what + "  =  " + notes.joinIntoString (" ") + "   (live, without the recipe's FX)");
        return;
    }

    // render it offline, exactly like a batch, then play the result
    auto* device = deviceManager.getCurrentAudioDevice();
    RenderSettings rs;
    rs.sampleRate = device != nullptr && device->getCurrentSampleRate() > 0 ? device->getCurrentSampleRate() : 44100.0;
    rs.blockSize = 512;
    rs.warmUpSec = 0;
    rs.maxTailSec = 2.0;
    rs.trimToSec = lengthBox.getSelectedId() == 1 ? -1.0 : lengthBox.getSelectedId() == 2 ? 0.0 : lengthBox.getSelectedId() / 1000.0;
    player.setProcessor (nullptr);
    if (auto* b = dynamic_cast<BuiltInSynth*> (instrument.get()))
    {
        b->reset();
        Engine r;
        if (parseEngine (spec.engine, r)) b->setEngine (r);
        b->setVariation (spec.variation);
    }
    juce::AudioBuffer<float> audio;
    juce::String error;
    bool ok;
    const auto started = juce::Time::getMillisecondCounterHiRes();
    {
        OfflineRenderer renderer (*instrument, rs);
        ok = renderer.render (spec, audio, error);
    }
    player.setProcessor (instrument.get());
    applyLiveEngine();
    if (! ok) { log ("Preview failed: " + error); return; }

    previewPlayer.setSource (nullptr);
    previewSource = std::make_unique<juce::MemoryAudioSource> (audio, true, false);
    previewPlayer.setSource (previewSource.get());
    log ("> " + what + "  =  " + notes.joinIntoString (" ") + "   (exactly as it renders: " + juce::String (juce::roundToInt (audio.getNumSamples() * 1000.0 / rs.sampleRate))
         + " ms with the recipe's FX" + (device == nullptr ? ", no audio output" : "") + ", rendered in "
         + juce::String (juce::roundToInt (juce::Time::getMillisecondCounterHiRes() - started)) + " ms)");
}

void MainComponent::addToPalette()
{
    auto c = currentGridChord();
    for (auto& p : palette)
        if (p.toString() == c.toString()) { log ("Already in the palette: " + c.describe()); return; }
    palette.push_back (c);
    savePalette();
    log ("+ Palette: " + c.describe() + "   (" + juce::String ((int) palette.size()) + ")");
}

void MainComponent::savePalette()
{
    juce::StringArray items;
    for (auto& p : palette) items.add (p.toString());
    props.getUserSettings()->setValue ("palette", items.joinIntoString (";"));
    updateExportInfo();
}

std::vector<SampleSpec> MainComponent::paletteSpecs() const
{
    std::vector<SampleSpec> specs;
    for (size_t k = 0; k < palette.size(); ++k) specs.push_back (specForChord (palette[k], (int) k + 1));
    return specs;
}

void MainComponent::exportChords (const std::vector<int>& indices)
{
    std::vector<SampleSpec> specs;
    for (auto idx : indices) if (juce::isPositiveAndBelow (idx, (int) currentChords.size())) specs.push_back (chordSpec (idx));
    exportSpecs (std::move (specs), "the key's chords");
}

void MainComponent::exportSpecs (std::vector<SampleSpec> specs, const juce::String& what)
{
    if (job != nullptr || sweep != nullptr || instrument == nullptr) return;
    auto i = styleBox.getSelectedItemIndex();
    if (! juce::isPositiveAndBelow (i, styles.size())) return;
    specs.erase (std::remove_if (specs.begin(), specs.end(), [] (const SampleSpec& s) { return s.notes.empty(); }), specs.end());
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
    req.render.trimToSec = lengthBox.getSelectedId() == 1 ? -1.0 : lengthBox.getSelectedId() == 2 ? 0.0 : lengthBox.getSelectedId() / 1000.0;
    req.roundRobins = juce::jlimit (1, 4, takesBox.getSelectedId());
    auto scaleName = scaleBox.getSelectedId() > 1 ? scaleBox.getText() : req.style.scale;
    req.outputFolder = outputFolder.getChildFile ("Chord Picks")
                           .getChildFile (safeFileName (req.style.name) + "_" + pitchClassName (currentTonic()) + "_" + scaleName);
    {
        juce::StringArray list;
        for (auto& sp : specs)
        {
            juce::StringArray notes;
            for (auto& n : sp.notes) notes.add (midiNoteName (n.note + sp.processing.pitchShift));
            list.add (sp.label + " [" + notes.joinIntoString (" ") + "]");
        }
        log ("Exporting " + what + " (" + juce::String ((int) specs.size()) + "): " + list.joinIntoString (", "));
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

void MainComponent::openPresetSweep()
{
    if (instrument == nullptr || job != nullptr || sweep != nullptr) return;
    auto i = styleBox.getSelectedItemIndex();
    if (! juce::isPositiveAndBelow (i, styles.size())) { log ("Pick a recipe first."); return; }

    PresetSweepPanel::Info info;
    info.instrumentName = instrument->getName();
    info.recipeName = styles[i].name;
    info.builtIn = dynamic_cast<BuiltInSynth*> (instrument.get()) != nullptr;
    info.numPrograms = instrument->getNumPrograms();
    for (auto& folder : standardPresetFolders (info.instrumentName))
        info.standardFolderPresets += (int) folder.findChildFiles (juce::File::findFiles, true, "*.vstpreset;*.forgesound").size();

    auto* panel = new PresetSweepPanel (info, props.getUserSettings());
    auto safe = juce::Component::SafePointer<MainComponent> (this);
    panel->onStart = [safe] (const SweepOptions& o)
    {
        if (safe == nullptr) return;
        juce::MessageManager::callAsync ([safe, o] { if (safe != nullptr) { safe->sweepWindow.reset(); safe->startSweep (o); } });
    };
    sweepWindow = std::make_unique<ToolWindow> ("Preset Sweep", panel, [safe] { if (safe != nullptr) safe->sweepWindow.reset(); });
}

void MainComponent::startSweep (const SweepOptions& options)
{
    BatchRequest req;
    std::vector<SampleSpec> specs;
    if (! buildRequest (req, specs)) return;
    req.outputFolder = outputFolder.getChildFile (safeFileName (req.style.name + "_Sweep_" + instrument->getName()))
                           .getNonexistentSibling (false);

    keyboardState.allNotesOff (0);
    player.setProcessor (nullptr); // the sweep owns the instrument while it renders
    stateBeforeSweep.reset();
    if (dynamic_cast<BuiltInSynth*> (instrument.get()) == nullptr) instrument->getStateInformation (stateBeforeSweep);
    makeExtraInstances (req);
    setBusy (true);
    progress = 0;

    auto safe = juce::Component::SafePointer<MainComponent> (this);
    sweep = std::make_unique<PresetSweep> (*instrument, req.extraInstances, req, options);
    sweep->onLog = [safe] (const juce::String& m) { if (safe != nullptr) safe->log (m); };
    sweep->onProgress = [safe] (double p) { if (safe != nullptr) safe->progress = p; };
    sweep->onFinished = [safe] (bool ok, const juce::String& summary, const juce::File& folder)
    {
        // finish outside the sweep's own call stack: it is deleted here
        juce::MessageManager::callAsync ([safe, ok, summary, folder] { if (safe != nullptr) safe->sweepFinished (ok, summary, folder); });
    };
    juce::String error;
    if (! sweep->start (error))
    {
        log ("Preset sweep: " + error);
        sweepFinished (false, "Nothing to sweep.", {});
    }
}

void MainComponent::sweepFinished (bool ok, const juce::String& summary, const juce::File& folder)
{
    juce::File best;
    if (sweep != nullptr)
        for (auto& r : sweep->results())
            if (r.kept && r.folderName.startsWith ("01_")) best = folder.getChildFile (r.folderName);
    sweep.reset();
    extraInstances.clear();
    if (stateBeforeSweep.getSize() > 0 && instrument != nullptr)
        instrument->setStateInformation (stateBeforeSweep.getData(), (int) stateBeforeSweep.getSize()); // back to your sound
    jobFinished (ok, summary);
    if (folder.isDirectory())
    {
        log ("Every kept preset has its own folder in " + folder.getFullPathName() + " - see _Preset_Sweep.txt for the ranking.");
        if (best.isDirectory()) { lastBatchFolder = best; openReview (best); }
    }
}

//==============================================================================
// Render queue

void MainComponent::saveQueueFile()
{
    saveQueue (defaultQueueFile(), queue);
    updateQueueButton();
    if (queueWindow != nullptr)
        if (auto* p = dynamic_cast<QueuePanel*> (queueWindow->getContentComponent())) p->refresh();
}

void MainComponent::updateQueueButton()
{
    int waiting = 0;
    for (auto& q : queue) waiting += q.status == "waiting" || q.status == "running";
    queueButton.setButtonText (waiting > 0 ? "Queue (" + juce::String (waiting) + ")" : juce::String ("Queue"));
    queueButton.setColour (juce::TextButton::buttonColourId, queueRunning ? accent : panel.brighter (0.1f));
}

void MainComponent::addToQueue()
{
    if (job != nullptr || sweep != nullptr) return;
    BatchRequest req;
    std::vector<SampleSpec> specs;
    if (! buildRequest (req, specs)) return;

    QueueItem item;
    item.request = req;
    item.request.smartPickTarget = 0;            // planned again when it runs
    item.smartPick = smartPickToggle.getToggleState();
    item.pluginPath = dynamic_cast<BuiltInSynth*> (instrument.get()) != nullptr ? juce::String() : instrumentPath;
    if (item.pluginPath.isNotEmpty()) instrument->getStateInformation (item.pluginState);
    item.instances = instancesBox.getSelectedId();
    queue.push_back (item);
    saveQueueFile();

    int waiting = 0;
    for (auto& q : queue) waiting += q.status == "waiting";
    log ("+ Queued #" + juce::String ((int) queue.size()) + ": " + item.describe() + "   (" + juce::String (waiting) + " waiting - open Queue to start)");
    seedEditor.setText (juce::String (juce::Random::getSystemRandom().nextInt (999999))); // the next one is different
}

void MainComponent::openQueue()
{
    if (queueWindow != nullptr) { queueWindow->toFront (true); return; }
    QueuePanel::Hooks hooks;
    hooks.items = [this]() -> std::vector<QueueItem>& { return queue; };
    hooks.running = [this] { return queueRunning; };
    hooks.start = [this] { startQueue(); };
    hooks.stop = [this] { stopQueue(); };
    hooks.changed = [this] { saveQueueFile(); };
    hooks.review = [this] (const juce::File& f) { openReview (f); };
    auto safe = juce::Component::SafePointer<MainComponent> (this);
    queueWindow = std::make_unique<ToolWindow> ("Render Queue", new QueuePanel (hooks), [safe] { if (safe != nullptr) safe->queueWindow.reset(); });
}

void MainComponent::startQueue()
{
    if (queueRunning || job != nullptr || sweep != nullptr) return;
    queueRunning = true;
    queueStopRequested = false;
    queueRunNumber = 0;
    queueRunTotal = 0;
    for (auto& q : queue) queueRunTotal += q.status == "waiting";
    queueStartPath = dynamic_cast<BuiltInSynth*> (instrument.get()) != nullptr ? juce::String() : instrumentPath;
    queueStartState.reset();
    if (queueStartPath.isNotEmpty() && instrument != nullptr) instrument->getStateInformation (queueStartState);
    log ("==== Render queue started");
    updateQueueButton();
    runNextQueued();
}

void MainComponent::stopQueue()
{
    if (! queueRunning) return;
    queueStopRequested = true;
    if (job != nullptr) job->signalThreadShouldExit();
    log ("Stopping the queue ...");
}

void MainComponent::runNextQueued()
{
    queueIndex = -1;
    for (size_t i = 0; i < queue.size(); ++i)
        if (queue[i].status == "waiting") { queueIndex = (int) i; break; }

    if (queueIndex < 0 || queueStopRequested)
    {
        // finished: back to the instrument and sound you had
        queueRunning = false;
        if (queueStartPath.isEmpty()) { if (dynamic_cast<BuiltInSynth*> (instrument.get()) == nullptr) useBuiltIn(); }
        else if (instrumentPath != queueStartPath) loadInstrument (queueStartPath);
        if (queueStartPath.isNotEmpty() && instrument != nullptr && queueStartState.getSize() > 0 && instrumentPath == queueStartPath)
            instrument->setStateInformation (queueStartState.getData(), (int) queueStartState.getSize());
        int done = 0, failed = 0;
        for (auto& q : queue) { done += q.status == "done"; failed += q.status == "failed"; }
        log ("==== Render queue " + juce::String (queueStopRequested ? "stopped" : "finished") + ": " + juce::String (done) + " done"
             + (failed > 0 ? ", " + juce::String (failed) + " failed" : juce::String()) + ". Output: " + outputFolder.getFullPathName());
        saveQueueFile();
        return;
    }

    auto& item = queue[(size_t) queueIndex];
    item.status = "running";
    saveQueueFile();
    // (the safe pointer is made out here: MSVC reads `this` inside a nested init-capture as the lambda)
    auto failItem = [this, safe = juce::Component::SafePointer<MainComponent> (this)] (const juce::String& why)
    {
        auto& it = queue[(size_t) queueIndex];
        it.status = "failed";
        it.result = why;
        log ("Queue: " + why);
        saveQueueFile();
        juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->runNextQueued(); });
    };

    // the instrument and sound it was queued with
    if (item.pluginPath.isEmpty())
    {
        if (dynamic_cast<BuiltInSynth*> (instrument.get()) == nullptr) useBuiltIn();
    }
    else
    {
        if (instrumentPath != item.pluginPath) loadInstrument (item.pluginPath);
        if (instrumentPath != item.pluginPath || instrument == nullptr) { failItem ("could not load " + item.pluginPath); return; }
        if (item.pluginState.getSize() > 0) instrument->setStateInformation (item.pluginState.getData(), (int) item.pluginState.getSize());
    }

    BatchRequest req = item.request;
    req.instrumentName = instrument->getName();
    req.plan.builtIn = dynamic_cast<BuiltInSynth*> (instrument.get()) != nullptr;
    req.outputFolder = req.outputFolder.getNonexistentSibling (false);
    std::vector<SampleSpec> specs;
    juce::StringArray notes;
    if (! planBatch (req, item.smartPick, specs, notes)) { failItem ("nothing planned: " + notes.joinIntoString ("; ")); return; }
    for (auto& n : notes) log (n);

    ++queueRunNumber;
    log ("==== Queue " + juce::String (queueRunNumber) + "/" + juce::String (queueRunTotal) + ": " + item.describe());
    queueCopies = item.instances;
    item.doneFolder = req.outputFolder;
    if (req.plan.useCard) writeCardMidiForPlan (req.plan, req.outputFolder);
    startJob (std::move (req), std::move (specs), false);
}

void MainComponent::queueItemFinished (bool ok, const juce::String& summary, const juce::File& folder)
{
    if (juce::isPositiveAndBelow (queueIndex, (int) queue.size()))
    {
        auto& it = queue[(size_t) queueIndex];
        it.status = queueStopRequested && ! ok ? "cancelled" : ok ? "done" : "failed";
        it.result = summary.upToFirstOccurrenceOf ("\n", false, false);
        it.doneFolder = folder;
        if (it.status == "cancelled") it.status = "waiting"; // stopped part-way: it will run again next time
    }
    saveQueueFile();
    juce::MessageManager::callAsync ([safe = juce::Component::SafePointer<MainComponent> (this)] { if (safe != nullptr) safe->runNextQueued(); });
}

//==============================================================================
// Progressions

void MainComponent::openProgressions()
{
    if (progressionsWindow != nullptr) { progressionsWindow->toFront (true); return; }
    ProgressionPanel::Hooks hooks;
    hooks.palette = [this] { return palette; };
    hooks.play = [this] (const juce::AudioBuffer<float>& audio, double)
    {
        auto copy = audio;
        previewPlayer.setSource (nullptr);
        previewSource = std::make_unique<juce::MemoryAudioSource> (copy, true, false);
        previewPlayer.setSource (previewSource.get());
    };
    hooks.stop = [this] { previewPlayer.setSource (nullptr); };
    hooks.outputFolder = [this] { return outputFolder; };
    hooks.sampleRate = [this]
    {
        auto* device = deviceManager.getCurrentAudioDevice();
        return device != nullptr && device->getCurrentSampleRate() > 0 ? device->getCurrentSampleRate() : 44100.0;
    };
    hooks.exportRate = [this] { return (double) rateBox.getSelectedId(); };
    hooks.bitDepth = [this] { return bitsBox.getSelectedId(); };
    hooks.log = [this] (const juce::String& m) { log (m); };
    auto safe = juce::Component::SafePointer<MainComponent> (this);
    progressionsWindow = std::make_unique<ToolWindow> ("Progressions", new ProgressionPanel (hooks, props.getUserSettings()),
                                                       [safe] { if (safe != nullptr) safe->progressionsWindow.reset(); });
}

//==============================================================================
// Recipe cards

bool MainComponent::buildCardRequest (const RecipeCardPanel::Options& o, int cardIndex, BatchRequest& req, std::vector<SampleSpec>& specs)
{
    if (! buildRequest (req, specs, true)) return false;   // format, length, takes ... from the main window
    req.plan.usePalette = false;
    req.plan.useCard = true;
    req.plan.cardText = o.text;
    req.plan.cardIndex = cardIndex;
    req.plan.cardMpcNotation = o.mpcNotation;
    req.plan.cardUseRecipeFx = o.useRecipeFx;
    req.plan.engine = o.useMainEngine ? selectedEngine() : juce::String();
    req.plan.octaveLow = req.plan.octaveHigh = -1;
    req.registers.clear();
    auto cards = parseRecipeCards (o.text, o.mpcNotation);
    if (! juce::isPositiveAndBelow (cardIndex, (int) cards.size())) return false;
    req.outputFolder = outputFolder.getChildFile ("Recipe Cards").getChildFile (safeFileName (cards[(size_t) cardIndex].code))
                           .getNonexistentSibling (false);
    juce::StringArray notes;
    return planBatch (req, false, specs, notes);
}

void MainComponent::openRecipeCards()
{
    if (cardsWindow != nullptr) { cardsWindow->toFront (true); return; }
    if (instrument == nullptr) return;
    auto* panel = new RecipeCardPanel (instrument->getName(), dynamic_cast<BuiltInSynth*> (instrument.get()) != nullptr, props.getUserSettings());
    auto safe = juce::Component::SafePointer<MainComponent> (this);
    panel->onRender = [safe] (const RecipeCardPanel::Options& opt)
    {
        if (safe == nullptr || safe->job != nullptr || safe->sweep != nullptr) return;
        const auto& o = opt;
        BatchRequest req;
        std::vector<SampleSpec> specs;
        if (! safe->buildCardRequest (o, opt.cardIndex, req, specs)) { safe->log ("Recipe card: nothing to render."); return; }
        auto midi = writeCardMidiForPlan (req.plan, req.outputFolder);
        safe->log ("RECIPE CARD: " + juce::String ((int) specs.size()) + " files" + (midi.isEmpty() ? juce::String() : " + " + midi.joinIntoString (", "))
                   + " -> " + req.outputFolder.getFullPathName());
        safe->startJob (std::move (req), std::move (specs), true);
    };
    panel->onQueueAll = [safe] (const RecipeCardPanel::Options& opt, int count)
    {
        if (safe == nullptr || safe->job != nullptr || safe->sweep != nullptr) return;
        const auto& o = opt;
        int added = 0;
        for (int k = 0; k < count; ++k)
        {
            BatchRequest req;
            std::vector<SampleSpec> specs;
            if (! safe->buildCardRequest (o, k, req, specs)) continue;
            QueueItem item;
            item.request = req;
            item.request.smartPickTarget = 0;
            auto* inst = safe->instrument.get();
            item.pluginPath = dynamic_cast<BuiltInSynth*> (inst) != nullptr ? juce::String() : safe->instrumentPath;
            if (item.pluginPath.isNotEmpty()) inst->getStateInformation (item.pluginState);
            item.instances = safe->instancesBox.getSelectedId();
            safe->queue.push_back (item);
            ++added;
        }
        safe->saveQueueFile();
        safe->log ("Queued " + juce::String (added) + " recipe cards - starting the queue.");
        safe->openQueue();
        safe->startQueue();
    };
    cardsWindow = std::make_unique<ToolWindow> ("Recipe Cards", panel, [safe] { if (safe != nullptr) safe->cardsWindow.reset(); });
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
