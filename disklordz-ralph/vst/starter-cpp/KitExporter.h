#pragma once

#include <JuceHeader.h>

/**
 * Export all loaded pad buffers to a folder (message thread). DLZ-VST-007 kit zip handoff.
 */
class KitExporter
{
public:
    struct PadSample
    {
        juce::String padName;
        const juce::AudioBuffer<float>* buffer = nullptr;
        double sampleRate = 44100.0;
    };

    static juce::File defaultExportDir()
    {
        return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("DetroitPhonkTrap")
            .getChildFile("ExportedKits");
    }

    static int exportPadsToFolder(const juce::Array<PadSample>& pads, juce::File destDir,
                                  int bitDepth = 24)
    {
        if (!destDir.createDirectory())
            return 0;

        juce::WavAudioFormat format;
        int written = 0;

        for (const auto& pad : pads)
        {
            if (pad.buffer == nullptr || pad.buffer->getNumSamples() == 0)
                continue;

            const auto safeName =
                pad.padName.isNotEmpty() ? pad.padName : juce::String("pad_") + juce::String(written);
            const auto out = destDir.getNonexistentChildFile(safeName, ".wav");

            std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(
                new juce::FileOutputStream(out), pad.sampleRate,
                static_cast<unsigned int>(pad.buffer->getNumChannels()), bitDepth, {}, 0));
            if (writer == nullptr)
                continue;

            writer->writeFromAudioSampleBuffer(*pad.buffer, 0, pad.buffer->getNumSamples());
            ++written;
        }

        return written;
    }
};