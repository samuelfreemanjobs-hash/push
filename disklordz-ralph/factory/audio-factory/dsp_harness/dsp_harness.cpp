#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

#include "dsp/VintageSaturator.h"

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        std::cerr << "Usage: dsp_harness <input.wav> <output.wav> <drive_0_to_1>\n";
        return 1;
    }

    juce::ScopedJuceInitialiser_GUI juceInit;

    const juce::File inputFile(argv[1]);
    const juce::File outputFile(argv[2]);
    const float drive = juce::jlimit(0.0f, 1.0f, std::stof(argv[3]));

    if (!inputFile.existsAsFile())
    {
        std::cerr << "Error: Input file not found: " << inputFile.getFullPathName() << "\n";
        return 1;
    }

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(inputFile));
    if (reader == nullptr)
    {
        std::cerr << "Error: Could not read WAV format.\n";
        return 1;
    }

    const int numChannels = static_cast<int>(reader->numChannels);
    const int totalSamples = static_cast<int>(reader->lengthInSamples);
    const double sampleRate = reader->sampleRate;
    constexpr int blockSize = 512;

    juce::AudioBuffer<float> fullBuffer(numChannels, totalSamples);
    reader->read(&fullBuffer, 0, totalSamples, 0, true, true);

    VintageSaturator saturator;
    saturator.prepare({ sampleRate, static_cast<juce::uint32>(blockSize), static_cast<juce::uint32>(numChannels) });
    saturator.setDrive(drive);

    for (int offset = 0; offset < totalSamples; offset += blockSize)
    {
        const int numThisBlock = std::min(blockSize, totalSamples - offset);
        juce::AudioBuffer<float> blockView(
            fullBuffer.getArrayOfWritePointers(), numChannels, offset, numThisBlock);
        saturator.process(blockView);
    }

    outputFile.deleteFile();
    auto outStream = std::unique_ptr<juce::FileOutputStream>(outputFile.createOutputStream());
    if (outStream == nullptr)
    {
        std::cerr << "Error: Cannot open output stream.\n";
        return 1;
    }

    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer(wavFormat.createWriterFor(
        outStream.get(),
        sampleRate,
        static_cast<unsigned int>(numChannels),
        24,
        {},
        0));

    if (writer == nullptr)
    {
        std::cerr << "Error: Failed to create WAV writer.\n";
        return 1;
    }

    outStream.release();
    writer->writeFromAudioSampleBuffer(fullBuffer, 0, totalSamples);
    writer.reset();

    std::cout << "Rendered " << totalSamples << " samples at drive=" << drive << "\n";
    return 0;
}