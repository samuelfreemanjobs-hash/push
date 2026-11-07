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

    // 01 Sampler
    layout.add (bp (pid::crushOn, "Sampler On", true));
    layout.add (fp (pid::crushRate, "Sample Rate", skewed (2000.0f, 48000.0f, 16000.0f, 1.0f), 22050.0f, hzText));
    layout.add (fp (pid::crushBits, "Bit Depth", { 4.0f, 16.0f, 0.1f }, 12.0f,
                    [] (float v, int) { return juce::String (v, 1) + " bit"; }));
    layout.add (bp (pid::crushAA, "Anti-Alias", false));

    // 02 Cassette EQ
    layout.add (bp (pid::eqOn, "EQ On", true));
    layout.add (fp (pid::lowCut,  "Low Cut",  skewed (20.0f, 300.0f, 60.0f, 1.0f), 30.0f, hzText));
    layout.add (fp (pid::boxFreq, "Box Freq", skewed (150.0f, 600.0f, 300.0f, 1.0f), 300.0f, hzText));
    layout.add (fp (pid::boxGain, "Box Gain", { -6.0f, 12.0f, 0.1f }, 4.0f, dbText));
    layout.add (fp (pid::boxQ,    "Box Q",    skewed (0.5f, 6.0f, 1.8f, 0.01f), 2.0f,
                    [] (float v, int) { return juce::String (v, 2); }));
    layout.add (fp (pid::highCut, "High Cut", skewed (3000.0f, 20000.0f, 10000.0f, 1.0f), 11000.0f, hzText));

    // 03 Tape
    layout.add (bp (pid::tapeOn, "Tape On", true));
    layout.add (fp (pid::tapeDrive, "Tape Drive", { 0.0f, 24.0f, 0.1f }, 9.0f, dbText));
    layout.add (fp (pid::tapeBias,  "Tape Bias",  { 0.0f, 1.0f, 0.01f }, 0.3f, pctText));
    layout.add (fp (pid::wow,       "Wow",        { 0.0f, 1.0f, 0.01f }, 0.15f, pctText));
    layout.add (fp (pid::flutter,   "Flutter",    { 0.0f, 1.0f, 0.01f }, 0.2f, pctText));
    layout.add (fp (pid::hiss,      "Hiss",       { -90.0f, -30.0f, 0.1f }, -64.0f,
                    [] (float v, int) { return v <= -89.9f ? juce::String ("Off") : juce::String (v, 1) + " dB"; }));

    // 04 FET comp
    layout.add (bp (pid::compOn, "Comp On", true));
    layout.add (fp (pid::compThresh, "Threshold", { -40.0f, 0.0f, 0.1f }, -18.0f, dbText));
    layout.add (std::make_unique<APC> (juce::ParameterID { pid::compRatio, 1 }, "Ratio",
                                       juce::StringArray { "4:1", "8:1", "12:1", "20:1", "ALL" }, 1));
    layout.add (fp (pid::compAttack,  "Attack",  skewed (0.02f, 30.0f, 1.0f, 0.01f), 0.1f, msText));
    layout.add (fp (pid::compRelease, "Release", skewed (20.0f, 1100.0f, 150.0f, 1.0f), 60.0f, msText));
    layout.add (fp (pid::compMakeup,  "Makeup",  { 0.0f, 24.0f, 0.1f }, 9.0f, dbText));
    layout.add (fp (pid::compMix,     "Comp Mix", { 0.0f, 1.0f, 0.01f }, 1.0f, pctText));

    // 05 808 clip
    layout.add (bp (pid::subOn, "808 Clip On", true));
    layout.add (fp (pid::subFreq,  "Sub Freq",  skewed (60.0f, 250.0f, 120.0f, 1.0f), 120.0f, hzText));
    layout.add (fp (pid::subDrive, "Sub Drive", { 0.0f, 30.0f, 0.1f }, 12.0f, dbText));
    layout.add (fp (pid::subHard,  "Hardness",  { 0.0f, 1.0f, 0.01f }, 0.6f, pctText));
    layout.add (fp (pid::subLevel, "Sub Level", { -24.0f, 12.0f, 0.1f }, 0.0f, dbText));

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
            { pid::tapeDrive, 6 }, { pid::wow, 0.05f }, { pid::flutter, 0.1f }, { pid::hiss, -80 },
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
        { "Clean (Init)", {
            { pid::crushOn, 0 }, { pid::eqOn, 0 }, { pid::tapeOn, 0 }, { pid::compOn, 0 }, { pid::subOn, 0 },
            { pid::outClip, 0 }, { pid::outputGain, 0 } } },
    };
    return presets;
}
} // namespace md
