#include "DiagnosticsPanel.h"

namespace forge
{
DiagnosticsPanel::DiagnosticsPanel (Hooks h) : juce::Thread ("Forge self-test"), hooks (std::move (h))
{
    text.setMultiLine (true);
    text.setReadOnly (true);
    text.setScrollbarsShown (true);
    text.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::plain));
    text.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff20232a));
    addAndMakeVisible (text);

    runButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffe8483b));
    runButton.onClick = [this] { startSelfTest(); };
    refreshButton.onClick = [this] { refresh(); };
    copyButton.onClick = [this]
    {
        juce::SystemClipboard::copyTextToClipboard (text.getText());
        status.setText ("Report copied - paste it into a message to report a problem.", juce::dontSendNotification);
    };
    saveButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> ("Save diagnostics report",
            juce::File::getSpecialLocation (juce::File::userDesktopDirectory).getChildFile ("MPCSampleForge_Diagnostics.txt"), "*.txt");
        chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
                              [this] (const juce::FileChooser& fc)
                              {
                                  auto f = fc.getResult();
                                  if (f == juce::File()) return;
                                  if (f.replaceWithText (text.getText()))
                                      status.setText ("Saved " + f.getFullPathName(), juce::dontSendNotification);
                              });
    };
    logButton.onClick = [] { Log::file().getParentDirectory().createDirectory(); Log::file().revealToUser(); };

    for (auto* b : { &runButton, &refreshButton, &copyButton, &saveButton, &logButton })
        addAndMakeVisible (b);

    status.setColour (juce::Label::textColourId, juce::Colour (0xff9aa0a6));
    status.setText ("Run the self-test after loading your VST3 to check it as well. Copy the report to share it.", juce::dontSendNotification);
    addAndMakeVisible (status);

    refresh();
    setSize (860, 640);
}

DiagnosticsPanel::~DiagnosticsPanel()
{
    cancel = true;
    stopThread (15000);
    if (holdingPlugin && hooks.release) hooks.release();
}

void DiagnosticsPanel::paint (juce::Graphics& g) { g.fillAll (juce::Colour (0xff16181d)); }

void DiagnosticsPanel::resized()
{
    auto r = getLocalBounds().reduced (12);
    auto top = r.removeFromTop (32);
    runButton.setBounds (top.removeFromLeft (130)); top.removeFromLeft (8);
    refreshButton.setBounds (top.removeFromLeft (90)); top.removeFromLeft (8);
    copyButton.setBounds (top.removeFromLeft (110)); top.removeFromLeft (8);
    saveButton.setBounds (top.removeFromLeft (120)); top.removeFromLeft (8);
    logButton.setBounds (top.removeFromLeft (130));
    r.removeFromTop (6);
    status.setBounds (r.removeFromBottom (24));
    text.setBounds (r);
}

void DiagnosticsPanel::refresh()
{
    text.setText (hooks.report ? hooks.report() : juce::String());
    if (selfTestText.isNotEmpty())
        text.setText (text.getText() + "\nSELF-TEST\n" + selfTestText);
}

void DiagnosticsPanel::append (const juce::String& s)
{
    selfTestText << s << "\n";
    text.moveCaretToEnd();
    text.insertTextAtCaret (s + "\n");
}

void DiagnosticsPanel::startSelfTest()
{
    if (isThreadRunning()) { cancel = true; return; }

    testPlugin = hooks.acquire ? hooks.acquire() : nullptr;
    holdingPlugin = true;
    cancel = false;
    summary = {};
    selfTestText = {};
    refresh();
    append ("\nSELF-TEST (" + juce::Time::getCurrentTime().toString (true, true) + ")");
    runButton.setButtonText ("Stop");
    status.setText ("Running... rendering every engine and recipe takes a few seconds.", juce::dontSendNotification);
    startThread();
}

void DiagnosticsPanel::run()
{
    auto safe = juce::Component::SafePointer<DiagnosticsPanel> (this);
    runSelfTest ([safe] (const DiagResult& r)
                 {
                     juce::MessageManager::callAsync ([safe, r]
                     {
                         if (safe == nullptr) return;
                         safe->summary.add (r);
                         safe->append (r.toString());
                     });
                 },
                 testPlugin, cancel);

    juce::MessageManager::callAsync ([safe]
    {
        if (safe == nullptr) return;
        safe->append ("\nRESULT: " + safe->summary.toString());
        Log::write ("self-test: " + safe->summary.toString());
        safe->runButton.setButtonText ("Run Self-Test");
        safe->status.setText (safe->summary.fail > 0 ? "Problems found - click Copy Report and send it."
                                                     : "Self-test finished: " + safe->summary.toString(), juce::dontSendNotification);
        if (safe->holdingPlugin && safe->hooks.release) safe->hooks.release();
        safe->holdingPlugin = false;
    });
}
}
