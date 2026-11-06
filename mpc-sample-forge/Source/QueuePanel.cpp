#include "QueuePanel.h"

namespace forge
{
namespace
{
    const juce::Colour bg (0xff16181d), panel (0xff20232a), accent (0xffe8483b), text (0xffe6e6e6), dim (0xff9aa0a6),
                       good (0xff4caf50), bad (0xffe57373);
}

QueuePanel::QueuePanel (Hooks h) : hooks (std::move (h))
{
    title.setText ("Render Queue", juce::dontSendNotification);
    title.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    title.setColour (juce::Label::textColourId, accent);
    addAndMakeVisible (title);
    help.setText ("Set up a batch in the main window and click \"Add to Queue\" - recipe, key, count, instrument and its sound, "
                  "format, Smart Pick, takes ... are all saved with it. Then Start Queue and walk away: the batches render one "
                  "after another into their own folders. The queue is kept when you close the app. Double-click a finished "
                  "batch to Review it.", juce::dontSendNotification);
    help.setColour (juce::Label::textColourId, dim);
    help.setJustificationType (juce::Justification::topLeft);
    help.setMinimumHorizontalScale (1.0f);
    addAndMakeVisible (help);

    list.setRowHeight (26);
    list.setColour (juce::ListBox::backgroundColourId, panel);
    list.setMultipleSelectionEnabled (true);
    addAndMakeVisible (list);

    status.setColour (juce::Label::textColourId, text);
    addAndMakeVisible (status);

    startButton.setColour (juce::TextButton::buttonColourId, accent);
    startButton.onClick = [this] { if (hooks.running()) hooks.stop(); else hooks.start(); refresh(); };
    removeButton.onClick = [this]
    {
        auto& items = hooks.items();
        auto rows = list.getSelectedRows();
        for (int i = rows.size(); --i >= 0;)
            if (juce::isPositiveAndBelow (rows[i], (int) items.size()) && items[(size_t) rows[i]].status != "running")
                items.erase (items.begin() + rows[i]);
        list.deselectAllRows();
        hooks.changed();
        refresh();
    };
    upButton.onClick = [this] { move (-1); };
    downButton.onClick = [this] { move (1); };
    retryButton.setTooltip ("Queue the selected finished / failed batches again (with a new seed, so you get new samples)");
    retryButton.onClick = [this]
    {
        auto& items = hooks.items();
        auto rows = list.getSelectedRows();
        for (int i = 0; i < rows.size(); ++i)
            if (juce::isPositiveAndBelow (rows[i], (int) items.size()) && items[(size_t) rows[i]].status != "running"
                && items[(size_t) rows[i]].status != "waiting")
            {
                auto& it = items[(size_t) rows[i]];
                it.status = "waiting";
                it.result = {};
                it.request.plan.seed = juce::Random::getSystemRandom().nextInt (999999);
            }
        hooks.changed();
        refresh();
    };
    clearDoneButton.onClick = [this]
    {
        auto& items = hooks.items();
        items.erase (std::remove_if (items.begin(), items.end(), [] (const QueueItem& i)
                                     { return i.status == "done" || i.status == "failed" || i.status == "cancelled"; }), items.end());
        list.deselectAllRows();
        hooks.changed();
        refresh();
    };
    clearAllButton.onClick = [this]
    {
        if (hooks.running()) return;
        hooks.items().clear();
        list.deselectAllRows();
        hooks.changed();
        refresh();
    };
    for (auto* b : { &startButton, &removeButton, &upButton, &downButton, &retryButton, &clearDoneButton, &clearAllButton })
        addAndMakeVisible (b);

    setSize (820, 460);
    refresh();
    startTimer (700);
}

QueuePanel::~QueuePanel() { stopTimer(); }

void QueuePanel::move (int delta)
{
    auto& items = hooks.items();
    auto row = list.getSelectedRow();
    auto other = row + delta;
    if (! juce::isPositiveAndBelow (row, (int) items.size()) || ! juce::isPositiveAndBelow (other, (int) items.size())) return;
    std::swap (items[(size_t) row], items[(size_t) other]);
    list.selectRow (other);
    hooks.changed();
    refresh();
}

void QueuePanel::refresh()
{
    auto& items = hooks.items();
    int waiting = 0, done = 0, failed = 0;
    for (auto& i : items)
    {
        waiting += i.status == "waiting" || i.status == "running";
        done += i.status == "done";
        failed += i.status == "failed";
    }
    const bool running = hooks.running();
    status.setText (juce::String ((int) items.size()) + " batches: " + juce::String (waiting) + " to render, " + juce::String (done) + " done"
                    + (failed > 0 ? ", " + juce::String (failed) + " failed" : juce::String()) + (running ? "   -   rendering ..." : ""),
                    juce::dontSendNotification);
    startButton.setButtonText (running ? "Stop Queue" : "Start Queue");
    startButton.setEnabled (running || waiting > 0);
    clearAllButton.setEnabled (! running && ! items.empty());
    list.updateContent();
    list.repaint();
}

int QueuePanel::getNumRows() { return (int) hooks.items().size(); }

void QueuePanel::paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected)
{
    auto& items = hooks.items();
    if (! juce::isPositiveAndBelow (row, (int) items.size())) return;
    auto& it = items[(size_t) row];
    g.fillAll (selected ? accent.withAlpha (0.35f) : (row % 2 ? panel.brighter (0.04f) : panel));

    auto st = it.status;
    auto colour = st == "done" ? good : st == "failed" ? bad : st == "running" ? accent : dim;
    g.setColour (colour);
    g.setFont (13.0f);
    g.drawText (st.toUpperCase(), w - 96, 0, 90, h, juce::Justification::centredRight);
    g.setColour (text);
    g.drawText (juce::String (row + 1) + ".  " + it.describe(), 8, 0, w - 110, h, juce::Justification::centredLeft, true);
}

juce::String QueuePanel::getTooltipForRow (int row)
{
    auto& items = hooks.items();
    if (! juce::isPositiveAndBelow (row, (int) items.size())) return {};
    auto& it = items[(size_t) row];
    return it.result.isNotEmpty() ? it.result : it.describe();
}

void QueuePanel::listBoxItemDoubleClicked (int row, const juce::MouseEvent&)
{
    auto& items = hooks.items();
    if (juce::isPositiveAndBelow (row, (int) items.size()) && items[(size_t) row].doneFolder.isDirectory() && hooks.review)
        hooks.review (items[(size_t) row].doneFolder);
}

void QueuePanel::paint (juce::Graphics& g) { g.fillAll (bg); }

void QueuePanel::resized()
{
    auto r = getLocalBounds().reduced (14, 10);
    title.setBounds (r.removeFromTop (30));
    help.setBounds (r.removeFromTop (56));
    r.removeFromTop (6);
    auto buttons = r.removeFromBottom (32);
    startButton.setBounds (buttons.removeFromRight (130));
    buttons.removeFromRight (8);
    for (auto* b : { &removeButton, &upButton, &downButton, &retryButton, &clearDoneButton, &clearAllButton })
    {
        b->setBounds (buttons.removeFromLeft (b == &upButton || b == &downButton ? 56 : 104));
        buttons.removeFromLeft (6);
    }
    r.removeFromBottom (6);
    status.setBounds (r.removeFromBottom (24));
    list.setBounds (r);
}
}
