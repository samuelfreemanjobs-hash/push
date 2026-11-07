#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace kf
{
constexpr int numTracks   = 16;  // pads A01..A16
constexpr int numSteps    = 16;
constexpr int numPatterns = 8;
constexpr int midiBaseNote = 36; // C1 triggers pad 1 ... D#2 (51) pad 16
constexpr int maxRatchet = 4;
constexpr int maxStepPitch = 24; // per-step pitch offset range, +/- semitones    // step repeats: 1 = normal, 2 = 1/32, 3 = 1/16 triplet-roll, 4 = 1/64

// What kind of drum a track is. Used by the Kit Factory to "forge" sounds and
// by the pattern generator to write sensible rhythms.
enum class VoiceType
{
    Kick, Snare, Clap, ClosedHat, OpenHat, Tom, Rim, Cymbal, Cowbell, Perc, Sub808, Shaker
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
    Wave, Pitch, PEnvAmt, PEnvDecay, ToneDecay, Hold, Glide, ToneLevel, FmAmt, FmRatio, FmDecay,
    // Noise generator
    NoiseType, NoiseLevel, NoiseDecay, FilterMode, Cutoff, Reso, Burst,
    // Shaping / output
    Click, Attack, Punch, PunchTime, Drive, Level, Pan, Choke, LoFiSend,
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

// Vintage Sampler Bus parameters (global). The drum send of every track runs
// through: drive -> bit reduction -> decimation (with the pitch-up/down alias
// trick) -> 2-pole low-pass, with a crossover that keeps the lows clean. The
// whole mix then gets a transient shaper (punch / tail) and a vinyl layer.
enum BP
{
    BDrive, BBits, BRate, BPitch, BLpf, BLowKeep, BPunch, BTail, BVinyl,
    NumBP
};

const VoiceParamSpec& busSpec (int bp);
juce::String busParamID (int bp);

// Machine presets for the bus ("SP-1200 style" etc.)
const juce::StringArray& machineNames();
std::array<float, NumBP> machinePreset (int index); // punch / tail are left at 0
// Which bus params a machine preset sets (punch / tail are left alone).
inline bool isMachineParam (int bp) { return bp != BPunch && bp != BTail; }

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
