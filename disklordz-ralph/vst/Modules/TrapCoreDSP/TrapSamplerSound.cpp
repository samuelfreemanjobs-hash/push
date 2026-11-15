#include "TrapSamplerSound.h"

TrapSamplerSound::TrapSamplerSound(const juce::String& name,
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

bool TrapSamplerSound::appliesToNote(int midiNoteNumber)
{
    return midiNoteNumber >= noteMin && midiNoteNumber <= noteMax;
}

bool TrapSamplerSound::appliesToChannel(int) { return true; }