#include "ReviewPanel.h"
#include "Diagnostics.h"

namespace forge
{
namespace
{
    const juce::Colour bg (0xff16181d), panel (0xff20232a), accent (0xffe8483b), keepCol (0xff3fb950), rejectCol (0xff8b949e),
                       text (0xffe6e6e6), dim (0xff9aa0a6);

    enum Columns { colIndex = 1, colName, colType, colRoot, colScore, colLike, colMark };

    /** Splits one CSV line, honouring "quoted, fields". */
    juce::StringArray splitCsv (const juce::String& line)
    {
        juce::StringArray out;
        juce::String cur;
        bool quoted = false;
        for (int i = 0; i < line.length(); ++i)
        {
            auto c = line[i];
            if (quoted)
            {
                if (c == '"' && line[i + 1] == '"') { cur << '"'; ++i; }
                else if (c == '"') quoted = false;
                else cur << c;
            }
            else if (c == '"') quoted = true;
            else if (c == ',') { out.add (cur); cur.clear(); }
            else cur << c;
        }
        out.add (cur);
        return out;
    }
}

ReviewPanel::ReviewPanel (const juce::File& batchFolder, juce::AudioDeviceManager& dm)
    : folder (batchFolder), devices (dm)
{
    formats.registerBasicFormats();
    sourcePlayer.setSource (&transport);
    devices.addAudioCallback (&sourcePlayer);
    transport.addChangeListener (this);
    thumbnail.addChangeListener (this);

    auto& h = table.getHeader();
    h.addColumn ("#", colIndex, 44);
    h.addColumn ("File", colName, 360);
    h.addColumn ("Type", colType, 70);
    h.addColumn ("Root", colRoot, 54);
    h.addColumn ("Score", colScore, 56);
    h.addColumn ("Like", colLike, 80);
    h.addColumn ("Mark", colMark, 70);
    table.setColour (juce::ListBox::backgroundColourId, panel);
    table.setRowHeight (24);
    table.setMultipleSelectionEnabled (false);
    table.addKeyListener (this);
    addAndMakeVisible (table);

    playButton.onClick   = [this] { auto r = table.getSelectedRow(); if (playingRow == r && transport.isPlaying()) stop(); else play (r); };
    keepButton.onClick   = [this] { setMark (Mark::Keep, true); };
    rejectButton.onClick = [this] { setMark (Mark::Reject, true); };
    clearButton.onClick  = [this] { setMark (Mark::None, false); };
    exportButton.onClick = [this] { exportKeepers(); };
    registersButton.setTooltip ("Render octave-down and octave-up copies of just your keepers (same sound, same settings) - "
                                "much faster than making registers for the whole batch. Load the same instrument first.");
    registersButton.onClick = [this]
    {
        juce::Array<juce::File> keep;
        for (auto& e : entries) if (e.mark == Mark::Keep) keep.add (e.file);
        if (keep.isEmpty()) { status.setText ("Mark some keepers (K) first.", juce::dontSendNotification); return; }
        if (onMakeRegisters) onMakeRegisters (folder, keep);
    };
    trashButton.onClick  = [this] { trashRejects(); };
    folderButton.onClick = [this] { folder.revealToUser(); };
    keepButton.setColour (juce::TextButton::buttonColourId, keepCol.darker (0.4f));
    exportButton.setColour (juce::TextButton::buttonColourId, accent);
    moreButton.setTooltip ("Render 16 close variations of the selected sample: same recipe and chord, new tone, voicing, "
                           "inversion, velocity and length. They open in a new Review.");
    moreButton.onClick = [this]
    {
        auto row = table.getSelectedRow();
        if (! juce::isPositiveAndBelow (row, (int) entries.size())) return;
        if (onMoreLikeThis) onMoreLikeThis (folder, entries[(size_t) row].file);
    };
    for (auto* b : { &playButton, &keepButton, &rejectButton, &clearButton, &exportButton, &trashButton, &folderButton, &registersButton, &moreButton })
    {
        b->setWantsKeyboardFocus (false); // keep the keys going to the list
        addAndMakeVisible (b);
    }
    downButton.onClick = [this] { setTranspose (transpose - 1); };
    upButton.onClick = [this] { setTranspose (transpose + 1); };
    resetPitchButton.onClick = [this] { setTranspose (0); };
    rangeButton.onClick = [this] { startRangeCheck(); };
    downButton.setTooltip ("Down a semitone (Left arrow, Shift = octave)");
    upButton.setTooltip ("Up a semitone (Right arrow, Shift = octave)");
    rangeButton.setTooltip ("Plays the sample from an octave down to an octave up, like 16 Levels > Tune across the pads");
    for (auto* b : { &downButton, &upButton, &resetPitchButton, &rangeButton })
    {
        b->setWantsKeyboardFocus (false);
        addAndMakeVisible (b);
    }
    pitchLabel.setColour (juce::Label::textColourId, text);
    pitchLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (pitchLabel);
    setTranspose (0);

    autoPlay.setToggleState (true, juce::dontSendNotification);
    autoPlay.setWantsKeyboardFocus (false);
    addAndMakeVisible (autoPlay);

    help.setText ("Up/Down move+play  Space play  K keep  X reject  U clear  M more  Left/Right pitch",
                  juce::dontSendNotification);
    help.setColour (juce::Label::textColourId, dim);
    addAndMakeVisible (help);
    status.setColour (juce::Label::textColourId, text);
    addAndMakeVisible (status);

    load();
    Log::write ("review: opened " + folder.getFullPathName() + " (" + juce::String ((int) entries.size()) + " samples)");
    setSize (900, 680);
    if (! entries.empty()) table.selectRow (0);
    juce::MessageManager::callAsync ([safe = juce::Component::SafePointer<ReviewPanel> (this)]
                                     { if (safe) safe->table.grabKeyboardFocus(); });
}

ReviewPanel::~ReviewPanel()
{
    stopTimer();
    transport.removeChangeListener (this);
    thumbnail.removeChangeListener (this);
    transport.setSource (nullptr);
    devices.removeAudioCallback (&sourcePlayer);
    sourcePlayer.setSource (nullptr);
}

//==============================================================================
void ReviewPanel::load()
{
    entries.clear();
    auto manifest = folder.getChildFile ("_manifest.csv");
    if (manifest.existsAsFile())
    {
        juce::StringArray lines;
        lines.addLines (manifest.loadFileAsString());
        auto header = splitCsv (lines[0]);
        auto col = [&header] (const char* name) { return header.indexOf (name); };
        std::vector<std::pair<juce::String, juce::File>> copies; // source -> copy
        for (int i = 1; i < lines.size(); ++i)
        {
            if (lines[i].trim().isEmpty()) continue;
            auto f = splitCsv (lines[i]);
            auto file = folder.getChildFile (f[col ("file")]);
            if (col ("register_semitones") >= 0 && f[col ("register_semitones")].getIntValue() != 0)
            {
                // octave copies are not reviewed on their own: they follow their original
                if (col ("source") >= 0 && file.existsAsFile()) copies.push_back ({ f[col ("source")], file });
                continue;
            }
            Entry e;
            e.file = file;
            if (! e.file.existsAsFile()) continue; // already moved / deleted
            if (col ("sounds_like") >= 0 && f[col ("sounds_like")].isNotEmpty())
                e.like = f[col ("sounds_like")] + " (" + juce::String (f[col ("sounds_like_db")].getFloatValue(), 1) + ")";
            e.recipe = col ("recipe") >= 0 ? f[col ("recipe")] : juce::String();
            e.type = f[col ("type")];
            e.name = f[col ("name")];
            e.root = f[col ("root_note")];
            e.score = col ("score") >= 0 && f[col ("score")].isNotEmpty() ? f[col ("score")].getIntValue() : -1;
            entries.push_back (e);
        }
        for (auto& [src, copy] : copies)
            for (auto& e : entries)
                if (e.file == folder.getChildFile (src)) { e.copies.add (copy); break; }
    }
    else // any folder of WAVs works too
    {
        for (auto& f : folder.findChildFiles (juce::File::findFiles, true, "*.wav"))
        {
            if (f.getFullPathName().contains ("_Keepers") || f.getFullPathName().contains (".candidates")
                || f.getFullPathName().contains ("Octave")) continue;
            Entry e;
            e.file = f;
            entries.push_back (e);
        }
        std::sort (entries.begin(), entries.end(), [] (auto& a, auto& b) { return a.file.getFullPathName() < b.file.getFullPathName(); });
    }

    // restore marks from an earlier session
    auto marks = juce::JSON::parse (folder.getChildFile ("_review.json"));
    if (auto* obj = marks.getDynamicObject())
        for (auto& e : entries)
        {
            auto m = obj->getProperty (e.file.getRelativePathFrom (folder).replaceCharacter ('\\', '/')).toString();
            e.mark = m == "keep" ? Mark::Keep : m == "reject" ? Mark::Reject : Mark::None;
        }

    table.updateContent();
    updateStatus();
}

void ReviewPanel::saveMarks() const
{
    auto* obj = new juce::DynamicObject();
    for (auto& e : entries)
        if (e.mark != Mark::None)
            obj->setProperty (e.file.getRelativePathFrom (folder).replaceCharacter ('\\', '/'), e.mark == Mark::Keep ? "keep" : "reject");
    folder.getChildFile ("_review.json").replaceWithText (juce::JSON::toString (juce::var (obj)));
}

void ReviewPanel::updateStatus()
{
    int keep = 0, reject = 0;
    for (auto& e : entries) { keep += e.mark == Mark::Keep; reject += e.mark == Mark::Reject; }
    status.setText (juce::String (entries.size()) + " samples   |   " + juce::String (keep) + " kept   |   " + juce::String (reject)
                    + " rejected   |   " + juce::String ((int) entries.size() - keep - reject) + " to go   ("
                    + folder.getFileName() + ")", juce::dontSendNotification);
    exportButton.setEnabled (keep > 0);
    registersButton.setEnabled (keep > 0);
    trashButton.setEnabled (reject > 0);
}

//==============================================================================
void ReviewPanel::paint (juce::Graphics& g)
{
    g.fillAll (bg);
    g.setColour (panel);
    g.fillRoundedRectangle (waveArea.toFloat(), 4.0f);

    auto row = table.getSelectedRow();
    if (thumbnail.getTotalLength() > 0 && juce::isPositiveAndBelow (row, (int) entries.size()))
    {
        auto area = waveArea.reduced (6);
        auto mark = entries[(size_t) row].mark;
        g.setColour (mark == Mark::Keep ? keepCol : mark == Mark::Reject ? rejectCol : accent);
        thumbnail.drawChannels (g, area, 0.0, thumbnail.getTotalLength(), 1.0f);

        if (transport.isPlaying() && playingRow == row)
        {
            auto x = area.getX() + (float) (transport.getCurrentPosition() / thumbnail.getTotalLength()) * area.getWidth();
            g.setColour (text);
            g.drawVerticalLine ((int) x, (float) area.getY(), (float) area.getBottom());
        }
        g.setColour (dim);
        g.setFont (12.0f);
        g.drawText (juce::String (juce::roundToInt (thumbnail.getTotalLength() * 1000.0)) + " ms", area.removeFromBottom (14), juce::Justification::bottomRight);
    }
}

void ReviewPanel::resized()
{
    auto r = getLocalBounds().reduced (12);
    auto top = r.removeFromTop (30);
    playButton.setBounds (top.removeFromLeft (110)); top.removeFromLeft (6);
    keepButton.setBounds (top.removeFromLeft (90)); top.removeFromLeft (6);
    rejectButton.setBounds (top.removeFromLeft (100)); top.removeFromLeft (6);
    clearButton.setBounds (top.removeFromLeft (90)); top.removeFromLeft (14);
    autoPlay.setBounds (top.removeFromLeft (150));
    folderButton.setBounds (top.removeFromRight (100)); top.removeFromRight (8);
    moreButton.setBounds (top.removeFromRight (150));

    r.removeFromTop (8);
    waveArea = r.removeFromTop (110);
    r.removeFromTop (6);
    auto helpRow = r.removeFromTop (26);
    rangeButton.setBounds (helpRow.removeFromRight (130)); helpRow.removeFromRight (6);
    upButton.setBounds (helpRow.removeFromRight (28)); helpRow.removeFromRight (2);
    resetPitchButton.setBounds (helpRow.removeFromRight (28)); helpRow.removeFromRight (2);
    downButton.setBounds (helpRow.removeFromRight (28)); helpRow.removeFromRight (6);
    pitchLabel.setBounds (helpRow.removeFromRight (150));
    help.setBounds (helpRow);

    auto bottom = r.removeFromBottom (32);
    exportButton.setBounds (bottom.removeFromRight (150)); bottom.removeFromRight (8);
    registersButton.setBounds (bottom.removeFromRight (190)); bottom.removeFromRight (8);
    trashButton.setBounds (bottom.removeFromRight (170)); bottom.removeFromRight (8);
    status.setBounds (bottom);
    r.removeFromBottom (6);
    table.setBounds (r);
}

//==============================================================================
int ReviewPanel::getNumRows() { return (int) entries.size(); }

void ReviewPanel::paintRowBackground (juce::Graphics& g, int row, int, int, bool selected)
{
    if (! juce::isPositiveAndBelow (row, (int) entries.size())) return;
    auto mark = entries[(size_t) row].mark;
    if (selected)                  g.fillAll (accent.withAlpha (0.35f));
    else if (mark == Mark::Keep)   g.fillAll (keepCol.withAlpha (0.15f));
    else if (row % 2)              g.fillAll (juce::Colours::white.withAlpha (0.03f));
}

void ReviewPanel::paintCell (juce::Graphics& g, int row, int col, int w, int h, bool)
{
    if (! juce::isPositiveAndBelow (row, (int) entries.size())) return;
    auto& e = entries[(size_t) row];
    g.setColour (e.mark == Mark::Reject ? rejectCol.withAlpha (0.6f) : text);
    g.setFont (13.0f);
    juce::String t;
    switch (col)
    {
        case colIndex: t = juce::String (row + 1); break;
        case colName:  t = e.file.getFileNameWithoutExtension(); break;
        case colType:  t = e.type; break;
        case colRoot:  t = e.root; break;
        case colScore: t = e.score >= 0 ? juce::String (e.score) : juce::String ("-"); break;
        case colLike:  t = e.like; break;
        case colMark:
            t = e.mark == Mark::Keep ? "KEEP" : e.mark == Mark::Reject ? "reject" : "";
            if (e.mark == Mark::Keep) g.setColour (keepCol);
            break;
        default: break;
    }
    g.drawText (t, 4, 0, w - 8, h, juce::Justification::centredLeft, true);
}

void ReviewPanel::selectedRowsChanged (int row)
{
    if (! juce::isPositiveAndBelow (row, (int) entries.size())) return;
    thumbnail.setSource (new juce::FileInputSource (entries[(size_t) row].file));
    if (autoPlay.getToggleState()) play (row);
    repaint (waveArea);
}

void ReviewPanel::cellDoubleClicked (int row, int, const juce::MouseEvent&) { play (row); }

void ReviewPanel::sortOrderChanged (int columnId, bool forwards)
{
    auto key = [columnId] (const Entry& e) -> juce::String
    {
        switch (columnId)
        {
            case colScore: return juce::String (e.score + 1000).paddedLeft ('0', 5);
            case colType:  return e.type;
            case colRoot:  return e.root;
            case colMark:  return juce::String ((int) e.mark);
            default:       return e.file.getFileName();
        }
    };
    std::stable_sort (entries.begin(), entries.end(), [&] (const Entry& a, const Entry& b)
                      { return forwards ? key (a) < key (b) : key (b) < key (a); });
    table.updateContent();
    table.repaint();
}

//==============================================================================
bool ReviewPanel::keyPressed (const juce::KeyPress& k) { return keyPressed (k, this); }

bool ReviewPanel::keyPressed (const juce::KeyPress& k, juce::Component*)
{
    if (! rangeSteps.empty() && k.getTextCharacter() != 'r') { stopTimer(); rangeSteps.clear(); }
    auto c = juce::CharacterFunctions::toLowerCase (k.getTextCharacter());
    if (k == juce::KeyPress::spaceKey || k == juce::KeyPress::returnKey)
    {
        auto r = table.getSelectedRow();
        if (playingRow == r && transport.isPlaying()) stop(); else play (r);
        return true;
    }
    if (k.getKeyCode() == juce::KeyPress::leftKey || k.getKeyCode() == juce::KeyPress::rightKey)
    {
        auto step = k.getModifiers().isShiftDown() ? 12 : 1;
        setTranspose (transpose + (k.getKeyCode() == juce::KeyPress::rightKey ? step : -step));
        return true;
    }
    if (c == 'r') { startRangeCheck(); return true; }
    if (c == 'm') { moreButton.triggerClick(); return true; }
    if (c == '0') { setTranspose (0); return true; }
    if (c == 'k') { setMark (Mark::Keep, true); return true; }
    if (c == 'x') { setMark (Mark::Reject, true); return true; }
    if (c == 'u') { setMark (Mark::None, false); return true; }
    return false; // arrows etc. go to the table
}

void ReviewPanel::setMark (Mark m, bool advance)
{
    auto row = table.getSelectedRow();
    if (! juce::isPositiveAndBelow (row, (int) entries.size())) return;
    entries[(size_t) row].mark = m;
    saveMarks();
    updateStatus();
    table.repaintRow (row);
    if (advance && row + 1 < (int) entries.size())
        table.selectRow (row + 1); // plays the next one when auto-play is on
    repaint (waveArea);
}

void ReviewPanel::play (int row)
{
    if (! juce::isPositiveAndBelow (row, (int) entries.size())) return;
    stop();

    // Only start when an audio device is really running: AudioTransportSource::stop()
    // waits up to a second for the audio thread, which froze the list on every key
    // press when no output was open.
    auto* device = devices.getCurrentAudioDevice();
    if (device == nullptr || ! device->isPlaying())
    {
        status.setText ("No audio output running (see Diagnostics). Marking and export still work.", juce::dontSendNotification);
        return;
    }

    if (auto* reader = formats.createReaderFor (entries[(size_t) row].file))
    {
        auto sr = reader->sampleRate;
        readerSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
        // Re-pitch like a sampler / MPC Tune: playing faster raises the pitch and shortens it.
        transport.setSource (readerSource.get(), 0, nullptr, sr * std::pow (2.0, transpose / 12.0));
        transport.setPosition (0);
        transport.start();
        playingRow = row;
    }
}

void ReviewPanel::setTranspose (int semitones, bool playNow)
{
    transpose = juce::jlimit (-24, 24, semitones);
    auto text_ = transpose == 0 ? juce::String ("Pitch: as recorded")
                                : "Pitch: " + juce::String (transpose > 0 ? "+" : "") + juce::String (transpose) + " st";
    pitchLabel.setText (text_, juce::dontSendNotification);
    pitchLabel.setColour (juce::Label::textColourId, std::abs (transpose) > 7 ? accent : text);
    pitchLabel.setTooltip (std::abs (transpose) > 7 ? "More than a 5th away from the root: listen for grain or dullness - "
                                                      "a register set may sound better" : juce::String());
    auto row = table.getSelectedRow();
    if (playNow && row >= 0 && isShowing()) play (row);
}

void ReviewPanel::startRangeCheck()
{
    if (! juce::isPositiveAndBelow (table.getSelectedRow(), (int) entries.size())) return;
    rangeSteps = { -12, -7, -5, 0, 5, 7, 12 }; // octave down to octave up, like reaching across the pads
    rangeIndex = 0;
    timerCallback();
    startTimer (330);
}

void ReviewPanel::timerCallback()
{
    if (rangeIndex >= rangeSteps.size())
    {
        stopTimer();
        rangeSteps.clear();
        setTranspose (0, false); // back to the recorded pitch, without playing again
        return;
    }
    setTranspose (rangeSteps[rangeIndex++]); // updates the label and plays at that pitch
}

void ReviewPanel::stop()
{
    if (transport.isPlaying()) transport.stop();
    transport.setSource (nullptr);
    readerSource.reset();
    playingRow = -1;
}

void ReviewPanel::changeListenerCallback (juce::ChangeBroadcaster*)
{
    repaint (waveArea);
    if (transport.isPlaying())
        juce::Timer::callAfterDelay (30, [safe = juce::Component::SafePointer<ReviewPanel> (this)]
                                         { if (safe) safe->changeListenerCallback (nullptr); });
}

//==============================================================================
void ReviewPanel::exportKeepers()
{
    std::vector<const Entry*> keep;
    for (auto& e : entries) if (e.mark == Mark::Keep) keep.push_back (&e);
    if (keep.empty()) return;

    auto dest = folder.getChildFile ("_Keepers").getNonexistentSibling (false);
    dest.createDirectory();
    juce::StringArray list, problems;
    for (size_t i = 0; i < keep.size(); ++i)
    {
        auto bank = (int) i / 16;
        auto bankDir = dest.getChildFile ("Bank_" + juce::String::charToString ((juce::juce_wchar) ('A' + bank % 26)));
        bankDir.createDirectory();
        // the number keeps your pad order when the MPC sorts by name
        auto target = bankDir.getChildFile ("K" + juce::String ((int) i + 1).paddedLeft ('0', 3) + "_" + keep[i]->file.getFileName());
        auto prefix = "K" + juce::String ((int) i + 1).paddedLeft ('0', 3) + "_";
        auto baseName = keep[i]->file.getFileName();
        // short names already start with a number: keep only the K-number for pad order
        if (baseName.length() > 4 && juce::CharacterFunctions::isDigit (baseName[0]) && baseName.containsChar ('_')
            && baseName.upToFirstOccurrenceOf ("_", false, false).containsOnly ("0123456789"))
            baseName = baseName.fromFirstOccurrenceOf ("_", false, false);
        target = bankDir.getChildFile (prefix + baseName);
        if (keep[i]->file.copyFileTo (target)) list.add (target.getRelativePathFrom (dest).replaceCharacter ('\\', '/') + "   <-  " + keep[i]->file.getFileName());
        else problems.add (keep[i]->file.getFileName());

        // its octave copies go along, into matching register folders
        for (auto& copy : keep[i]->copies)
        {
            auto registerName = copy.getRelativePathFrom (folder).upToFirstOccurrenceOf ("/", false, false)
                                    .upToFirstOccurrenceOf ("\\", false, false);
            auto copyName = copy.getFileName();
            if (copyName.upToFirstOccurrenceOf ("_", false, false).containsOnly ("0123456789"))
                copyName = copyName.fromFirstOccurrenceOf ("_", false, false);
            auto copyTarget = dest.getChildFile (registerName).getChildFile (bankDir.getFileName()).getChildFile (prefix + copyName);
            copyTarget.getParentDirectory().createDirectory();
            if (! copy.copyFileTo (copyTarget)) problems.add (copy.getFileName());
        }
    }

    auto setup = folder.getChildFile ("_MPC_Setup.txt");
    if (setup.existsAsFile()) setup.copyFileTo (dest.getChildFile ("_MPC_Setup.txt"));
    dest.getChildFile ("_keepers.txt").replaceWithText ("Keepers from " + folder.getFullPathName() + "\n\n" + list.joinIntoString ("\n") + "\n");

    Log::write ("review: exported " + juce::String (list.size()) + " keepers to " + dest.getFullPathName());
    status.setText ("Exported " + juce::String (list.size()) + " keepers to " + dest.getFileName()
                    + (problems.isEmpty() ? juce::String() : "  (" + juce::String (problems.size()) + " could not be copied)"),
                    juce::dontSendNotification);
    dest.revealToUser();
}

void ReviewPanel::trashRejects()
{
    int n = 0;
    for (auto& e : entries) n += e.mark == Mark::Reject;
    if (n == 0) return;

    auto options = juce::MessageBoxOptions::makeOptionsOkCancel (juce::MessageBoxIconType::QuestionIcon, "Move rejects to Trash?",
        juce::String (n) + " rejected samples will be moved to the Trash / Recycle Bin. You can restore them from there.",
        "Move to Trash", "Cancel", this);

    juce::AlertWindow::showAsync (options, [safe = juce::Component::SafePointer<ReviewPanel> (this)] (int result)
    {
        if (safe == nullptr || result != 1) return;
        safe->stop();
        int moved = 0;
        std::vector<Entry> remaining;
        for (auto& e : safe->entries)
        {
            if (e.mark == Mark::Reject && e.file.moveToTrash()) { ++moved; continue; }
            remaining.push_back (e);
        }
        safe->entries = std::move (remaining);
        safe->saveMarks();
        safe->table.updateContent();
        safe->table.repaint();
        safe->updateStatus();
        Log::write ("review: moved " + juce::String (moved) + " rejects to the trash from " + safe->folder.getFullPathName());
        safe->status.setText ("Moved " + juce::String (moved) + " rejects to the Trash.", juce::dontSendNotification);
    });
}
}
