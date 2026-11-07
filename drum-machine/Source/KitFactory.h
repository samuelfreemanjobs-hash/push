#pragma once

#include "Params.h"
#include "Pattern.h"
#include <array>

namespace kf
{
// A full sound design for one voice, in real parameter units (indexed by VP).
using VoiceDef = std::array<float, NumVP>;

struct Kit
{
    juce::String name;
    std::array<VoiceDef, numTracks> voices;
};

// The Kit Factory: builds drum kits out of the synth engine.
//  - factory kits: hand-designed starting points
//  - forge: generate a brand new voice / kit from a drum type's "recipe"
//  - mutate: nudge an existing kit in a random direction
//  - save / load kits as .kitforge (XML) files
namespace KitFactory
{
    VoiceDef defaultVoice();

    const juce::StringArray& factoryKitNames();
    Kit factoryKit (int index);

    VoiceDef forgeVoice (VoiceType type, juce::Random& rng);
    Kit forgeKit (juce::Random& rng);

    // Read / write the kit currently loaded in the processor's parameters.
    Kit captureKit (juce::AudioProcessorValueTreeState& apvts, const juce::String& name);
    void applyKit (juce::AudioProcessorValueTreeState& apvts, const Kit& kit);
    void applyVoice (juce::AudioProcessorValueTreeState& apvts, int track, const VoiceDef& voice);

    void mutateKit (juce::AudioProcessorValueTreeState& apvts, juce::Random& rng, float amount);

    std::unique_ptr<juce::XmlElement> kitToXml (const Kit& kit);
    bool kitFromXml (const juce::XmlElement& xml, Kit& kit);

    // Write a rhythm into one pattern that suits each track's drum type.
    void generatePattern (PatternBank& bank, int pattern,
                          juce::AudioProcessorValueTreeState& apvts, juce::Random& rng);

    // A simple groove so the machine makes sound out of the box.
    void writeDefaultPatterns (PatternBank& bank);
}
} // namespace kf
