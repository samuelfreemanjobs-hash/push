#pragma once
#include "RenderQueue.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace forge
{
    /** The render queue window: batches waiting to render, one after another. */
    class QueuePanel final : public juce::Component,
                             private juce::ListBoxModel,
                             private juce::Timer
    {
    public:
        struct Hooks
        {
            std::function<std::vector<QueueItem>&()> items;
            std::function<bool()> running;
            std::function<void()> start, stop, changed;
            std::function<void (const juce::File&)> review;
        };

        explicit QueuePanel (Hooks hooks);
        ~QueuePanel() override;

        void refresh();
        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        int getNumRows() override;
        void paintListBoxItem (int row, juce::Graphics&, int w, int h, bool selected) override;
        void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override;
        juce::String getTooltipForRow (int row) override;
        void timerCallback() override { refresh(); }
        void move (int delta);

        Hooks hooks;
        juce::ListBox list { "queue", this };
        juce::Label title, help, status;
        juce::TextButton startButton { "Start Queue" }, removeButton { "Remove" }, upButton { "Up" }, downButton { "Down" },
                         clearDoneButton { "Clear Finished" }, clearAllButton { "Clear All" }, retryButton { "Run Again" };
    };
}
