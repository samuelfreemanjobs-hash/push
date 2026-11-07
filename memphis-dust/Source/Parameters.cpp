#include "Parameters.h"

namespace md
{
namespace
{
using APF = juce::AudioParameterFloat;
using APB = juce::AudioParameterBool;
using APC = juce::AudioParameterChoice;

juce::NormalisableRange<float> skewed (float lo, float hi, float centre, float interval = 0.0f)
{
    juce::NormalisableRange<float> r (lo, hi, interval);
    r.setSkewForCentre (centre);
    return r;
}

auto attrs (std::function<juce::String (float, int)> toText, const juce::String& label = {})
{
    return juce::AudioParameterFloatAttributes().withStringFromValueFunction (std::move (toText)).withLabel (label);
}

juce::String hzText (float v, int)
{
    return v >= 1000.0f ? juce::String (v / 1000.0f, v >= 10000.0f ? 1 : 2) + " kHz" : juce::String ((int) std::round (v)) + " Hz";
}
juce::String dbText (float v, int)   { return (v > 0.0f ? "+" : "") + juce::String (v, 1) + " dB"; }
juce::String pctText (float v, int)  { return juce::String ((int) std::round (v * 100.0f)) + "%"; }
juce::String msText (float v, int)
{
    return v < 1.0f ? juce::String ((int) std::round (v * 1000.0f)) + " us" : juce::String (v, v < 10.0f ? 1 : 0) + " ms";
}

std::unique_ptr<APF> fp (const char* id, const char* name, juce::NormalisableRange<float> r, float def,
                         std::function<juce::String (float, int)> toText)
{
    return std::make_unique<APF> (juce::ParameterID { id, 1 }, name, r, def, attrs (std::move (toText)));
}
std::unique_ptr<APB> bp (const char* id, const char* name, bool def)
{
    return std::make_unique<APB> (juce::ParameterID { id, 1 }, name, def);
}
} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Global
    layout.add (fp (pid::inputGain,  "Input",  { -24.0f, 24.0f, 0.1f }, 0.0f, dbText));
    layout.add (fp (pid::outputGain, "Output", { -24.0f, 24.0f, 0.1f }, 0.0f, dbText));
    layout.add (fp (pid::mix,        "Mix",    { 0.0f, 1.0f, 0.01f }, 1.0f, pctText));
    layout.add (bp (pid::outClip,    "Output Clip", true));
    layout.add (bp (pid::autoLevel,  "Auto Level", false));

    // 01 Sampler
    layout.add (bp (pid::crushOn, "Sampler On", true));
    layout.add (fp (pid::crushRate, "Sample Rate", skewed (2000.0f, 48000.0f, 16000.0f, 1.0f), 22050.0f, hzText));
    layout.add (fp (pid::crushBits, "Bit Depth", { 4.0f, 16.0f, 0.1f }, 12.0f,
                    [] (float v, int) { return juce::String (v, 1) + " bit"; }));
    layout.add (bp (pid::crushAA, "Anti-Alias", false));

    // 02 Cassette EQ
    layout.add (bp (pid::eqOn, "EQ On", true));
    layout.add (fp (pid::lowCut,  "Low Cut",  skewed (20.0f, 300.0f, 60.0f, 1.0f), 40.0f, hzText));
    layout.add (fp (pid::boxFreq, "Box Freq", skewed (150.0f, 600.0f, 300.0f, 1.0f), 300.0f, hzText));
    layout.add (fp (pid::boxGain, "Box Gain", { -6.0f, 12.0f, 0.1f }, 4.0f, dbText));
    layout.add (fp (pid::boxQ,    "Box Q",    skewed (0.5f, 6.0f, 1.8f, 0.01f), 2.0f,
                    [] (float v, int) { return juce::String (v, 2); }));
    layout.add (fp (pid::highCut, "High Cut", skewed (3000.0f, 20000.0f, 10000.0f, 1.0f), 11000.0f, hzText));
    layout.add (fp (pid::presence, "Presence", { -6.0f, 3.0f, 0.1f }, -1.5f, dbText));

    // 03 Tape
    layout.add (bp (pid::tapeOn, "Tape On", true));
    layout.add (fp (pid::tapeDrive, "Tape Drive", { 0.0f, 24.0f, 0.1f }, 9.0f, dbText));
    layout.add (fp (pid::tapeBias,  "Tape Bias",  { 0.0f, 1.0f, 0.01f }, 0.3f, pctText));
    layout.add (fp (pid::wow,       "Wow",        { 0.0f, 1.0f, 0.01f }, 0.15f, pctText));
    layout.add (fp (pid::flutter,   "Flutter",    { 0.0f, 1.0f, 0.01f }, 0.2f, pctText));
    layout.add (fp (pid::hiss,      "Hiss",       { -90.0f, -30.0f, 0.1f }, -64.0f,
                    [] (float v, int) { return v <= -89.9f ? juce::String ("Off") : juce::String (v, 1) + " dB"; }));
    layout.add (fp (pid::hissDuck,  "Hiss Duck",  { 0.0f, 1.0f, 0.01f }, 0.0f, pctText));
    layout.add (fp (pid::dropouts,  "Dropouts",   { 0.0f, 1.0f, 0.01f }, 0.05f, pctText));

    // 04 dbx Type II
    layout.add (bp (pid::dbxOn, "dbx NR On", true));
    layout.add (fp (pid::dbxBreath,  "dbx Breath",  { 0.0f, 1.0f, 0.01f }, 0.3f, pctText));
    layout.add (fp (pid::dbxRelease, "dbx Release", skewed (30.0f, 400.0f, 120.0f, 1.0f), 120.0f, msText));
    layout.add (fp (pid::dbxTilt,    "dbx Tilt",    { 0.0f, 1.0f, 0.01f }, 0.3f, pctText));

    // 04 FET comp
    layout.add (bp (pid::compOn, "Comp On", true));
    layout.add (fp (pid::compThresh, "Threshold", { -40.0f, 0.0f, 0.1f }, -18.0f, dbText));
    layout.add (std::make_unique<APC> (juce::ParameterID { pid::compRatio, 1 }, "Ratio",
                                       juce::StringArray { "4:1", "8:1", "12:1", "20:1", "ALL" }, 1));
    layout.add (fp (pid::compAttack,  "Attack",  skewed (0.02f, 30.0f, 1.0f, 0.01f), 0.1f, msText));
    layout.add (fp (pid::compRelease, "Release", skewed (20.0f, 1100.0f, 150.0f, 1.0f), 60.0f, msText));
    layout.add (fp (pid::compMakeup,  "Makeup",  { 0.0f, 24.0f, 0.1f }, 9.0f, dbText));
    layout.add (fp (pid::compMix,     "Comp Mix", { 0.0f, 1.0f, 0.01f }, 1.0f, pctText));
    layout.add (std::make_unique<APC> (juce::ParameterID { pid::compMode, 1 }, "Comp Mode",
                                       juce::StringArray { "FET", "VCA" }, 0));

    // 05 808 clip
    layout.add (bp (pid::subOn, "808 Clip On", true));
    layout.add (fp (pid::subFreq,  "Sub Freq",  skewed (60.0f, 250.0f, 120.0f, 1.0f), 120.0f, hzText));
    layout.add (fp (pid::subDrive, "Sub Drive", { 0.0f, 30.0f, 0.1f }, 12.0f, dbText));
    layout.add (fp (pid::subHard,  "Hardness",  { 0.0f, 1.0f, 0.01f }, 0.6f, pctText));
    layout.add (fp (pid::subLevel, "Sub Level", { -24.0f, 12.0f, 0.1f }, 0.0f, dbText));

    // 07 Master
    layout.add (bp (pid::monoOn, "Mono Bass", false));
    layout.add (fp (pid::monoFreq, "Mono Below", skewed (40.0f, 300.0f, 120.0f, 1.0f), 120.0f, hzText));
    layout.add (bp (pid::limitOn, "Limiter", false));
    layout.add (fp (pid::limitDrive, "Limit Drive", { 0.0f, 12.0f, 0.1f }, 3.0f, dbText));
    layout.add (fp (pid::limitCeiling, "Ceiling", { -3.0f, 0.0f, 0.1f }, -0.3f, dbText));

    return layout;
}

const std::vector<Preset>& getFactoryPresets()
{
    // Anything not listed falls back to the parameter default.
    static const std::vector<Preset> presets {
        { "Memphis '94 Bus", {} },
        { "Portastudio Slam", {
            { pid::crushRate, 16000 }, { pid::crushBits, 10 }, { pid::highCut, 8500 }, { pid::boxGain, 6 },
            { pid::boxFreq, 280 }, { pid::tapeDrive, 15 }, { pid::tapeBias, 0.45f }, { pid::wow, 0.3f },
            { pid::flutter, 0.35f }, { pid::hiss, -54 }, { pid::compThresh, -26 }, { pid::compRatio, 2 },
            { pid::compMakeup, 14 }, { pid::compRelease, 50 }, { pid::outputGain, 3 } } },
        { "SP Crunch (26k/12)", {
            { pid::crushRate, 26040 }, { pid::crushBits, 12 }, { pid::highCut, 13000 }, { pid::boxGain, 3 },
            { pid::tapeDrive, 6 }, { pid::wow, 0.05f }, { pid::flutter, 0.1f }, { pid::hiss, -80 }, { pid::dbxOn, 0 },
            { pid::compThresh, -14 }, { pid::compRatio, 0 }, { pid::compMakeup, 6 }, { pid::subDrive, 8 } } },
        { "8-Bit Basement", {
            { pid::crushRate, 11025 }, { pid::crushBits, 8 }, { pid::crushAA, 1 }, { pid::highCut, 7000 },
            { pid::boxFreq, 350 }, { pid::boxGain, 7 }, { pid::boxQ, 2.5f }, { pid::lowCut, 45 },
            { pid::tapeDrive, 18 }, { pid::tapeBias, 0.6f }, { pid::wow, 0.45f }, { pid::flutter, 0.4f },
            { pid::hiss, -50 }, { pid::compThresh, -24 }, { pid::compRatio, 4 },
            { pid::compMakeup, 15 }, { pid::compMix, 0.85f }, { pid::outputGain, 3.5f } } },
        { "808 Car Knock", {
            { pid::crushRate, 32000 }, { pid::crushBits, 14 }, { pid::boxGain, 2 }, { pid::tapeDrive, 6 },
            { pid::wow, 0.08f }, { pid::flutter, 0.1f }, { pid::hiss, -72 }, { pid::subFreq, 110 },
            { pid::subDrive, 22 }, { pid::subHard, 0.85f }, { pid::subLevel, 3 }, { pid::compThresh, -14 },
            { pid::compRatio, 0 }, { pid::compMakeup, 5 } } },
        { "Parallel Smash", {
            { pid::compRatio, 4 }, { pid::compThresh, -30 }, { pid::compMakeup, 12 }, { pid::compMix, 0.45f }, { pid::outputGain, 2 },
            { pid::tapeDrive, 12 } } },
        // From the RC-20 / stock-chain recipe: no digital stage, 45 Hz-11.5 kHz head bandwidth,
        // broad +2 dB @ 250 Hz, -1.5 dB @ 3.5 kHz, flutter 40 / wow 25 / dropouts 15, ducked hiss,
        // 4:1 comp with fast attack and ~120 ms release for 4-6 dB of pumping.
        { "Portastudio 424", {
            { pid::crushOn, 0 }, { pid::lowCut, 45 }, { pid::highCut, 11500 }, { pid::boxFreq, 250 },
            { pid::boxGain, 2 }, { pid::boxQ, 0.8f }, { pid::presence, -1.5f }, { pid::tapeDrive, 5 },
            { pid::tapeBias, 0.35f }, { pid::wow, 0.25f }, { pid::flutter, 0.4f }, { pid::dropouts, 0.15f },
            { pid::hiss, -52 }, { pid::hissDuck, 0.6f }, { pid::dbxBreath, 0.35f }, { pid::dbxTilt, 0.35f },
            { pid::compRatio, 0 }, { pid::compAttack, 3 }, { pid::compRelease, 120 }, { pid::compThresh, -18 },
            { pid::compMakeup, 5 }, { pid::subOn, 0 }, { pid::autoLevel, 1 } } },
        { "dbx Breather (mistracked)", {
            { pid::crushOn, 0 }, { pid::lowCut, 45 }, { pid::highCut, 10500 }, { pid::boxFreq, 250 },
            { pid::boxGain, 2 }, { pid::boxQ, 0.8f }, { pid::tapeDrive, 6 }, { pid::wow, 0.2f },
            { pid::flutter, 0.3f }, { pid::dropouts, 0.1f }, { pid::hiss, -46 }, { pid::dbxBreath, 0.85f },
            { pid::dbxRelease, 160 }, { pid::dbxTilt, 0.8f }, { pid::compRatio, 1 }, { pid::compAttack, 4 },
            { pid::compRelease, 100 }, { pid::compThresh, -20 }, { pid::compMakeup, 6 }, { pid::subOn, 0 },
            { pid::autoLevel, 1 } } },
        { "Dubbed Tape Master", {
            { pid::crushOn, 0 }, { pid::lowCut, 35 }, { pid::highCut, 12500 }, { pid::boxFreq, 250 },
            { pid::boxGain, 1.5f }, { pid::boxQ, 0.7f }, { pid::presence, -1 }, { pid::tapeDrive, 4 },
            { pid::tapeBias, 0.25f }, { pid::wow, 0.12f }, { pid::flutter, 0.2f }, { pid::dropouts, 0.03f },
            { pid::hiss, -60 }, { pid::hissDuck, 0.3f }, { pid::dbxBreath, 0.15f }, { pid::dbxTilt, 0.2f },
            { pid::compRatio, 0 }, { pid::compAttack, 5 }, { pid::compRelease, 150 }, { pid::compThresh, -12 },
            { pid::compMakeup, 2 }, { pid::compMode, 1 }, { pid::subOn, 0 }, { pid::monoOn, 1 }, { pid::autoLevel, 1 } } },
        // The 6-step master bus chain: HPF 32 Hz + mono < 120 Hz -> gentle tape (+3 dB) -> 12 kHz LPF,
        // wide +1.75 dB @ 300 Hz, narrow -1.5 dB @ 3.5 kHz -> VCA 4:1 / 10 ms / 150 ms for 3-5 dB ->
        // 0.6 Hz wow -> brickwall limiter at -0.3 dBFS shaving 3-6 dB.
        { "Memphis Master Bus", {
            { pid::crushOn, 0 }, { pid::monoOn, 1 }, { pid::monoFreq, 120 }, { pid::lowCut, 32 },
            { pid::tapeDrive, 3 }, { pid::tapeBias, 0.25f }, { pid::highCut, 12000 }, { pid::boxFreq, 300 },
            { pid::boxGain, 1.75f }, { pid::boxQ, 0.7f }, { pid::presence, -1.5f }, { pid::wow, 0.3f },
            { pid::flutter, 0.08f }, { pid::dropouts, 0.0f }, { pid::hiss, -72 }, { pid::dbxOn, 0 },
            { pid::subOn, 0 }, { pid::compMode, 1 }, { pid::compRatio, 0 }, { pid::compThresh, -16 },
            { pid::compAttack, 10 }, { pid::compRelease, 150 }, { pid::compMakeup, 3 }, { pid::outClip, 0 },
            { pid::limitOn, 1 }, { pid::limitDrive, 5 }, { pid::limitCeiling, -0.3f } } },
        { "Clean (Init)", {
            { pid::crushOn, 0 }, { pid::eqOn, 0 }, { pid::tapeOn, 0 }, { pid::dbxOn, 0 }, { pid::compOn, 0 },
            { pid::subOn, 0 }, { pid::outClip, 0 }, { pid::outputGain, 0 } } },
    };
    return presets;
}
} // namespace md
