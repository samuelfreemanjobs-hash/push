#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace md::pid
{
// Global
inline constexpr auto inputGain  = "inputGain";
inline constexpr auto outputGain = "outputGain";
inline constexpr auto mix        = "mix";
inline constexpr auto outClip    = "outClip";
inline constexpr auto autoLevel  = "autoLevel";
// 01 Sampler
inline constexpr auto crushOn   = "crushOn";
inline constexpr auto crushRate = "crushRate";
inline constexpr auto crushBits = "crushBits";
inline constexpr auto crushAA   = "crushAA";
// 02 Cassette EQ
inline constexpr auto eqOn     = "eqOn";
inline constexpr auto lowCut   = "lowCut";
inline constexpr auto boxFreq  = "boxFreq";
inline constexpr auto boxGain  = "boxGain";
inline constexpr auto boxQ     = "boxQ";
inline constexpr auto highCut  = "highCut";
inline constexpr auto presence = "presence";
// 03 Tape
inline constexpr auto tapeOn    = "tapeOn";
inline constexpr auto tapeDrive = "tapeDrive";
inline constexpr auto tapeBias  = "tapeBias";
inline constexpr auto wow       = "wow";
inline constexpr auto flutter   = "flutter";
inline constexpr auto hiss      = "hiss";
inline constexpr auto hissDuck  = "hissDuck";
inline constexpr auto dropouts  = "dropouts";
// 04 dbx Type II NR
inline constexpr auto dbxOn      = "dbxOn";
inline constexpr auto dbxBreath  = "dbxBreath";
inline constexpr auto dbxRelease = "dbxRelease";
inline constexpr auto dbxTilt    = "dbxTilt";
// 04 FET comp
inline constexpr auto compOn      = "compOn";
inline constexpr auto compThresh  = "compThresh";
inline constexpr auto compRatio   = "compRatio";
inline constexpr auto compAttack  = "compAttack";
inline constexpr auto compRelease = "compRelease";
inline constexpr auto compMakeup  = "compMakeup";
inline constexpr auto compMix     = "compMix";
inline constexpr auto compMode    = "compMode";
// 05 808 clip
inline constexpr auto subOn    = "subOn";
inline constexpr auto subFreq  = "subFreq";
inline constexpr auto subDrive = "subDrive";
inline constexpr auto subHard  = "subHard";
inline constexpr auto subLevel = "subLevel";
// 07 Master
inline constexpr auto monoOn       = "monoOn";
inline constexpr auto monoFreq     = "monoFreq";
inline constexpr auto limitOn      = "limitOn";
inline constexpr auto limitDrive   = "limitDrive";
inline constexpr auto limitCeiling = "limitCeiling";
} // namespace md::pid

namespace md
{
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

struct Preset
{
    const char* name;
    std::vector<std::pair<const char*, float>> values; // plain (denormalised) values
};

const std::vector<Preset>& getFactoryPresets();
} // namespace md
