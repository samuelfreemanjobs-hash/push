#pragma once

#include <JuceHeader.h>

/** Factory or user sample; load buffer on message thread only. */
class TrapSamplerSound final : public juce::SynthesiserSound
{
public:
    TrapSamplerSound(const juce::String& name,
                     juce::AudioBuffer<float> buffer,
                     double sourceSampleRate,
                     int rootMidiNote,
                     int midiNoteLow,
                     int midiNoteHigh)
        : soundName(name),
          audio(std::move(buffer)),
          sourceRate(sourceSampleRate),
          rootNote(rootMidiNote),
          noteMin(midiNoteLow),
          noteMax(midiNoteHigh)
    {
    }

    bool appliesToNote(int midiNoteNumber) override
    {
        return midiNoteNumber >= noteMin && midiNoteNumber <= noteMax;
    }

    bool appliesToChannel(int) override { return true; }

    const juce::String soundName;
    const juce::AudioBuffer<float> audio;
    const double sourceRate;
    const int rootNote;
    const int noteMin;
    const int noteMax;
};