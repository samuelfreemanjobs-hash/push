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
            auto cell = cellBounds (t, s).reduced (2.5f, 2.0f);
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

            // Ratchet / roll: split the cell into its repeats
            if (const int ratchet = proc.patterns.getRatchet (pattern, t, s); ratchet > 1 && vel > 0)
            {
                g.setColour (juce::Colours::black.withAlpha (0.55f));
                for (int i = 1; i < ratchet; ++i)
                {
                    const float x = cell.getX() + cell.getWidth() * (float) i / (float) ratchet;
                    g.fillRect (x - 1.0f, cell.getY() + 2.0f, 2.0f, cell.getHeight() - 8.0f);
                }
                g.setColour (juce::Colours::white);
                g.setFont (juce::Font (juce::FontOptions (9.5f, juce::Font::bold)));
                g.drawText ("x" + juce::String (ratchet), cell.reduced (3.0f, 1.0f), juce::Justification::topRight);
            }
            else if (rollMode && vel > 0)
            {
                g.setColour (juce::Colours::white.withAlpha (0.5f));
                g.setFont (juce::Font (juce::FontOptions (9.5f, juce::Font::bold)));
                g.drawText ("x1", cell.reduced (3.0f, 1.0f), juce::Justification::topRight);
            }

            // Per-step pitch
            if (const int st = proc.patterns.getPitch (pattern, t, s); vel > 0 && (st != 0 || pitchMode))
            {
                g.setColour (st != 0 ? juce::Colours::white : juce::Colours::white.withAlpha (0.45f));
                g.setFont (juce::Font (juce::FontOptions (9.5f, juce::Font::bold)));
                g.drawText ((st > 0 ? "+" : "") + juce::String (st), cell.reduced (4.0f, 1.0f), juce::Justification::topLeft);
            }

            if (s == playing)
            {
                g.setColour (juce::Colours::white.withAlpha (vel > 0 ? 0.45f : 0.12f));
                g.fillRoundedRectangle (cell, 4.0f);
            }
        }
    }
}

void SequencerGrid::paintOverChildren (juce::Graphics& g)
{
    // Divider between pad bank 1 (A01-A08) and bank 2 (A09-A16)
    const float y = cellBounds (8, 0).getY();
    g.setColour (juce::Colours::white.withAlpha (0.18f));
    g.drawHorizontalLine ((int) y, 0.0f, (float) getWidth());
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

    if (pitchMode)
    {
        if (e.mods.isPopupMenu())
        {
            proc.patterns.setPitch (pattern, t, s, 0);
            dragMode = DragMode::None;
        }
        else
        {
            if (current == 0)
                proc.patterns.set (pattern, t, s, defaultVelocity);
            dragMode = DragMode::Pitch;
            velTrack = t;
            velStep = s;
            pitchStartValue = proc.patterns.getPitch (pattern, t, s);
        }
        repaint();
        return;
    }

    if (rollMode || e.mods.isCommandDown())
    {
        // Cycle the roll. An empty step is switched on with a x2 roll.
        if (current == 0)
        {
            proc.patterns.set (pattern, t, s, defaultVelocity);
            proc.patterns.setRatchet (pattern, t, s, 2);
        }
        else
        {
            const int r = proc.patterns.getRatchet (pattern, t, s);
            proc.patterns.setRatchet (pattern, t, s, r >= maxRatchet ? 1 : r + 1);
        }
        dragMode = DragMode::None;
        repaint();
        return;
    }

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
    if (paintValue == 0)
    {
        proc.patterns.setRatchet (pattern, t, s, 1);
        proc.patterns.setPitch (pattern, t, s, 0);
    }
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

    if (dragMode == DragMode::Pitch)
    {
        // 6 pixels per semitone
        const int st = pitchStartValue - e.getDistanceFromDragStartY() / 6;
        if (st != proc.patterns.getPitch (pattern, velTrack, velStep))
        {
            proc.patterns.setPitch (pattern, velTrack, velStep, st);
            repaint();
        }
        return;
    }

    if (dragMode != DragMode::Paint)
        return;

    int t, s;
    if (cellAt (e.getPosition(), t, s) && (t != lastTrack || s != lastStep))
    {
        proc.patterns.set (pattern, t, s, paintValue);
        if (paintValue == 0)
        {
            proc.patterns.setRatchet (pattern, t, s, 1);
            proc.patterns.setPitch (pattern, t, s, 0);
        }
        lastTrack = t;
        lastStep = s;
        repaint();
    }
}

void SequencerGrid::mouseUp (const juce::MouseEvent& e)
{
    // In pitch mode, audition the new note when the drag ends.
    if (dragMode == DragMode::Pitch && e.getDistanceFromDragStart() > 2)
        proc.previewTrack (velTrack, 0.9f, (float) proc.patterns.getPitch (proc.getCurrentPattern(), velTrack, velStep));
    dragMode = DragMode::None;
}

void SequencerGrid::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    int t, s;
    const int pattern = proc.getCurrentPattern();
    if (! cellAt (e.getPosition(), t, s) || proc.patterns.get (pattern, t, s) == 0 || std::abs (wheel.deltaY) < 1.0e-4f)
        return;
    const int st = proc.patterns.getPitch (pattern, t, s) + (wheel.deltaY > 0 ? 1 : -1);
    proc.patterns.setPitch (pattern, t, s, st);
    proc.previewTrack (t, 0.9f, (float) proc.patterns.getPitch (pattern, t, s));
    repaint();
}
