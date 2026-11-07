#include "ProgressionPanel.h"

namespace forge
{
namespace
{
    const juce::Colour bg (0xff16181d), panel (0xff20232a), accent (0xffe8483b), text (0xffe6e6e6), dim (0xff9aa0a6);
    const char* partIds[] = { "keys", "bass", "pad", "lead", "drums" };
    const char* partNames[] = { "Keys", "Bass", "Pad / Strings", "Lead", "Drums" };
    const char* chordTypes[] = { "maj7", "m7", "7", "maj9", "m9", "9", "m11", "13", "6", "m6", "69", "add9", "maj", "min",
                                 "sus2", "sus4", "7sus4", "9sus4", "7s9", "7b9", "m7b5", "dim7", "aug", "maj7s11", "5" };
}

//==============================================================================
void BlockLane::setBlocks (const std::vector<ChordBlock>* b, int t, int s)
{
    blocks = b; tonic = t; selected = s;
    repaint();
}

juce::Rectangle<float> BlockLane::blockBounds (int index) const
{
    if (blocks == nullptr || blocks->empty()) return {};
    double total = 0;
    for (auto& b : *blocks) total += b.beats;
    total = std::max (total, 16.0);
    const auto w = (double) getWidth() - 4;
    double x = 2;
    for (int i = 0; i < index; ++i) x += (*blocks)[(size_t) i].beats / total * w;
    return { (float) x, 2.0f, (float) ((*blocks)[(size_t) index].beats / total * w) - 3.0f, (float) getHeight() - 4.0f };
}

int BlockLane::blockAt (float x) const
{
    if (blocks == nullptr) return -1;
    for (int i = 0; i < (int) blocks->size(); ++i)
        if (blockBounds (i).getX() <= x && x <= blockBounds (i).getRight() + 3) return i;
    return -1;
}

int BlockLane::dropIndexAt (float x) const
{
    if (blocks == nullptr) return -1;
    for (int i = 0; i < (int) blocks->size(); ++i)
        if (x < blockBounds (i).getCentreX()) return i;
    return (int) blocks->size();
}

void BlockLane::paint (juce::Graphics& g)
{
    g.fillAll (panel);
    if (blocks == nullptr || blocks->empty())
    {
        g.setColour (dim);
        g.drawText ("No chords yet: Generate Progression, or add chords below.", getLocalBounds(), juce::Justification::centred);
        return;
    }
    double beat = 0;
    for (int i = 0; i < (int) blocks->size(); ++i)
    {
        auto r = blockBounds (i);
        const auto& b = (*blocks)[(size_t) i];
        const bool sel = i == selected;
        auto colour = juce::Colour::fromHSV ((float) (((b.rootPc - tonic + 12) % 12) / 12.0), 0.35f, sel ? 0.55f : 0.35f, 1.0f);
        g.setColour (i == dragging ? colour.withAlpha (0.4f) : colour);
        g.fillRoundedRectangle (r, 5.0f);
        g.setColour (sel ? accent : colour.brighter (0.4f));
        g.drawRoundedRectangle (r, 5.0f, sel ? 2.0f : 1.0f);
        g.setColour (text);
        const auto nameArea = juce::Rectangle<float> (r.getX() + 3, r.getY() + 16, r.getWidth() - 6, r.getHeight() * 0.45f);
        const auto infoArea = juce::Rectangle<float> (r.getX() + 3, nameArea.getBottom() + 2, r.getWidth() - 6, 16.0f);
        g.setFont (juce::FontOptions (r.getWidth() > 70 ? 18.0f : 13.0f, juce::Font::bold));
        g.drawFittedText (b.name(), nameArea.toNearestInt(), juce::Justification::centred, 1);
        g.setFont (juce::FontOptions (12.0f));
        g.setColour (text.withAlpha (0.75f));
        g.drawFittedText (b.roman (tonic) + "  -  " + juce::String (b.beats, b.beats == std::floor (b.beats) ? 0 : 1) + (b.beats == 1 ? " beat" : " beats"),
                          infoArea.toNearestInt(), juce::Justification::centred, 1);
        g.setColour (dim);
        g.setFont (juce::FontOptions (10.0f));
        if (std::fmod (beat, 4.0) < 1e-6) g.drawText ("bar " + juce::String ((int) (beat / 4) + 1), (int) r.getX() + 4, (int) r.getY() + 2, 60, 12, juce::Justification::left);
        beat += b.beats;
    }
    if (dragging >= 0 && dropAt >= 0)
    {
        auto x = dropAt < (int) blocks->size() ? blockBounds (dropAt).getX() - 2 : blockBounds ((int) blocks->size() - 1).getRight() + 1;
        g.setColour (accent);
        g.fillRect (x - 1.5f, 0.0f, 3.0f, (float) getHeight());
    }
}

void BlockLane::mouseDown (const juce::MouseEvent& e)
{
    auto i = blockAt ((float) e.x);
    if (i >= 0 && onSelect) onSelect (i);
    dragging = -1;
}

void BlockLane::mouseDrag (const juce::MouseEvent& e)
{
    if (e.getDistanceFromDragStart() < 6) return;
    if (dragging < 0) dragging = blockAt ((float) e.getMouseDownX());
    dropAt = dropIndexAt ((float) e.x);
    repaint();
}

void BlockLane::mouseUp (const juce::MouseEvent&)
{
    if (dragging >= 0 && dropAt >= 0 && onMove)
    {
        auto to = dropAt > dragging ? dropAt - 1 : dropAt;
        if (to != dragging) onMove (dragging, to);
    }
    dragging = dropAt = -1;
    repaint();
}

void BlockLane::mouseDoubleClick (const juce::MouseEvent& e)
{
    auto i = blockAt ((float) e.x);
    if (i >= 0 && onPlay) onPlay (i);
}

//==============================================================================
ProgressionPanel::ProgressionPanel (Hooks h, juce::PropertiesFile* s) : hooks (std::move (h)), settings (s)
{
    auto label = [this] (juce::Label& l, const juce::String& t)
    {
        l.setText (t, juce::dontSendNotification);
        l.setColour (juce::Label::textColourId, dim);
        addAndMakeVisible (l);
    };
    title.setText ("Progressions", juce::dontSendNotification);
    title.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    title.setColour (juce::Label::textColourId, accent);
    addAndMakeVisible (title);
    label (presetDesc, {});
    presetDesc.setJustificationType (juce::Justification::topLeft);
    presetDesc.setMinimumHorizontalScale (1.0f);

    for (size_t k = 0; k < songPresets().size(); ++k) presetBox.addItem (songPresets()[k].name, (int) k + 1);
    {
        int saved = 1;
        for (size_t k = 0; k < songPresets().size(); ++k)
            if (settings != nullptr && songPresets()[k].id == settings->getValue ("songPreset")) saved = (int) k + 1;
        presetBox.setSelectedId (saved, juce::dontSendNotification);
    }
    presetBox.setTooltip ("Genre preset: how Generate Progression writes chords, and how the keys, bass, pad, lead and drums play them");
    presetBox.onChange = [this] { presetChanged (true); };

    keyBox.addItem ("Key: preset's", 1);
    for (int pc = 0; pc < 12; ++pc) keyBox.addItem ("Key: " + pitchClassName (pc), pc + 2);
    keyBox.setSelectedId (1, juce::dontSendNotification);
    keyBox.onChange = [this]
    {
        if (keyBox.getSelectedId() <= 1) return;
        const int tonic = keyBox.getSelectedId() - 2, shift = tonic - arr.tonic;
        for (auto& b : arr.blocks) b.rootPc = ((b.rootPc + shift) % 12 + 12) % 12;   // move the progression to the new key
        arr.tonic = tonic;
        blocksChanged();
    };
    for (auto b : { 4, 8, 16 }) barsBox.addItem (juce::String (b) + " bars", b);
    barsBox.setSelectedId (8, juce::dontSendNotification);

    label (tempoLabel, "Tempo");
    tempoSlider.setRange (50, 140, 1);
    tempoSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 44, 22);
    tempoSlider.setColour (juce::Slider::trackColourId, accent);
    tempoSlider.onDragEnd = [this] { arr.tempo = tempoSlider.getValue(); blocksChanged(); };
    tempoSlider.onValueChange = [this] { if (! tempoSlider.isMouseButtonDown()) { arr.tempo = tempoSlider.getValue(); blocksChanged(); } };
    label (swingLabel, "Swing");
    swingSlider.setRange (0, 1, 0.05);
    swingSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 40, 22);
    swingSlider.setColour (juce::Slider::trackColourId, accent);
    swingSlider.onValueChange = [this] { if (! swingSlider.isMouseButtonDown()) { arr.swing = swingSlider.getValue(); blocksChanged(); } };
    swingSlider.onDragEnd = [this] { arr.swing = swingSlider.getValue(); blocksChanged(); };

    seedEditor.setText (juce::String (juce::Random::getSystemRandom().nextInt (999999)), false);
    seedEditor.setTooltip ("The same seed writes the same progression and parts");
    diceButton.onClick = [this] { seedEditor.setText (juce::String (juce::Random::getSystemRandom().nextInt (999999)), false); generate(); };
    generateButton.setColour (juce::TextButton::buttonColourId, accent);
    generateButton.setTooltip ("Write a progression in the preset's style (and keys, bass, pad, lead and drums for it)");
    generateButton.onClick = [this] { generate(); };

    lane.onSelect = [this] (int i) { selectBlock (i); };
    lane.onMove = [this] (int from, int to)
    {
        auto b = arr.blocks[(size_t) from];
        arr.blocks.erase (arr.blocks.begin() + from);
        arr.blocks.insert (arr.blocks.begin() + to, b);
        selected = to;
        blocksChanged();
    };
    lane.onPlay = [this] (int i)
    {
        Arrangement one = arr;
        one.blocks = { arr.blocks[(size_t) i] };
        generateParts (one, preset());
        for (auto& p : one.parts) if (p.id == "lead") p.enabled = false;
        juce::AudioBuffer<float> mix;
        std::vector<juce::AudioBuffer<float>> stems;
        juce::String err;
        auto sr = hooks.sampleRate ? hooks.sampleRate() : 44100.0;
        if (renderArrangement (one, sr, mix, stems, err) && hooks.play) hooks.play (mix, sr);
    };

    label (selectedLabel, "Select a block");
    leftButton.setTooltip ("Move the selected chord left");
    rightButton.setTooltip ("Move the selected chord right");
    leftButton.onClick = [this] { if (selected > 0) { std::swap (arr.blocks[(size_t) selected], arr.blocks[(size_t) selected - 1]); --selected; blocksChanged(); } };
    rightButton.onClick = [this] { if (selected >= 0 && selected + 1 < (int) arr.blocks.size()) { std::swap (arr.blocks[(size_t) selected], arr.blocks[(size_t) selected + 1]); ++selected; blocksChanged(); } };
    for (auto beats : { 1, 2, 3, 4, 6, 8, 16 }) lengthBox.addItem (juce::String (beats) + (beats == 1 ? " beat" : " beats") + (beats == 4 ? " (1 bar)" : beats == 8 ? " (2 bars)" : ""), beats);
    lengthBox.onChange = [this] { if (selected >= 0) { arr.blocks[(size_t) selected].beats = lengthBox.getSelectedId(); blocksChanged(); } };
    dupButton.onClick = [this] { if (selected >= 0) { arr.blocks.insert (arr.blocks.begin() + selected + 1, arr.blocks[(size_t) selected]); ++selected; blocksChanged(); } };
    delButton.onClick = [this] { if (selected >= 0) { arr.blocks.erase (arr.blocks.begin() + selected); selected = std::min (selected, (int) arr.blocks.size() - 1); blocksChanged(); } };
    clearButton.onClick = [this] { arr.blocks.clear(); selected = -1; blocksChanged(); };

    label (addLabel, "Add");
    for (int pc = 0; pc < 12; ++pc) rootBox.addItem (pitchClassName (pc), pc + 1);
    rootBox.setSelectedId (1, juce::dontSendNotification);
    for (int k = 0; k < (int) (sizeof (chordTypes) / sizeof (chordTypes[0])); ++k)
    {
        auto* q = findQuality (chordTypes[k]);
        typeBox.addItem (q != nullptr ? (q->suffix.isEmpty() ? juce::String ("maj") : q->suffix == "power" ? juce::String ("5") : q->suffix) : juce::String (chordTypes[k]), k + 1);
    }
    typeBox.setSelectedId (5, juce::dontSendNotification);
    addButton.setTooltip ("Add this chord after the selected block (or at the end)");
    addButton.onClick = [this]
    {
        ChordBlock b;
        b.rootPc = rootBox.getSelectedId() - 1;
        b.quality = chordTypes[juce::jlimit (0, (int) (sizeof (chordTypes) / sizeof (chordTypes[0])) - 1, typeBox.getSelectedId() - 1)];
        addBlock (b);
    };
    addPaletteButton.setTooltip ("Add every chord of the main window's palette (the chord grid's picks)");
    addPaletteButton.onClick = [this]
    {
        if (! hooks.palette) return;
        for (auto& p : hooks.palette())
            if (! p.isNote()) { ChordBlock b; b.rootPc = p.rootPc; b.quality = p.quality; addBlock (b); }
    };
    for (int k = 0; k < 7; ++k)
    {
        auto* b = keyChordButtons.add (new juce::TextButton());
        b->setTooltip ("Add the key's chord on this degree");
        b->onClick = [this, k] { auto kc = keyChords(); if (k < (int) kc.size()) addBlock (kc[(size_t) k]); };
        addAndMakeVisible (b);
    }

    label (typedLabel, "Type it");
    typedEditor.setTextToShowWhenEmpty ("ii:m9 V:13 I:maj9 vi:m9   or   Dm9 G13 Cmaj9 Am9   (*2 = 2 beats)", dim);
    setTypedButton.onClick = [this]
    {
        std::vector<ChordBlock> b;
        juce::String err;
        if (parseProgression (typedEditor.getText(), arr.tonic, b, err)) { arr.blocks = b; selected = -1; blocksChanged(); }
        else status.setText ("Can't read that: " + err, juce::dontSendNotification);
    };

    label (tracksLabel, "Parts (MIDI tracks) - each plays with a recipe's sound in the loop");
    juce::StringArray recipeIds, recipeNames;
    for (auto& st : builtInStyles()) if (! st.isKit() && st.progressionLibrary.isEmpty()) { recipeIds.add (st.id); recipeNames.add (st.name); }
    for (int p = 0; p < 5; ++p)
    {
        auto* t = partToggles.add (new juce::ToggleButton (partNames[p]));
        t->setToggleState (true, juce::dontSendNotification);
        t->onClick = [this, p]
        {
            for (auto& part : arr.parts) if (part.id == partIds[p]) part.enabled = partToggles[p]->getToggleState();
            blocksChanged();
        };
        addAndMakeVisible (t);
        auto* c = partRecipes.add (new juce::ComboBox());
        if (p == 4) { c->addItem ("GM drums (MIDI only - loops stay drumless for sampling)", 1); c->setSelectedId (1, juce::dontSendNotification); c->setEnabled (false); }
        else
        {
            for (int k = 0; k < recipeIds.size(); ++k) c->addItem (recipeNames[k], k + 1);
            c->onChange = [this, p, c, recipeIds]
            {
                auto id = recipeIds[c->getSelectedId() - 1];
                for (auto& part : arr.parts) if (part.id == partIds[p]) part.recipeId = id;
                blocksChanged();
            };
        }
        c->getProperties().set ("ids", recipeIds.joinIntoString (" "));
        addAndMakeVisible (c);
    }
    newPartsButton.setTooltip ("Keep the chords, write new keys / bass / lead / drum parts (new seed)");
    newPartsButton.onClick = [this] { arr.seed = juce::Random::getSystemRandom().nextInt (999999); blocksChanged(); };

    playButton.onClick = [this] { playLoop(); };
    stopButton.onClick = [this] { if (hooks.stop) hooks.stop(); };
    midiButton.onClick = [this] { exportFiles (true, false); };
    loopButton.onClick = [this] { exportFiles (false, true); };
    bothButton.setColour (juce::TextButton::buttonColourId, accent);
    bothButton.onClick = [this] { exportFiles (true, true); };
    openButton.onClick = [this] { if (lastFolder.isDirectory()) lastFolder.revealToUser(); };
    status.setColour (juce::Label::textColourId, text);

    for (auto* c : std::initializer_list<juce::Component*> { &presetBox, &keyBox, &barsBox, &tempoSlider, &swingSlider, &seedEditor, &diceButton,
                                                             &generateButton, &lane, &leftButton, &rightButton, &lengthBox, &dupButton, &delButton,
                                                             &clearButton, &rootBox, &typeBox, &addButton, &addPaletteButton, &typedEditor,
                                                             &setTypedButton, &newPartsButton, &playButton, &stopButton, &midiButton, &loopButton,
                                                             &bothButton, &openButton, &status })
        addAndMakeVisible (c);

    setSize (1120, 720);
    presetChanged (false);
    generate();
}

const SongPreset& ProgressionPanel::preset() const
{
    return songPresets()[(size_t) juce::jlimit (0, (int) songPresets().size() - 1, presetBox.getSelectedId() - 1)];
}

void ProgressionPanel::presetChanged (bool keepChords)
{
    const auto& p = preset();
    if (settings != nullptr) settings->setValue ("songPreset", p.id);
    presetDesc.setText (p.description + "   (" + juce::String (p.tempoLow, 0) + "-" + juce::String (p.tempoHigh, 0) + " BPM, " + p.scale + ")",
                        juce::dontSendNotification);
    arr.presetId = p.id;
    arr.scale = p.scale;
    arr.swing = p.swing;
    arr.tempo = std::round ((p.tempoLow + p.tempoHigh) * 0.5);
    tempoSlider.setValue (arr.tempo, juce::dontSendNotification);
    swingSlider.setValue (arr.swing, juce::dontSendNotification);
    arr.parts.clear();                                     // the preset's own recipes
    if (keepChords) blocksChanged();
}

void ProgressionPanel::generate()
{
    const auto& p = preset();
    arr.seed = seedEditor.getText().getLargeIntValue();
    juce::Random rng (arr.seed);
    arr.tonic = keyBox.getSelectedId() > 1 ? keyBox.getSelectedId() - 2 : parsePitchClass (p.keys[rng.nextInt (p.keys.size())]);
    arr.blocks = generateProgression (p, arr.tonic, barsBox.getSelectedId(), arr.seed);
    selected = -1;
    blocksChanged();
    if (hooks.log) hooks.log ("Progression (" + p.name + ", " + pitchClassName (arr.tonic) + " " + arr.scale + "): " + arr.progressionText());
}

std::vector<ChordBlock> ProgressionPanel::keyChords() const
{
    // the 7th chord on each degree of the key's scale
    std::vector<int> scale;
    if (! getScale (arr.scale, scale) || scale.size() < 7) getScale ("major", scale);
    std::vector<ChordBlock> out;
    for (int d = 0; d < 7; ++d)
    {
        std::vector<int> iv;
        for (int k = 0; k < 4; ++k)
        {
            int idx = d + 2 * k, oct = idx / 7;
            iv.push_back (scale[(size_t) (idx % 7)] + 12 * oct - scale[(size_t) d]);
        }
        ChordBlock b;
        b.rootPc = (arr.tonic + scale[(size_t) d]) % 12;
        b.quality = "maj7";
        for (auto& q : allQualities()) if (q.intervals == iv) { b.quality = q.id; break; }
        out.push_back (b);
    }
    return out;
}

void ProgressionPanel::refreshKeyChords()
{
    auto kc = keyChords();
    for (int k = 0; k < keyChordButtons.size(); ++k)
        keyChordButtons[k]->setButtonText (k < (int) kc.size() ? kc[(size_t) k].roman (arr.tonic) + " " + kc[(size_t) k].name() : juce::String ("-"));
}

void ProgressionPanel::addBlock (const ChordBlock& b)
{
    auto at = selected >= 0 ? selected + 1 : (int) arr.blocks.size();
    arr.blocks.insert (arr.blocks.begin() + at, b);
    selected = at;
    blocksChanged();
}

void ProgressionPanel::selectBlock (int index)
{
    selected = index;
    if (juce::isPositiveAndBelow (index, (int) arr.blocks.size()))
    {
        const auto& b = arr.blocks[(size_t) index];
        lengthBox.setSelectedId ((int) b.beats, juce::dontSendNotification);
        selectedLabel.setText ("Selected: " + b.name() + " (" + b.roman (arr.tonic) + ")", juce::dontSendNotification);
    }
    lane.setBlocks (&arr.blocks, arr.tonic, selected);
}

void ProgressionPanel::blocksChanged()
{
    generateParts (arr, preset());
    // show each part's recipe
    for (int p = 0; p < 4; ++p)
        for (auto& part : arr.parts)
            if (part.id == partIds[p])
            {
                auto ids = juce::StringArray::fromTokens (partRecipes[p]->getProperties()["ids"].toString(), " ", "");
                partRecipes[p]->setSelectedId (ids.indexOf (part.recipeId) + 1, juce::dontSendNotification);
                partToggles[p]->setToggleState (part.enabled, juce::dontSendNotification);
            }
    for (auto& part : arr.parts) if (part.id == "drums") partToggles[4]->setToggleState (part.enabled, juce::dontSendNotification);
    refreshKeyChords();
    typedEditor.setText (arr.progressionText(), false);
    selectBlock (juce::jmin (selected, (int) arr.blocks.size() - 1));
    if (selected < 0) selectedLabel.setText ("Click a block to select it, drag to move it, double-click to hear it", juce::dontSendNotification);
    const auto beats = arr.totalBeats();
    status.setText (arr.blocks.empty() ? juce::String ("No chords.")
                                       : juce::String ((int) arr.blocks.size()) + " chords, " + juce::String (beats / 4.0, beats / 4.0 == std::floor (beats / 4.0) ? 0 : 2)
                                         + " bars, " + juce::String (arr.lengthSeconds(), 1) + " s at " + juce::String (arr.tempo, 0) + " BPM in "
                                         + pitchClassName (arr.tonic) + " " + arr.scale, juce::dontSendNotification);
    for (auto* b : { &playButton, &midiButton, &loopButton, &bothButton, &newPartsButton }) b->setEnabled (! arr.blocks.empty());
}

void ProgressionPanel::playLoop()
{
    if (arr.blocks.empty()) return;
    juce::AudioBuffer<float> mix;
    std::vector<juce::AudioBuffer<float>> stems;
    juce::String err;
    auto sr = hooks.sampleRate ? hooks.sampleRate() : 44100.0;
    if (! renderArrangement (arr, sr, mix, stems, err)) { status.setText ("Can't play: " + err, juce::dontSendNotification); return; }
    if (hooks.play) hooks.play (mix, sr);
    if (hooks.log) hooks.log ("> Progression loop: " + arr.progressionText() + "  (" + juce::String (arr.lengthSeconds(), 1) + " s, drumless)");
}

juce::String ProgressionPanel::baseName() const
{
    return safeFileName (preset().name.removeCharacters ("/()") + "_" + pitchClassName (arr.tonic) + "_" + juce::String (juce::roundToInt (arr.tempo))
                         + "bpm_" + juce::String (arr.seed));
}

void ProgressionPanel::exportFiles (bool midi, bool loops)
{
    if (arr.blocks.empty()) return;
    auto base = hooks.outputFolder ? hooks.outputFolder() : juce::File::getSpecialLocation (juce::File::userHomeDirectory);
    auto folder = base.getChildFile ("Progressions").getChildFile (baseName()).getNonexistentSibling (false);
    juce::StringArray written;
    if (midi) written.addArray (writeArrangementMidi (arr, folder, baseName()));
    if (loops)
    {
        juce::String err;
        auto wavs = writeArrangementLoops (arr, folder, baseName(), hooks.exportRate ? hooks.exportRate() : 44100.0, hooks.bitDepth ? hooks.bitDepth() : 24, err);
        if (wavs.isEmpty()) { status.setText ("Loop failed: " + err, juce::dontSendNotification); return; }
        written.addArray (wavs);
    }
    folder.getChildFile ("_progression.txt").replaceWithText (preset().name + "\n" + pitchClassName (arr.tonic) + " " + arr.scale + ", "
                                                              + juce::String (arr.tempo, 0) + " BPM, swing " + juce::String (arr.swing, 2) + ", seed " + juce::String (arr.seed)
                                                              + "\nChords: " + arr.progressionText() + "\n");
    lastFolder = folder;
    status.setText ("Wrote " + juce::String (written.size()) + " files to " + folder.getFullPathName(), juce::dontSendNotification);
    if (hooks.log) hooks.log ("Progression export: " + written.joinIntoString (", ") + " -> " + folder.getFullPathName());
}

void ProgressionPanel::paint (juce::Graphics& g) { g.fillAll (bg); }

void ProgressionPanel::resized()
{
    auto r = getLocalBounds().reduced (14, 10);
    auto row = [&r] (int h) { auto x = r.removeFromTop (h); r.removeFromTop (6); return x; };

    auto top = row (30);
    title.setBounds (top.removeFromLeft (150));
    generateButton.setBounds (top.removeFromRight (180)); top.removeFromRight (6);
    diceButton.setBounds (top.removeFromRight (52)); top.removeFromRight (4);
    seedEditor.setBounds (top.removeFromRight (80)); top.removeFromRight (10);
    presetBox.setBounds (top.removeFromLeft (260)); top.removeFromLeft (6);
    keyBox.setBounds (top.removeFromLeft (120)); top.removeFromLeft (6);
    barsBox.setBounds (top.removeFromLeft (100));
    auto second = row (34);
    tempoLabel.setBounds (second.removeFromLeft (46));
    tempoSlider.setBounds (second.removeFromLeft (170).withSizeKeepingCentre (170, 26)); second.removeFromLeft (8);
    swingLabel.setBounds (second.removeFromLeft (44));
    swingSlider.setBounds (second.removeFromLeft (150).withSizeKeepingCentre (150, 26)); second.removeFromLeft (14);
    presetDesc.setBounds (second);
    lane.setBounds (row (110));

    auto edit = row (28);
    selectedLabel.setBounds (edit.removeFromLeft (330));
    leftButton.setBounds (edit.removeFromLeft (34)); edit.removeFromLeft (2);
    rightButton.setBounds (edit.removeFromLeft (34)); edit.removeFromLeft (8);
    lengthBox.setBounds (edit.removeFromLeft (150)); edit.removeFromLeft (6);
    dupButton.setBounds (edit.removeFromLeft (90)); edit.removeFromLeft (4);
    delButton.setBounds (edit.removeFromLeft (70)); edit.removeFromLeft (4);
    clearButton.setBounds (edit.removeFromLeft (70));

    auto add = row (28);
    addLabel.setBounds (add.removeFromLeft (40));
    rootBox.setBounds (add.removeFromLeft (70)); add.removeFromLeft (4);
    typeBox.setBounds (add.removeFromLeft (100)); add.removeFromLeft (4);
    addButton.setBounds (add.removeFromLeft (90)); add.removeFromLeft (6);
    addPaletteButton.setBounds (add.removeFromLeft (100)); add.removeFromLeft (10);
    auto w = add.getWidth() / 7;
    for (auto* b : keyChordButtons) b->setBounds (add.removeFromLeft (w).reduced (2, 0));

    auto typed = row (28);
    typedLabel.setBounds (typed.removeFromLeft (60));
    setTypedButton.setBounds (typed.removeFromRight (90)); typed.removeFromRight (6);
    typedEditor.setBounds (typed);

    r.removeFromTop (4);
    auto th = row (22);
    newPartsButton.setBounds (th.removeFromRight (110));
    tracksLabel.setBounds (th);
    for (int p = 0; p < 5; ++p)
    {
        auto pr = row (26);
        partToggles[p]->setBounds (pr.removeFromLeft (150));
        partRecipes[p]->setBounds (pr.removeFromLeft (420));
    }

    auto bottom = r.removeFromBottom (32);
    bothButton.setBounds (bottom.removeFromRight (170)); bottom.removeFromRight (6);
    loopButton.setBounds (bottom.removeFromRight (110)); bottom.removeFromRight (6);
    midiButton.setBounds (bottom.removeFromRight (110)); bottom.removeFromRight (12);
    playButton.setBounds (bottom.removeFromLeft (100)); bottom.removeFromLeft (4);
    stopButton.setBounds (bottom.removeFromLeft (60)); bottom.removeFromLeft (6);
    openButton.setBounds (bottom.removeFromLeft (100)); bottom.removeFromLeft (8);
    status.setBounds (bottom);
}
}
