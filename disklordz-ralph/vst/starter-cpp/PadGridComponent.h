#pragma once

#include "KitExporter.h"
#include "MidiPatternDragExport.h"
#include "PadDragExport.h"
#include <JuceHeader.h>
#include <functional>

/**
 * Five-pad UI stub: drag WAV to FL / load user WAV onto pad.
 * Wire sample getters from PluginProcessor. DLZ-VST-007.
 */
class PadGridComponent : public juce::Component, private juce::FileDragAndDropTarget
{
public:
    using PadAudioFn = std::function<juce::AudioBuffer<float>*(int padIndex)>;
    using PadRateFn = std::function<double(int padIndex)>;
    using PatternFn = std::function<juce::MidiMessageSequence(int padIndex)>;
    using TempoFn = std::function<double()>;

    explicit PadGridComponent(juce::DragAndDropContainer& dragHost) : dragHost(dragHost) {}

    void setPadCount(int count) { padCount = juce::jlimit(1, 8, count); }
    void setGetPadAudio(PadAudioFn fn) { getPadAudio = std::move(fn); }
    void setGetPadRate(PadRateFn fn) { getPadRate = std::move(fn); }
    void setGetPattern(PatternFn fn) { getPattern = std::move(fn); }
    void setGetHostTempo(TempoFn fn) { getHostTempo = std::move(fn); }
    void setLoadPadFromFile(std::function<void(int, const juce::File&)> fn)
    {
        loadPadFromFile = std::move(fn);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff1a1a1e));
        const int w = getWidth() / padCount;
        for (int i = 0; i < padCount; ++i)
        {
            auto r = getLocalBounds().removeFromLeft(w).reduced(4);
            g.setColour(hoverPad == i ? juce::Colours::orange : juce::Colour(0xff2d2d35));
            g.fillRoundedRectangle(r.toFloat(), 6.0f);
            g.setColour(juce::Colours::white);
            g.drawFittedText("Pad " + juce::String(i + 1), r, juce::Justification::centred, 1);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        const int pad = padIndexAt(e.getPosition());
        if (pad < 0 || !getPadAudio)
            return;

        const bool midiDrag = e.mods.isShiftDown();

        if (midiDrag && getPattern)
        {
            const auto seq = getPattern(pad);
            const double bpm = getHostTempo ? getHostTempo() : 140.0;
            MidiPatternDragExport::beginDragExport(dragHost, seq, bpm);
            return;
        }

        if (auto* buf = getPadAudio(pad))
        {
            const double sr = getPadRate ? getPadRate(pad) : 44100.0;
            PadDragExport::beginDragExport(dragHost, *buf, sr);
        }
    }

    bool isInterestedInFileDrag(const juce::StringArray& files) override
    {
        for (const auto& path : files)
            if (juce::File(path).hasFileExtension("wav"))
                return true;
        return false;
    }

    void fileDragEnter(const juce::StringArray&, int, int) override { repaint(); }

    void fileDragExit(const juce::StringArray&) override
    {
        hoverPad = -1;
        repaint();
    }

    void filesDropped(const juce::StringArray& files, int x, int y) override
    {
        const int pad = padIndexAt({ x, y });
        if (pad < 0 || !loadPadFromFile)
            return;
        for (const auto& path : files)
        {
            const juce::File f(path);
            if (f.hasFileExtension("wav"))
            {
                loadPadFromFile(pad, f);
                break;
            }
        }
    }

private:
    int padIndexAt(juce::Point<int> p) const
    {
        const int w = juce::jmax(1, getWidth() / padCount);
        const int idx = p.x / w;
        return idx >= 0 && idx < padCount ? idx : -1;
    }

    juce::DragAndDropContainer& dragHost;
    int padCount = 5;
    int hoverPad = -1;
    PadAudioFn getPadAudio;
    PadRateFn getPadRate;
    PatternFn getPattern;
    TempoFn getHostTempo;
    std::function<void(int, const juce::File&)> loadPadFromFile;
};