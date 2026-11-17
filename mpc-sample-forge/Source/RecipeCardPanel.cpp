#include "RecipeCardPanel.h"

namespace forge
{
namespace
{
    const juce::Colour bg (0xff16181d), panel (0xff20232a), accent (0xffe8483b), text (0xffe6e6e6), dim (0xff9aa0a6);
}

RecipeCardPanel::RecipeCardPanel (const juce::String& instrumentName, bool isBuiltIn, juce::PropertiesFile* s)
    : settings (s), builtIn (isBuiltIn)
{
    title.setText ("Recipe Cards", juce::dontSendNotification);
    title.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    title.setColour (juce::Label::textColourId, accent);
    help.setText ("Paste a recipe book page (e.g. your daily ChatGPT installment) - one recipe or several. The app reads the chords with their "
                  "exact notes, the melody, the lead / soft notes, textures, key, tempo, sound and FX, and renders every file with the "
                  "card's own names, plus the chords and melody as .mid files. Most reliable with a FORGE CARD block at the end of each "
                  "recipe: 'Copy ChatGPT prompt' copies the instruction to give ChatGPT.", juce::dontSendNotification);
    help.setJustificationType (juce::Justification::topLeft);
    help.setMinimumHorizontalScale (1.0f);
    for (auto* l : { &help, &pasteLabel, &previewLabel, &status }) l->setColour (juce::Label::textColourId, dim);
    pasteLabel.setText ("Paste the recipe here", juce::dontSendNotification);
    previewLabel.setText ("What it will render", juce::dontSendNotification);

    input.setMultiLine (true, true);
    input.setReturnKeyStartsNewLine (true);
    input.setFont (juce::FontOptions (13.0f));
    input.setText (settings != nullptr ? settings->getValue ("cardText") : juce::String(), false);
    preview.setMultiLine (true, false);
    preview.setReadOnly (true);
    preview.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 12.5f, juce::Font::plain));
    preview.setColour (juce::TextEditor::backgroundColourId, panel);

    cardBox.onChange = [this] { lastText = {}; timerCallback(); };
    notationBox.addItem ("Note names: standard (C4 = middle C)", 1);
    notationBox.addItem ("Note names: MPC (C3 = middle C)", 2);
    notationBox.setSelectedId (settings != nullptr && settings->getBoolValue ("cardMpcNotes", false) ? 2 : 1, juce::dontSendNotification);
    notationBox.setTooltip ("How the card writes octaves. ChatGPT and most books use standard names (C4 = middle C); the MPC calls middle C 'C3'.");
    notationBox.onChange = [this] { lastText = {}; timerCallback(); };
    if (builtIn)
    {
        soundBox.addItem ("Sound: the card's (built-in synth)", 1);
        soundBox.addItem ("Sound: the main window's engine", 2);
    }
    else soundBox.addItem ("Sound: " + instrumentName + " (current sound)", 1);
    soundBox.setSelectedId (1, juce::dontSendNotification);
    fxBox.addItem ("FX: the card's", 1);
    fxBox.addItem ("FX: the main window's recipe", 2);
    fxBox.setSelectedId (1, juce::dontSendNotification);

    pasteButton.onClick = [this] { input.setText (juce::SystemClipboard::getTextFromClipboard(), true); };
    loadButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> ("Open a recipe card", juce::File(), "*.txt;*.md");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) input.setText (fc.getResult().loadFileAsString(), true); });
    };
    promptButton.setTooltip ("Copies the instruction for ChatGPT: end every recipe with a FORGE CARD block the app reads exactly");
    promptButton.onClick = [this]
    {
        juce::SystemClipboard::copyTextToClipboard (forgeCardPrompt());
        status.setText ("Copied - paste it into ChatGPT (e.g. in your recipe book's instructions).", juce::dontSendNotification);
    };
    renderButton.setColour (juce::TextButton::buttonColourId, accent);
    renderButton.onClick = [this] { if (onRender && ! cards.empty()) onRender (current()); };
    queueButton.setTooltip ("Add every card to the render queue (one batch each) and start it");
    queueButton.onClick = [this] { if (onQueueAll && ! cards.empty()) onQueueAll (current(), (int) cards.size()); };

    for (auto* c : std::initializer_list<juce::Component*> { &title, &help, &pasteLabel, &previewLabel, &status, &input, &preview, &cardBox,
                                                             &notationBox, &soundBox, &fxBox, &pasteButton, &loadButton, &promptButton,
                                                             &renderButton, &queueButton })
        addAndMakeVisible (c);
    setSize (1100, 680);
    startTimer (300);
    timerCallback();
}

RecipeCardPanel::Options RecipeCardPanel::current() const
{
    Options o;
    o.text = input.getText();
    o.cardIndex = juce::jmax (0, cardBox.getSelectedId() - 1);
    o.mpcNotation = notationBox.getSelectedId() == 2;
    o.useRecipeFx = fxBox.getSelectedId() == 2;
    o.useMainEngine = builtIn && soundBox.getSelectedId() == 2;
    return o;
}

void RecipeCardPanel::timerCallback()
{
    auto t = input.getText() + "|" + juce::String (notationBox.getSelectedId());
    if (t == lastText) return;
    lastText = t;
    reparse();
}

void RecipeCardPanel::reparse()
{
    if (settings != nullptr)
    {
        settings->setValue ("cardText", input.getText());
        settings->setValue ("cardMpcNotes", notationBox.getSelectedId() == 2);
    }
    const int previous = cardBox.getSelectedId();
    cards = parseRecipeCards (input.getText(), notationBox.getSelectedId() == 2);
    cardBox.clear (juce::dontSendNotification);
    for (size_t k = 0; k < cards.size(); ++k)
        cardBox.addItem (juce::String ((int) k + 1) + ". " + cards[k].name + "  (" + juce::String ((int) cards[k].items.size()) + " files)", (int) k + 1);
    if (! cards.empty()) cardBox.setSelectedId (juce::jlimit (1, (int) cards.size(), previous > 0 ? previous : 1), juce::dontSendNotification);

    auto index = juce::jmax (0, cardBox.getSelectedId() - 1);
    if (cards.empty())
    {
        preview.setText (input.getText().trim().isEmpty() ? "Paste a recipe on the left."
                                                          : "No recipe found yet. The app looks for a FORGE CARD block, or a numbered recipe "
                                                            "(\"1. Name\") with a chord table (chord name + exact notes), a melody table and a "
                                                            "list of .wav file names.", false);
        status.setText ({}, juce::dontSendNotification);
    }
    else
    {
        preview.setText (cards[(size_t) index].describe(), false);
        int files = 0;
        for (auto& c : cards) files += (int) c.items.size();
        status.setText (juce::String ((int) cards.size()) + " recipe" + (cards.size() == 1 ? "" : "s") + ", " + juce::String (files) + " files in all",
                        juce::dontSendNotification);
    }
    renderButton.setEnabled (! cards.empty());
    queueButton.setEnabled (! cards.empty());
}

void RecipeCardPanel::paint (juce::Graphics& g) { g.fillAll (bg); }

void RecipeCardPanel::resized()
{
    auto r = getLocalBounds().reduced (14, 10);
    title.setBounds (r.removeFromTop (28));
    help.setBounds (r.removeFromTop (50));
    r.removeFromTop (6);

    auto bottom = r.removeFromBottom (32);
    renderButton.setBounds (bottom.removeFromRight (150)); bottom.removeFromRight (6);
    queueButton.setBounds (bottom.removeFromRight (140)); bottom.removeFromRight (12);
    promptButton.setBounds (bottom.removeFromLeft (170)); bottom.removeFromLeft (8);
    status.setBounds (bottom);
    r.removeFromBottom (8);
    auto options = r.removeFromBottom (28);
    cardBox.setBounds (options.removeFromLeft (300)); options.removeFromLeft (6);
    notationBox.setBounds (options.removeFromLeft (270)); options.removeFromLeft (6);
    fxBox.setBounds (options.removeFromRight (200)); options.removeFromRight (6);
    soundBox.setBounds (options);
    r.removeFromBottom (8);

    auto left = r.removeFromLeft (r.getWidth() * 45 / 100);
    r.removeFromLeft (10);
    auto leftTop = left.removeFromTop (26);
    loadButton.setBounds (leftTop.removeFromRight (100)); leftTop.removeFromRight (4);
    pasteButton.setBounds (leftTop.removeFromRight (70));
    pasteLabel.setBounds (leftTop);
    left.removeFromTop (4);
    input.setBounds (left);
    previewLabel.setBounds (r.removeFromTop (26));
    r.removeFromTop (4);
    preview.setBounds (r);
}
}
