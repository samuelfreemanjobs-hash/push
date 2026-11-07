#pragma once

#include "KitFactory.h"
#include "VintageBus.h"

// Renders a kit to a sample pack: one WAV per pad (through the vintage bus),
// an .sfz keymap, the .kitforge kit file and, if the bus has vinyl, a vinyl
// texture loop.
//
// File naming:  <Prefix>_<Type>_<Name>_A<pad>_<bits>bit.wav
//   e.g.        JazzPhonkA_Kick_JazzSubKick_A01_12bit.wav
// Levels:       kicks / snares / claps -1 dBFS, toms / rims -2 dBFS,
//               percussion -3 dBFS, hats / cymbals -4 dBFS
namespace kf::Exporter
{
struct Options
{
    double sampleRate = 44100.0;
    int bitDepth = 24;
    int firstPad = 1;          // 1 -> A01..A08, 9 -> A09..A16
    juce::String prefix;       // defaults to the kit name
    bool normalise = true;
    bool writeSfz = true;
    bool writeKitFile = true;
    bool writeVinylLoop = true;
};

struct Result
{
    juce::StringArray files;
    juce::String error;
    bool ok() const { return error.isEmpty(); }
};

float targetPeakDb (VoiceType type);
juce::String sanitise (const juce::String& s);

// Render one hit (mono) through the vintage bus, trimmed and faded.
juce::AudioBuffer<float> renderVoice (const VoiceDef& voice, const BusDef& bus, double sampleRate);

Result exportKit (const Kit& kit, const juce::File& folder, const Options& options);
} // namespace kf::Exporter
