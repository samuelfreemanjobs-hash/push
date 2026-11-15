#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <memory>

/**
 * Per-pad sample buffers: load on message thread, read on audio thread via shared_ptr snapshot.
 * DLZ-VST-007 — pair with PadGridComponent + PluginProcessor hooks.
 * shortcut: atomic shared_ptr per pad, upgrade to fixed pool + seqlock if loading during playback
 */
class PadSampleStore
{
public:
    static constexpr int kMaxPads = 8;

    struct PadData
    {
        juce::AudioBuffer<float> buffer;
        double sampleRate = 44100.0;
        juce::MidiMessageSequence midiPattern;
    };

    juce::AudioBuffer<float>* getPadBufferForDrag(int padIndex)
    {
        if (!isPositiveAndBelow(padIndex, kMaxPads))
            return nullptr;
        return &pads[static_cast<size_t>(padIndex)].buffer;
    }

    double getPadSampleRate(int padIndex) const
    {
        if (!isPositiveAndBelow(padIndex, kMaxPads))
            return 44100.0;
        return pads[static_cast<size_t>(padIndex)].sampleRate;
    }

    juce::MidiMessageSequence getPadMidiPattern(int padIndex) const
    {
        if (!isPositiveAndBelow(padIndex, kMaxPads))
            return {};
        return pads[static_cast<size_t>(padIndex)].midiPattern;
    }

    void assignPadBuffer(int padIndex, const juce::AudioBuffer<float>& buffer, double sampleRate)
    {
        jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
        if (!isPositiveAndBelow(padIndex, kMaxPads) || buffer.getNumSamples() == 0)
            return;

        PadData next;
        next.sampleRate = sampleRate;
        next.buffer.makeCopyOf(buffer);
        pads[static_cast<size_t>(padIndex)] = std::move(next);
        publishPadSnapshot(padIndex);
    }

    void loadUserWavOntoPad(int padIndex, const juce::File& wavFile)
    {
        jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
        if (!isPositiveAndBelow(padIndex, kMaxPads) || !wavFile.existsAsFile())
            return;

        juce::AudioFormatManager manager;
        manager.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> reader(manager.createReaderFor(wavFile));
        if (reader == nullptr)
            return;

        PadData next;
        next.sampleRate = reader->sampleRate;
        next.buffer.setSize(static_cast<int>(reader->numChannels),
                            static_cast<int>(reader->lengthInSamples));
        reader->read(&next.buffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

        pads[static_cast<size_t>(padIndex)] = std::move(next);
        publishPadSnapshot(padIndex);
    }

    void setPadMidiPattern(int padIndex, juce::MidiMessageSequence sequence)
    {
        jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
        if (!isPositiveAndBelow(padIndex, kMaxPads))
            return;
        pads[static_cast<size_t>(padIndex)].midiPattern = std::move(sequence);
    }

    std::shared_ptr<const PadData> getPublishedPad(int padIndex) const
    {
        if (!isPositiveAndBelow(padIndex, kMaxPads))
            return nullptr;
        return published[static_cast<size_t>(padIndex)].load();
    }

    void setLastKnownBpm(double bpm) { lastBpm.store(bpm); }
    double getLastKnownBpm() const { return lastBpm.load(); }

private:
    void publishPadSnapshot(int padIndex)
    {
        auto snapshot = std::make_shared<PadData>(pads[static_cast<size_t>(padIndex)]);
        published[static_cast<size_t>(padIndex)].store(std::move(snapshot));
    }

    std::array<PadData, kMaxPads> pads{};
    std::array<std::atomic<std::shared_ptr<const PadData>>, kMaxPads> published{};
    std::atomic<double> lastBpm{ 140.0 };

public:
    PadSampleStore()
    {
        for (auto& slot : published)
        {
            slot.store(std::make_shared<PadData>());
        }
    }
};