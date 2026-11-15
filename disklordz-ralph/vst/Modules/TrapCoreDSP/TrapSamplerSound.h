#pragma once

#include <JuceHeader.h>

class TrapSamplerSound final : public juce::SynthesiserSound
{
public:
    TrapSamplerSound(const juce::String& name,
                     juce::AudioBuffer<float> buffer,
                     double sourceSampleRate,
                     int rootMidiNote,
                     int midiNoteLow,
                     int midiNoteHigh);

    bool appliesToNote(int midiNoteNumber) override;
    bool appliesToChannel(int) override;

    const juce::String soundName;
    const juce::AudioBuffer<float> audio;
    const double sourceRate;
    const int rootNote;
    const int noteMin;
    const int noteMax;
};