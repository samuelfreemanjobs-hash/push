#include "Params.h"

namespace kf
{
const juce::StringArray& voiceTypeNames()
{
    static const juce::StringArray names { "Kick", "Snare", "Clap", "Closed Hat", "Open Hat",
                                           "Tom", "Rim", "Cymbal", "Cowbell", "Perc", "808 Sub", "Shaker" };
    return names;
}

const juce::StringArray& waveNames()
{
    static const juce::StringArray names { "Sine", "Triangle", "Saw", "Square" };
    return names;
}

const juce::StringArray& noiseTypeNames()
{
    static const juce::StringArray names { "White", "Metallic" };
    return names;
}

const juce::StringArray& filterModeNames()
{
    static const juce::StringArray names { "Low-pass", "Band-pass", "High-pass" };
    return names;
}

const VoiceParamSpec& voiceSpec (int vp)
{
    static const VoiceParamSpec specs[NumVP] = {
        //  id            name           kind          min     max      def     centre  interval unit   choices
        { "type",       "Type",        Kind::Choice, 0,      11,       0,      0,      1,       "",    &voiceTypeNames() },
        { "wave",       "Wave",        Kind::Choice, 0,      3,       0,      0,      1,       "",    &waveNames() },
        { "pitch",      "Pitch",       Kind::Float,  20,     5000,    55,     200,    0,       "Hz",  nullptr },
        { "penv",       "Pitch Env",   Kind::Float,  0,      60,      24,     0,      0,       "st",  nullptr },
        { "pdecay",     "Pitch Decay", Kind::Float,  1,      1000,    40,     60,     0,       "ms",  nullptr },
        { "tdecay",     "Tone Decay",  Kind::Float,  5,      10000,   400,    300,    0,       "ms",  nullptr },
        { "hold",       "Hold",        Kind::Float,  0,      4000,    0,      300,    0,       "ms",  nullptr },
        { "glide",      "Glide",       Kind::Float,  0,      1000,    0,      120,    0,       "ms",  nullptr },
        { "tlevel",     "Tone Level",  Kind::Float,  0,      1,       1,      0,      0,       "",    nullptr },
        { "fm",         "FM Amount",   Kind::Float,  0,      10,      0,      0,      0,       "",    nullptr },
        { "ratio",      "FM Ratio",    Kind::Float,  0.25f,  12,      1.5f,   2,      0,       "",    nullptr },
        { "fmdecay",    "FM Decay",    Kind::Float,  5,      4000,    4000,   300,    0,       "ms",  nullptr },
        { "ntype",      "Noise Type",  Kind::Choice, 0,      1,       0,      0,      1,       "",    &noiseTypeNames() },
        { "nlevel",     "Noise Level", Kind::Float,  0,      1,       0,      0,      0,       "",    nullptr },
        { "ndecay",     "Noise Decay", Kind::Float,  5,      6000,    150,    300,    0,       "ms",  nullptr },
        { "fmode",      "Filter Mode", Kind::Choice, 0,      2,       0,      0,      1,       "",    &filterModeNames() },
        { "cutoff",     "Cutoff",      Kind::Float,  20,     20000,   8000,   1000,   0,       "Hz",  nullptr },
        { "reso",       "Resonance",   Kind::Float,  0.3f,   12,      0.7f,   2,      0,       "Q",   nullptr },
        { "burst",      "Bursts",      Kind::Float,  1,      6,       1,      0,      1,       "",    nullptr },
        { "click",      "Click",       Kind::Float,  0,      1,       0.2f,   0,      0,       "",    nullptr },
        { "attack",     "Attack",      Kind::Float,  0,      50,      0,      5,      0,       "ms",  nullptr },
        { "punch",      "Punch",       Kind::Float,  -1,     1,       0,      0,      0,       "",    nullptr },
        { "ptime",      "Punch Time",  Kind::Float,  2,      150,     15,     25,     0,       "ms",  nullptr },
        { "drive",      "Drive",       Kind::Float,  0,      1,       0,      0,      0,       "",    nullptr },
        { "level",      "Level",       Kind::Float,  0,      1,       0.8f,   0,      0,       "",    nullptr },
        { "pan",        "Pan",         Kind::Float,  -1,     1,       0,      0,      0,       "",    nullptr },
        { "choke",      "Choke Group", Kind::Float,  0,      4,       0,      0,      1,       "",    nullptr },
        { "lofi",       "Lo-Fi Send",  Kind::Float,  0,      1,       1,      0,      0,       "",    nullptr },
        { "mute",       "Mute",        Kind::Bool,   0,      1,       0,      0,      1,       "",    nullptr },
        { "solo",       "Solo",        Kind::Bool,   0,      1,       0,      0,      1,       "",    nullptr },
    };
    jassert (vp >= 0 && vp < NumVP);
    return specs[vp];
}

const VoiceParamSpec& busSpec (int bp)
{
    static const VoiceParamSpec specs[NumBP] = {
        { "lf_drive",   "Bus Drive",   Kind::Float,  0,      1,       0,      0,      0,       "",    nullptr },
        { "lf_bits",    "Bit Depth",   Kind::Float,  4,      16,      16,     0,      0,       "bit", nullptr },
        { "lf_rate",    "Sample Rate", Kind::Float,  4000,   48000,   48000,  20000,  0,       "Hz",  nullptr },
        { "lf_pitch",   "Pitch Hack",  Kind::Float,  0,      12,      0,      0,      0,       "st",  nullptr },
        { "lf_lpf",     "Bus Low-pass",Kind::Float,  1000,   20000,   20000,  8000,   0,       "Hz",  nullptr },
        { "lf_lowkeep", "Low Keep",    Kind::Float,  0,      250,     0,      0,      0,       "Hz",  nullptr },
        { "lf_punch",   "Punch",       Kind::Float,  -1,     1,       0,      0,      0,       "",    nullptr },
        { "lf_tail",    "Tail",        Kind::Float,  -1,     1,       0,      0,      0,       "",    nullptr },
        { "lf_vinyl",   "Vinyl",       Kind::Float,  0,      1,       0,      0,      0,       "",    nullptr },
    };
    jassert (bp >= 0 && bp < NumBP);
    return specs[bp];
}

juce::String busParamID (int bp)
{
    return busSpec (bp).id;
}

const juce::StringArray& machineNames()
{
    static const juce::StringArray names { "Clean (bypass)", "SP-1200 style  26k/12-bit", "MPC60 style  40k/12-bit",
                                           "DR-660 phonk  32k/12-bit", "SP-808 / Lofi-12 style  22k" };
    return names;
}

std::array<float, NumBP> machinePreset (int index)
{
    //                       drive   bits   rate      pitch  lpf      lowkeep punch tail vinyl
    switch (index)
    {
        case 1:  return { 0.25f, 12.0f, 26040.0f, 5.0f, 11000.0f, 90.0f,  0, 0, 0.08f };
        case 2:  return { 0.35f, 12.0f, 40000.0f, 0.0f, 15000.0f, 0.0f,   0, 0, 0.03f };
        case 3:  return { 0.60f, 12.0f, 32000.0f, 0.0f, 12000.0f, 0.0f,   0, 0, 0.05f };
        case 4:  return { 0.50f, 12.0f, 22050.0f, 3.0f, 10000.0f, 80.0f,  0, 0, 0.25f };
        default: return { 0.0f,  16.0f, 48000.0f, 0.0f, 20000.0f, 0.0f,   0, 0, 0.0f };
    }
}

juce::String voiceParamID (int track, int vp)
{
    return "t" + juce::String (track + 1) + "_" + voiceSpec (vp).id;
}

static juce::String formatValue (float v, const char* unit)
{
    const juce::String u (unit);
    if (u == "bit")
        return juce::String (v, 1) + " bit";
    if (u == "Hz")
        return v >= 1000.0f ? juce::String (v / 1000.0f, 2) + " kHz" : juce::String (v, 1) + " Hz";
    if (u == "ms")
        return v >= 1000.0f ? juce::String (v / 1000.0f, 2) + " s" : juce::String (v, 1) + " ms";
    if (u.isNotEmpty())
        return juce::String (v, 1) + " " + u;
    return juce::String (v, 2);
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // ---- Global ----
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ids::tempo, 1 }, "Tempo",
        juce::NormalisableRange<float> (40.0f, 240.0f, 0.1f), 120.0f,
        juce::AudioParameterFloatAttributes().withLabel ("BPM")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ids::swing, 1 }, "Swing",
        juce::NormalisableRange<float> (50.0f, 75.0f, 0.1f), 50.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ids::length, 1 }, "Pattern Length", 1, numSteps, numSteps));

    juce::StringArray patternNames;
    for (int i = 0; i < numPatterns; ++i)
        patternNames.add (juce::String::charToString ((juce::juce_wchar) ('A' + i)));
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ids::pattern, 1 }, "Pattern", patternNames, 0));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ids::master, 1 }, "Master",
        juce::NormalisableRange<float> (-60.0f, 6.0f, 0.1f), -3.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    // ---- Vintage sampler bus ----
    {
        auto group = std::make_unique<juce::AudioProcessorParameterGroup> ("bus", "Vintage Bus", " | ");
        for (int bp = 0; bp < NumBP; ++bp)
        {
            const auto& s = busSpec (bp);
            juce::NormalisableRange<float> range (s.min, s.max, s.interval);
            if (s.centre > 0.0f)
                range.setSkewForCentre (s.centre);

            const char* unit = s.unit;
            const juce::String id (s.id);
            auto attrs = juce::AudioParameterFloatAttributes()
                             .withLabel (unit)
                             .withStringFromValueFunction ([unit, id] (float v, int)
                             {
                                 if (id == "lf_lowkeep" && v < 20.0f) return juce::String ("Off");
                                 if (id == "lf_lpf" && v >= 19999.0f) return juce::String ("Off");
                                 if (id == "lf_pitch" && v < 0.05f) return juce::String ("Off");
                                 return formatValue (v, unit);
                             });
            group->addChild (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { s.id, 1 }, s.name, range, s.def, attrs));
        }
        layout.add (std::move (group));
    }

    // ---- Voices ----
    for (int t = 0; t < numTracks; ++t)
    {
        auto group = std::make_unique<juce::AudioProcessorParameterGroup> (
            "track" + juce::String (t + 1), "Track " + juce::String (t + 1), " | ");

        for (int vp = 0; vp < NumVP; ++vp)
        {
            const auto& s = voiceSpec (vp);
            const juce::ParameterID pid { voiceParamID (t, vp), 1 };
            const juce::String name = "T" + juce::String (t + 1) + " " + s.name;

            switch (s.kind)
            {
                case Kind::Choice:
                    group->addChild (std::make_unique<juce::AudioParameterChoice> (pid, name, *s.choices, (int) s.def));
                    break;

                case Kind::Bool:
                    group->addChild (std::make_unique<juce::AudioParameterBool> (pid, name, s.def > 0.5f));
                    break;

                case Kind::Float:
                {
                    juce::NormalisableRange<float> range (s.min, s.max, s.interval);
                    if (s.centre > 0.0f)
                        range.setSkewForCentre (s.centre);

                    const char* unit = s.unit;
                    const bool isInt = s.interval >= 1.0f;
                    auto attrs = juce::AudioParameterFloatAttributes()
                                     .withLabel (unit)
                                     .withStringFromValueFunction ([unit, isInt] (float v, int)
                                     {
                                         return isInt ? juce::String ((int) std::round (v)) : formatValue (v, unit);
                                     });
                    group->addChild (std::make_unique<juce::AudioParameterFloat> (pid, name, range, s.def, attrs));
                    break;
                }
            }
        }
        layout.add (std::move (group));
    }

    return layout;
}

void setParamValue (juce::AudioProcessorValueTreeState& apvts, const juce::String& id, float value)
{
    if (auto* p = apvts.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (value));
        p->endChangeGesture();
    }
}

float getParamValue (juce::AudioProcessorValueTreeState& apvts, const juce::String& id)
{
    if (auto* v = apvts.getRawParameterValue (id))
        return v->load();
    return 0.0f;
}

juce::Colour trackColour (int track)
{
    static const juce::uint32 colours[numTracks] = {
        // bank 1: pads A01-A08
        0xffff5a4e, 0xffffa53c, 0xfff5d547, 0xff7ddc5b, 0xff3fd6c0, 0xff4aa8ff, 0xff9b7bff, 0xffff6fc8,
        // bank 2: pads A09-A16
        0xffe8746a, 0xffd99a5a, 0xffc9bd5e, 0xff8fc57a, 0xff5fc0b2, 0xff6f9fd8, 0xffa592e0, 0xffe08bbf,
    };
    return juce::Colour (colours[juce::jlimit (0, numTracks - 1, track)]);
}
} // namespace kf
