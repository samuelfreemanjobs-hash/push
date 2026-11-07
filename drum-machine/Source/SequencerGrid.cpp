#include "SequencerGrid.h"

using namespace kf;

namespace
{
    constexpr uint8_t defaultVelocity = 100;
    constexpr uint8_t velocityCycle[] = { 40, 70, 100, 127 };
}

SequencerGrid::SequencerGrid (KitForgeProcessor& p) : proc (p) {}

juce::Rectangle<float> SequencerGrid::cellBounds (int track, int step) const
{
    const float cw = (float) getWidth() / (float) numSteps;
    const float ch = (float) getHeight() / (float) numTracks;
    return { step * cw, track * ch, cw, ch };
}

bool SequencerGrid::cellAt (juce::Point<int> pos, int& track, int& step) const
{
    if (! getLocalBounds().contains (pos))
        return false;
    step  = juce::jlimit (0, numSteps - 1, pos.x * numSteps / juce::jmax (1, getWidth()));
    track = juce::jlimit (0, numTracks - 1, pos.y * numTracks / juce::jmax (1, getHeight()));
    return true;
}

void SequencerGrid::paint (juce::Graphics& g)
{
    const int pattern = proc.getCurrentPattern();
    const int playing = proc.getCurrentStep();
    const int length = (int) getParamValue (proc.apvts, ids::length);

    for (int t = 0; t < numTracks; ++t)
    {
        const auto colour = trackColour (t);

        if (t == selectedTrack)
        {
            g.setColour (colour.withAlpha (0.07f));
            g.fillRect (cellBounds (t, 0).withRight ((float) getWidth()));
        }

        for (int s = 0; s < numSteps; ++s)
        {
            auto cell = cellBounds (t, s).reduced (3.0f);
            const bool beyondLength = s >= length;
            const bool beatStart = s % 4 == 0;
            const auto vel = proc.patterns.get (pattern, t, s);

            // Background
            auto bg = beatStart ? juce::Colour (0xff2b2f3a) : juce::Colour (0xff23262f);
            if (beyondLength) bg = bg.darker (0.6f);
            g.setColour (bg);
            g.fillRoundedRectangle (cell, 4.0f);

            if (vel > 0)
            {
                const float amount = (float) vel / 127.0f;
                auto c = colour.withMultipliedBrightness (0.45f + 0.55f * amount);
                if (beyondLength) c = c.withAlpha (0.3f);
                g.setColour (c);
                g.fillRoundedRectangle (cell, 4.0f);

                // velocity bar
                auto bar = cell.reduced (4.0f).removeFromBottom (3.0f);
                g.setColour (juce::Colours::black.withAlpha (0.35f));
                g.fillRect (bar);
                g.setColour (juce::Colours::white.withAlpha (0.8f));
                g.fillRect (bar.withWidth (bar.getWidth() * amount));

                if (vel >= 127)
                {
                    g.setColour (juce::Colours::white);
                    g.drawRoundedRectangle (cell.reduced (1.0f), 4.0f, 1.5f);
                }
            }

            if (s == playing)
            {
                g.setColour (juce::Colours::white.withAlpha (vel > 0 ? 0.45f : 0.12f));
                g.fillRoundedRectangle (cell, 4.0f);
            }
        }
    }
}

void SequencerGrid::mouseDown (const juce::MouseEvent& e)
{
    int t, s;
    if (! cellAt (e.getPosition(), t, s))
        return;

    const int pattern = proc.getCurrentPattern();
    const auto current = proc.patterns.get (pattern, t, s);

    if (onTrackClicked)
        onTrackClicked (t);

    if (e.mods.isAltDown() && current > 0)
    {
        dragMode = DragMode::Velocity;
        velTrack = t;
        velStep = s;
        velStartValue = current;
        return;
    }

    if (e.mods.isPopupMenu() || e.mods.isShiftDown())
    {
        uint8_t next = velocityCycle[0];
        if (current > 0)
        {
            next = velocityCycle[0];
            for (auto v : velocityCycle)
                if (v > current) { next = v; break; }
        }
        proc.patterns.set (pattern, t, s, next);
        dragMode = DragMode::None;
        repaint();
        return;
    }

    paintValue = current > 0 ? 0 : defaultVelocity;
    proc.patterns.set (pattern, t, s, paintValue);
    dragMode = DragMode::Paint;
    lastTrack = t;
    lastStep = s;
    repaint();
}

void SequencerGrid::mouseDrag (const juce::MouseEvent& e)
{
    const int pattern = proc.getCurrentPattern();

    if (dragMode == DragMode::Velocity)
    {
        const int v = juce::jlimit (1, 127, velStartValue - e.getDistanceFromDragStartY());
        proc.patterns.set (pattern, velTrack, velStep, (uint8_t) v);
        repaint();
        return;
    }

    if (dragMode != DragMode::Paint)
        return;

    int t, s;
    if (cellAt (e.getPosition(), t, s) && (t != lastTrack || s != lastStep))
    {
        proc.patterns.set (pattern, t, s, paintValue);
        lastTrack = t;
        lastStep = s;
        repaint();
    }
}

void SequencerGrid::mouseUp (const juce::MouseEvent&)
{
    dragMode = DragMode::None;
}
