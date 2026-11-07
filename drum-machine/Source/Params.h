#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace kf
{
constexpr int numTracks   = 8;
constexpr int numSteps    = 16;
constexpr int numPatterns = 8;
constexpr int midiBaseNote = 36; // C1 triggers track 1, C#1 track 2 ...

// What kind of drum a track is. Used by the Kit Factory to "forge" sounds and
// by the pattern generator to write sensible rhythms.
enum class VoiceType
{
    Kick, Snare, Clap, ClosedHat, OpenHat, Tom, Rim, Cymbal, Cowbell, Perc
};

const juce::StringArray& voiceTypeNames();
const juce::StringArray& waveNames();
const juce::StringArray& noiseTypeNames();
const juce::StringArray& filterModeNames();

// Per-voice synthesis parameters ("VP" = voice parameter).
enum VP
{
    Type,
    // Tone oscillator
    Wave, Pitch, PEnvAmt, PEnvDecay, ToneDecay, ToneLevel, FmAmt, FmRatio,
    // Noise generator
    NoiseType, NoiseLevel, NoiseDecay, FilterMode, Cutoff, Reso, Burst,
    // Shaping / output
    Click, Attack, Drive, Level, Pan, Choke,
    // Mixer (not part of a kit)
    Mute, Solo,
    NumVP
};

enum class Kind { Float, Choice, Bool };

struct VoiceParamSpec
{
    const char* id;
    const char* name;
    Kind kind;
    float min, max, def;
    float centre;   // skew centre (0 = linear)
    float interval; // 0 = continuous
    const char* unit;
    const juce::StringArray* choices;
};

const VoiceParamSpec& voiceSpec (int vp);

// Mute and solo are mixer state, not part of a sound design / kit.
inline bool isKitParam (int vp) { return vp != Mute && vp != Solo; }

juce::String voiceParamID (int track, int vp);

namespace ids
{
    inline constexpr const char* tempo   = "tempo";
    inline constexpr const char* swing   = "swing";
    inline constexpr const char* length  = "length";
    inline constexpr const char* pattern = "pattern";
    inline constexpr const char* master  = "master";
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

// Convenience: set a parameter by its real (denormalised) value from the message thread.
void setParamValue (juce::AudioProcessorValueTreeState& apvts, const juce::String& id, float value);
float getParamValue (juce::AudioProcessorValueTreeState& apvts, const juce::String& id);

juce::Colour trackColour (int track);
} // namespace kf
