#pragma once

#include <JuceHeader.h>

/**
 * Render one shot / loop to temp WAV and start OS drag (message thread).
 * DLZ-VST-007 — FL Channel Rack / Playlist file drop.
 */
class PadDragExport : public juce::DragAndDropContainer
{
public:
    static bool writeTempWav24(const juce::AudioBuffer<float>& audio, double sampleRate,
                               juce::File& outFile)
    {
        const auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                             .getChildFile("DetroitPhonkTrap")
                             .getNonexistentChildFile("pad", ".wav");
        dir.getParentDirectory().createDirectory();

        juce::WavAudioFormat format;
        std::unique_ptr<juce::AudioFormatWriter> writer(
            format.createWriterFor(new juce::FileOutputStream(dir), sampleRate,
                                   static_cast<unsigned int>(audio.getNumChannels()), 24, {}, 0));
        if (writer == nullptr)
            return false;
        writer->writeFromAudioSampleBuffer(audio, 0, audio.getNumSamples());
        outFile = dir;
        return true;
    }

    void beginDragExport(const juce::AudioBuffer<float>& audio, double sampleRate)
    {
        juce::File f;
        if (!writeTempWav24(audio, sampleRate, f))
            return;
        performExternalDragDropOfFiles({ f.getFullPathName() }, true);
    }
};