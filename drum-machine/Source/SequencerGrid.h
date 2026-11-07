#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

// The 16-pad x 16-step grid.
//  - Left click toggles a step (drag to paint / erase across steps)
//  - Right click (or shift-click) cycles velocity: soft -> medium -> normal -> accent
//  - Alt/Option-drag vertically on a step to set its velocity freely
//  - Ctrl/Cmd-click (or any click in Roll mode) cycles the step's ratchet:
//    x1 -> x2 (1/32) -> x3 (1/16 triplet roll) -> x4 (1/64) -> x1
class SequencerGrid : public juce::Component
{
public:
    explicit SequencerGrid (KitForgeProcessor&);

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

    void setSelectedTrack (int t) { selectedTrack = t; repaint(); }
    void setRollMode (bool on) { rollMode = on; repaint(); }
    int getRowHeight() const { return getHeight() / kf::numTracks; }

    std::function<void (int track)> onTrackClicked;

private:
    bool cellAt (juce::Point<int> pos, int& track, int& step) const;
    juce::Rectangle<float> cellBounds (int track, int step) const;

    KitForgeProcessor& proc;
    int selectedTrack = 0;
    bool rollMode = false;

    enum class DragMode { None, Paint, Velocity };
    DragMode dragMode = DragMode::None;
    uint8_t paintValue = 0;
    int velTrack = -1, velStep = -1, velStartValue = 0;
    int lastTrack = -1, lastStep = -1;
};
