#include "VoiceDSP.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <cstdlib>
#include <iostream>
#include <memory>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: HeadlessRender <output.wav> [velocity 0-1] [midi-note]\n";
        return 1;
    }

    const double sampleRate = 44100.0;
    const int numSamples = static_cast<int>(sampleRate * 1.1);
    const float velocity = argc >= 3 ? static_cast<float>(std::atof(argv[2])) : 1.0f;
    const int midiNote = argc >= 4 ? std::atoi(argv[3]) : 36;

    juce::AudioBuffer<float> buffer(1, numSamples);
    buffer.clear();

    VoiceDSP dsp;
    dsp.prepare(sampleRate, 512);
    dsp.triggerNote(midiNote, velocity);
    dsp.processBlock(buffer);

    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        peak = std::max(peak, std::abs(buffer.getSample(0, i)));
    if (peak > 1.0e-6f)
        buffer.applyGain(0.95f / peak);

    juce::File outFile(juce::File::getCurrentWorkingDirectory().getChildFile(argv[1]));
    outFile.getParentDirectory().createDirectory();
    outFile.deleteFile();

    juce::WavAudioFormat wav;
    if (auto stream = std::unique_ptr<juce::FileOutputStream>(outFile.createOutputStream()))
    {
        if (auto writer = std::unique_ptr<juce::AudioFormatWriter>(
                wav.createWriterFor(stream.release(), sampleRate, 1, 24, {}, 0)))
        {
            writer->writeFromAudioSampleBuffer(buffer, 0, numSamples);
            std::cout << "Wrote " << outFile.getFullPathName() << "\n";
            return 0;
        }
    }

    std::cerr << "Failed to write WAV\n";
    return 2;
}