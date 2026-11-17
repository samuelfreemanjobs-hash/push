#pragma once
#include "RecipeCard.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace forge
{
    /** The "Recipe Cards" window: paste a recipe book page (e.g. a daily ChatGPT installment),
        see exactly which files each recipe will make, then render them with their own names. */
    class RecipeCardPanel final : public juce::Component, private juce::Timer
    {
    public:
        struct Options
        {
            juce::String text;
            int cardIndex = 0;
            bool mpcNotation = false, useRecipeFx = false, useMainEngine = false;
        };

        RecipeCardPanel (const juce::String& instrumentName, bool builtIn, juce::PropertiesFile* settings);

        std::function<void (const Options&)> onRender;      // one card
        std::function<void (const Options&, int count)> onQueueAll;

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        void timerCallback() override;
        void reparse();
        Options current() const;

        juce::PropertiesFile* settings;
        bool builtIn;
        std::vector<RecipeCard> cards;
        juce::String lastText;
        std::unique_ptr<juce::FileChooser> chooser;

        juce::Label title, help, pasteLabel, previewLabel, status;
        juce::TextEditor input, preview;
        juce::ComboBox cardBox, notationBox, soundBox, fxBox;
        juce::TextButton pasteButton { "Paste" }, loadButton { "Open .txt..." }, promptButton { "Copy ChatGPT prompt" },
                         renderButton { "Render this card" }, queueButton { "Queue all cards" };
    };
}
