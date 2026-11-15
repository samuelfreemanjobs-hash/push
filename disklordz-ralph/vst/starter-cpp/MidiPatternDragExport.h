#pragma once

#include <JuceHeader.h>

/**
 * Export captured MIDI (e.g. Triggerman roll pattern) as a temp .mid for FL Playlist drop.
 * DLZ-VST-007 — call from message thread after recording a pattern into MidiMessageSequence.
 */
class MidiPatternDragExport
{
public:
    static bool writeTempMidi(const juce::MidiMessageSequence& sequence, double tempoBpm,
                              juce::File& outFile)
    {
        if (sequence.getNumEvents() == 0)
            return false;

        juce::MidiFile midiFile;
        midiFile.setTicksPerQuarterNote(960);

        juce::MidiMessageSequence track(sequence);
        track.addEvent(juce::MidiMessage::tempoMetaEvent(
            static_cast<int>(60.0 * 1'000'000.0 / juce::jmax(1.0, tempoBpm))));

        midiFile.addTrack(track);

        const auto dest = juce::File::getSpecialLocation(juce::File::tempDirectory)
                              .getChildFile("DetroitPhonkTrap")
                              .getNonexistentChildFile("pattern", ".mid");
        dest.getParentDirectory().createDirectory();

        juce::FileOutputStream stream(dest);
        if (!stream.openedOk())
            return false;
        midiFile.writeTo(stream);
        outFile = dest;
        return true;
    }

    static void beginDragExport(juce::DragAndDropContainer& host,
                                const juce::MidiMessageSequence& sequence, double tempoBpm)
    {
        juce::File f;
        if (!writeTempMidi(sequence, tempoBpm, f))
            return;
        host.performExternalDragDropOfFiles({ f.getFullPathName() }, true);
    }
};