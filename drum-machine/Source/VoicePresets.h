#pragma once

#include "KitFactory.h"

// Sound presets for each drum type.
//  - Factory: hand-designed presets for every type (Kick, 808 Sub, Snare ...)
//  - From kits: every pad of every factory kit
//  - User: presets you save, stored as .kfvoice files in
//          Documents/KitForge Voices/<Type>/
namespace kf::Presets
{
struct Preset
{
    juce::String name;
    VoiceType type;
    VoiceDef voice;
    juce::String source; // "Factory", the kit name, or "User"
};

const std::vector<Preset>& factory();            // curated presets, all types
std::vector<Preset> fromKits();                  // pads of the factory kits
std::vector<Preset> user();                      // saved user presets
std::vector<Preset> all();                       // factory + kits + user

std::vector<Preset> forType (const std::vector<Preset>& list, VoiceType type);
const Preset* findFactory (const juce::String& name);

juce::File userFolder();
bool saveUser (const juce::String& name, const VoiceDef& voice, juce::File* written = nullptr);
std::unique_ptr<juce::XmlElement> voiceToXml (const juce::String& name, const VoiceDef& voice);
bool voiceFromXml (const juce::XmlElement& xml, Preset& out);
} // namespace kf::Presets
