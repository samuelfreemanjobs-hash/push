#include "KitFactory.h"

namespace kf::KitFactory
{
namespace
{
    using Overrides = std::initializer_list<std::pair<VP, float>>;

    VoiceDef make (VoiceType type, Overrides overrides)
    {
        auto v = defaultVoice();
        v[Type] = (float) type;
        for (auto& [vp, value] : overrides)
            v[vp] = value;
        return v;
    }

    // Shorthands for enum values used in the kit tables.
    constexpr float sine = 0, tri = 1, saw = 2, square = 3;
    constexpr float white = 0, metal = 1;
    constexpr float LP = 0, BP = 1, HP = 2;

    float uni (juce::Random& r, float lo, float hi)  { return lo + r.nextFloat() * (hi - lo); }
    float logu (juce::Random& r, float lo, float hi) { return lo * std::pow (hi / lo, r.nextFloat()); }
    bool chance (juce::Random& r, float p)           { return r.nextFloat() < p; }
}

VoiceDef defaultVoice()
{
    VoiceDef v {};
    for (int i = 0; i < NumVP; ++i)
        v[(size_t) i] = voiceSpec (i).def;
    return v;
}

//==============================================================================
// Factory kits
//==============================================================================
const juce::StringArray& factoryKitNames()
{
    static const juce::StringArray names { "Boom Analog", "Punch Machine", "Electro Steel",
                                           "Dusty Lo-Fi", "Industrial Forge", "Minimal Tick",
                                           "Jazz Phonk (16 pads)" };
    return names;
}

static void clampVoice (VoiceDef& v)
{
    for (int i = 0; i < NumVP; ++i)
    {
        const auto& sp = voiceSpec (i);
        v[(size_t) i] = juce::jlimit (sp.min, sp.max, v[(size_t) i]);
    }
}

static void deriveBankTwo (Kit& k)
{
    using VT = VoiceType;

    // Find a bank-1 voice of one of the given types to base a variation on.
    auto find = [&k] (std::initializer_list<VT> types, int fallback)
    {
        for (int t = 0; t < 8; ++t)
            for (auto type : types)
                if ((VT) (int) k.voices[(size_t) t][Type] == type)
                    return k.voices[(size_t) t];
        return k.voices[(size_t) fallback];
    };

    auto set = [&k] (int pad, VoiceDef v, VT type, const juce::String& name)
    {
        v[Type] = (float) type;
        clampVoice (v);
        k.voices[(size_t) pad] = v;
        k.voiceNames[(size_t) pad] = name;
    };

    const auto tom = find ({ VT::Tom, VT::Perc }, 5);
    auto lowTom = tom;
    lowTom[Pitch] *= 0.7f; lowTom[ToneDecay] *= 1.3f; lowTom[Pan] = -0.3f;
    set (8, lowTom, VT::Tom, "Low Tom");

    auto highTom = tom;
    highTom[Pitch] *= 1.4f; highTom[ToneDecay] *= 0.8f; highTom[Pan] = 0.3f;
    set (9, highTom, VT::Tom, "High Tom");

    auto snare = find ({ VT::Snare, VT::Clap }, 1);
    snare[Pitch] *= 1.25f; snare[ToneDecay] *= 0.7f; snare[NoiseDecay] *= 0.6f; snare[Level] *= 0.85f;
    set (10, snare, VT::Snare, "Snare Alt");

    // A shorter, brighter hat for rolls. Same choke group as the main hats.
    auto hat = find ({ VT::ClosedHat }, 3);
    hat[NoiseDecay] *= 0.6f; hat[Pitch] *= 1.12f; hat[Cutoff] *= 1.1f; hat[Level] *= 0.9f;
    set (11, hat, VT::ClosedHat, "Roll Hat");

    set (12, make (VT::Sub808, { { Wave, sine }, { Pitch, 43.65f }, { PEnvAmt, 7 }, { PEnvDecay, 150 }, { ToneDecay, 1600 },
                                 { Click, 0.05f }, { Drive, 0.2f }, { Level, 0.9f }, { LoFiSend, 0 } }),
         VT::Sub808, "808 Sub");

    auto perc = find ({ VT::Cowbell, VT::Rim, VT::Perc }, 6);
    perc[Pitch] *= 1.5f; perc[Pan] = -perc[Pan];
    set (13, perc, (VT) (int) perc[Type], "Perc Alt");

    auto cym = find ({ VT::Cymbal, VT::OpenHat }, 7);
    cym[NoiseDecay] *= 0.5f; cym[Choke] = 0;
    set (14, cym, VT::Cymbal, "Crash Short");

    set (15, make (VT::Shaker, { { ToneLevel, 0 }, { NoiseType, white }, { NoiseLevel, 1 }, { NoiseDecay, 80 },
                                 { FilterMode, HP }, { Cutoff, 7000 }, { Reso, 0.7f }, { Burst, 2 }, { Click, 0 },
                                 { Attack, 10 }, { Level, 0.45f }, { Pan, 0.25f } }),
         VT::Shaker, "Shaker");
}

Kit factoryKit (int index)
{
    using VT = VoiceType;
    Kit k;
    k.name = factoryKitNames()[juce::jlimit (0, factoryKitNames().size() - 1, index)];
    k.hasBus = true;
    k.bus = defaultBus();

    // Each kit designs bank 1 (pads A01-A08). Bank 2 (A09-A16) is either
    // designed too, or derived from bank 1 by deriveBankTwo().
    using Bank = std::array<VoiceDef, 8>;
    using Names = std::array<juce::String, 8>;
    Bank bankA, bankB;
    Names namesA, namesB;
    bool haveBankB = false;

    switch (index)
    {
        default:
        case 0: // Boom Analog -- deep, round, long-tailed
            bankA = {
                make (VT::Kick,      { { Wave, sine }, { Pitch, 46 }, { PEnvAmt, 30 }, { PEnvDecay, 25 }, { ToneDecay, 1100 },
                                       { Click, 0.15f }, { Drive, 0.15f }, { Level, 0.95f } }),
                make (VT::Snare,     { { Wave, tri }, { Pitch, 185 }, { PEnvAmt, 8 }, { PEnvDecay, 15 }, { ToneDecay, 110 }, { ToneLevel, 0.6f },
                                       { NoiseLevel, 0.8f }, { NoiseDecay, 200 }, { FilterMode, HP }, { Cutoff, 1800 }, { Click, 0.2f } }),
                make (VT::Clap,      { { ToneLevel, 0 }, { NoiseLevel, 1 }, { NoiseDecay, 260 }, { FilterMode, BP }, { Cutoff, 1100 },
                                       { Reso, 1.6f }, { Burst, 4 }, { Click, 0 }, { Level, 0.8f }, { Pan, -0.1f } }),
                make (VT::ClosedHat, { { ToneLevel, 0 }, { NoiseType, metal }, { Pitch, 400 }, { NoiseLevel, 1 }, { NoiseDecay, 45 },
                                       { FilterMode, HP }, { Cutoff, 7500 }, { Reso, 1 }, { Click, 0 }, { Level, 0.6f }, { Pan, 0.2f }, { Choke, 1 } }),
                make (VT::OpenHat,   { { ToneLevel, 0 }, { NoiseType, metal }, { Pitch, 400 }, { NoiseLevel, 1 }, { NoiseDecay, 420 },
                                       { FilterMode, HP }, { Cutoff, 7500 }, { Reso, 1 }, { Click, 0 }, { Level, 0.55f }, { Pan, 0.2f }, { Choke, 1 } }),
                make (VT::Tom,       { { Wave, sine }, { Pitch, 110 }, { PEnvAmt, 7 }, { PEnvDecay, 60 }, { ToneDecay, 400 },
                                       { NoiseLevel, 0.05f }, { NoiseDecay, 40 }, { Click, 0.1f }, { Pan, -0.25f } }),
                make (VT::Cowbell,   { { Wave, square }, { Pitch, 560 }, { PEnvAmt, 0 }, { ToneDecay, 250 }, { ToneLevel, 0.6f },
                                       { FmAmt, 0.8f }, { FmRatio, 1.48f }, { Click, 0 }, { Level, 0.45f }, { Pan, 0.3f } }),
                make (VT::Cymbal,    { { ToneLevel, 0 }, { NoiseType, metal }, { Pitch, 450 }, { NoiseLevel, 1 }, { NoiseDecay, 1800 },
                                       { FilterMode, HP }, { Cutoff, 5500 }, { Reso, 0.8f }, { Click, 0 }, { Level, 0.5f }, { Pan, -0.2f } }),
            };
            break;

        case 1: // Punch Machine -- tight, punchy, bright
            bankA = {
                make (VT::Kick,      { { Wave, sine }, { Pitch, 52 }, { PEnvAmt, 36 }, { PEnvDecay, 35 }, { ToneDecay, 450 },
                                       { NoiseLevel, 0.08f }, { NoiseDecay, 20 }, { FilterMode, LP }, { Cutoff, 5000 },
                                       { Click, 0.5f }, { Drive, 0.35f }, { Level, 0.95f } }),
                make (VT::Snare,     { { Wave, tri }, { Pitch, 210 }, { PEnvAmt, 12 }, { PEnvDecay, 20 }, { ToneDecay, 90 }, { ToneLevel, 0.7f },
                                       { NoiseLevel, 0.9f }, { NoiseDecay, 230 }, { FilterMode, BP }, { Cutoff, 4500 }, { Reso, 0.8f },
                                       { Click, 0.35f }, { Drive, 0.2f } }),
                make (VT::Clap,      { { ToneLevel, 0 }, { NoiseLevel, 1 }, { NoiseDecay, 320 }, { FilterMode, BP }, { Cutoff, 1400 },
                                       { Reso, 1.2f }, { Burst, 4 }, { Click, 0 }, { Level, 0.8f } }),
                make (VT::ClosedHat, { { ToneLevel, 0 }, { NoiseType, white }, { NoiseLevel, 1 }, { NoiseDecay, 55 },
                                       { FilterMode, HP }, { Cutoff, 9000 }, { Reso, 0.7f }, { Click, 0 }, { Level, 0.55f }, { Pan, 0.15f }, { Choke, 1 } }),
                make (VT::OpenHat,   { { ToneLevel, 0 }, { NoiseType, white }, { NoiseLevel, 1 }, { NoiseDecay, 500 },
                                       { FilterMode, HP }, { Cutoff, 8500 }, { Reso, 0.7f }, { Click, 0 }, { Level, 0.5f }, { Pan, 0.15f }, { Choke, 1 } }),
                make (VT::Tom,       { { Wave, sine }, { Pitch, 140 }, { PEnvAmt, 10 }, { PEnvDecay, 50 }, { ToneDecay, 300 },
                                       { Click, 0.25f }, { Pan, 0.25f } }),
                make (VT::Rim,       { { Wave, tri }, { Pitch, 1700 }, { PEnvAmt, 3 }, { PEnvDecay, 5 }, { ToneDecay, 25 },
                                       { NoiseLevel, 0.3f }, { NoiseDecay, 15 }, { FilterMode, HP }, { Cutoff, 3000 },
                                       { Click, 0.6f }, { Level, 0.6f }, { Pan, -0.2f } }),
                make (VT::Cymbal,    { { ToneLevel, 0 }, { NoiseType, metal }, { Pitch, 520 }, { NoiseLevel, 1 }, { NoiseDecay, 2200 },
                                       { FilterMode, HP }, { Cutoff, 7000 }, { Reso, 0.7f }, { Click, 0.1f }, { Level, 0.45f }, { Pan, -0.15f } }),
            };
            break;

        case 2: // Electro Steel -- FM, metallic, driven
            bankA = {
                make (VT::Kick,      { { Wave, sine }, { Pitch, 60 }, { PEnvAmt, 40 }, { PEnvDecay, 18 }, { ToneDecay, 300 },
                                       { FmAmt, 1.5f }, { FmRatio, 0.5f }, { Click, 0.3f }, { Drive, 0.6f }, { Level, 0.9f } }),
                make (VT::Snare,     { { Wave, saw }, { Pitch, 240 }, { PEnvAmt, 6 }, { PEnvDecay, 10 }, { ToneDecay, 70 }, { ToneLevel, 0.6f },
                                       { FmAmt, 3 }, { FmRatio, 2.3f }, { NoiseLevel, 0.7f }, { NoiseDecay, 140 },
                                       { FilterMode, HP }, { Cutoff, 2500 }, { Click, 0.3f }, { Drive, 0.3f } }),
                make (VT::Clap,      { { ToneLevel, 0 }, { NoiseLevel, 1 }, { NoiseDecay, 180 }, { FilterMode, BP }, { Cutoff, 1800 },
                                       { Reso, 2.5f }, { Burst, 5 }, { Click, 0 }, { Level, 0.75f }, { Pan, 0.1f } }),
                make (VT::ClosedHat, { { ToneLevel, 0 }, { NoiseType, metal }, { Pitch, 560 }, { NoiseLevel, 1 }, { NoiseDecay, 30 },
                                       { FilterMode, HP }, { Cutoff, 9500 }, { Reso, 2 }, { Click, 0 }, { Level, 0.6f }, { Pan, -0.2f }, { Choke, 1 } }),
                make (VT::OpenHat,   { { ToneLevel, 0 }, { NoiseType, metal }, { Pitch, 560 }, { NoiseLevel, 1 }, { NoiseDecay, 300 },
                                       { FilterMode, HP }, { Cutoff, 9000 }, { Reso, 2 }, { Click, 0 }, { Level, 0.5f }, { Pan, -0.2f }, { Choke, 1 } }),
                make (VT::Perc,      { { Wave, sine }, { Pitch, 180 }, { PEnvAmt, 12 }, { PEnvDecay, 40 }, { ToneDecay, 220 },
                                       { FmAmt, 4 }, { FmRatio, 3.5f }, { Click, 0.1f }, { Level, 0.65f }, { Pan, 0.3f } }),
                make (VT::Cowbell,   { { Wave, square }, { Pitch, 800 }, { PEnvAmt, 0 }, { ToneDecay, 120 }, { ToneLevel, 0.6f },
                                       { FmAmt, 2 }, { FmRatio, 1.33f }, { Click, 0 }, { Level, 0.4f }, { Pan, 0.35f } }),
                make (VT::Cymbal,    { { ToneLevel, 0 }, { NoiseType, metal }, { Pitch, 620 }, { NoiseLevel, 1 }, { NoiseDecay, 1200 },
                                       { FilterMode, BP }, { Cutoff, 8000 }, { Reso, 1.5f }, { Click, 0 }, { Level, 0.45f } }),
            };
            break;

        case 3: // Dusty Lo-Fi -- dark, soft, saturated
            bankA = {
                make (VT::Kick,      { { Wave, tri }, { Pitch, 50 }, { PEnvAmt, 18 }, { PEnvDecay, 40 }, { ToneDecay, 350 },
                                       { NoiseLevel, 0.1f }, { NoiseDecay, 30 }, { FilterMode, LP }, { Cutoff, 1200 },
                                       { Click, 0.1f }, { Attack, 1 }, { Drive, 0.5f }, { Level, 0.9f } }),
                make (VT::Snare,     { { Wave, sine }, { Pitch, 170 }, { PEnvAmt, 5 }, { PEnvDecay, 20 }, { ToneDecay, 80 }, { ToneLevel, 0.7f },
                                       { NoiseLevel, 0.6f }, { NoiseDecay, 160 }, { FilterMode, LP }, { Cutoff, 3500 }, { Reso, 0.9f },
                                       { Click, 0.1f }, { Drive, 0.4f } }),
                make (VT::Clap,      { { ToneLevel, 0 }, { NoiseLevel, 1 }, { NoiseDecay, 200 }, { FilterMode, BP }, { Cutoff, 900 },
                                       { Reso, 1.4f }, { Burst, 3 }, { Click, 0 }, { Drive, 0.3f }, { Level, 0.75f } }),
                make (VT::ClosedHat, { { ToneLevel, 0 }, { NoiseType, white }, { NoiseLevel, 1 }, { NoiseDecay, 35 },
                                       { FilterMode, BP }, { Cutoff, 6000 }, { Reso, 0.8f }, { Click, 0 }, { Level, 0.5f }, { Pan, 0.2f }, { Choke, 1 } }),
                make (VT::OpenHat,   { { ToneLevel, 0 }, { NoiseType, white }, { NoiseLevel, 1 }, { NoiseDecay, 260 },
                                       { FilterMode, BP }, { Cutoff, 5500 }, { Reso, 0.8f }, { Click, 0 }, { Level, 0.45f }, { Pan, 0.2f }, { Choke, 1 } }),
                make (VT::Tom,       { { Wave, sine }, { Pitch, 90 }, { PEnvAmt, 6 }, { PEnvDecay, 70 }, { ToneDecay, 500 },
                                       { Click, 0.05f }, { Drive, 0.3f }, { Pan, -0.3f } }),
                make (VT::Rim,       { { Wave, tri }, { Pitch, 900 }, { PEnvAmt, 2 }, { PEnvDecay, 5 }, { ToneDecay, 30 },
                                       { Click, 0.3f }, { Drive, 0.5f }, { Level, 0.55f }, { Pan, 0.3f } }),
                make (VT::Cymbal,    { { ToneLevel, 0 }, { NoiseType, white }, { NoiseLevel, 1 }, { NoiseDecay, 900 },
                                       { FilterMode, BP }, { Cutoff, 7000 }, { Reso, 0.9f }, { Click, 0 }, { Drive, 0.2f }, { Level, 0.4f } }),
            };
            break;

        case 4: // Industrial Forge -- huge, distorted, resonant
            bankA = {
                make (VT::Kick,      { { Wave, sine }, { Pitch, 42 }, { PEnvAmt, 48 }, { PEnvDecay, 60 }, { ToneDecay, 700 },
                                       { NoiseLevel, 0.2f }, { NoiseDecay, 80 }, { FilterMode, LP }, { Cutoff, 1500 },
                                       { Click, 0.6f }, { Drive, 0.9f }, { Level, 0.85f } }),
                make (VT::Snare,     { { Wave, square }, { Pitch, 160 }, { PEnvAmt, 10 }, { PEnvDecay, 25 }, { ToneDecay, 160 }, { ToneLevel, 0.5f },
                                       { FmAmt, 5 }, { FmRatio, 1.41f }, { NoiseLevel, 1 }, { NoiseDecay, 300 },
                                       { FilterMode, BP }, { Cutoff, 2000 }, { Reso, 3 }, { Click, 0.3f }, { Drive, 0.7f }, { Level, 0.7f } }),
                make (VT::Clap,      { { ToneLevel, 0 }, { NoiseLevel, 1 }, { NoiseDecay, 450 }, { FilterMode, BP }, { Cutoff, 700 },
                                       { Reso, 4 }, { Burst, 6 }, { Click, 0 }, { Drive, 0.5f }, { Level, 0.7f } }),
                make (VT::ClosedHat, { { ToneLevel, 0 }, { NoiseType, metal }, { Pitch, 300 }, { NoiseLevel, 1 }, { NoiseDecay, 70 },
                                       { FilterMode, BP }, { Cutoff, 6000 }, { Reso, 4 }, { Click, 0 }, { Drive, 0.4f }, { Level, 0.5f }, { Choke, 1 } }),
                make (VT::OpenHat,   { { ToneLevel, 0 }, { NoiseType, metal }, { Pitch, 300 }, { NoiseLevel, 1 }, { NoiseDecay, 600 },
                                       { FilterMode, BP }, { Cutoff, 6000 }, { Reso, 4 }, { Click, 0 }, { Drive, 0.4f }, { Level, 0.45f }, { Choke, 1 } }),
                make (VT::Perc,      { { Wave, saw }, { Pitch, 70 }, { PEnvAmt, 24 }, { PEnvDecay, 120 }, { ToneDecay, 600 },
                                       { FmAmt, 2 }, { FmRatio, 0.75f }, { Click, 0.2f }, { Drive, 0.6f }, { Level, 0.6f }, { Pan, -0.3f } }),
                make (VT::Perc,      { { Wave, square }, { Pitch, 300 }, { PEnvAmt, 0 }, { ToneDecay, 400 }, { ToneLevel, 0.4f },
                                       { FmAmt, 8 }, { FmRatio, 3.7f }, { NoiseType, metal }, { NoiseLevel, 0.3f }, { NoiseDecay, 400 },
                                       { FilterMode, BP }, { Cutoff, 3000 }, { Reso, 6 }, { Click, 0.2f }, { Level, 0.45f }, { Pan, 0.3f } }),
                make (VT::Cymbal,    { { ToneLevel, 0 }, { NoiseType, metal }, { Pitch, 250 }, { NoiseLevel, 1 }, { NoiseDecay, 3000 },
                                       { FilterMode, HP }, { Cutoff, 3000 }, { Reso, 2 }, { Click, 0 }, { Drive, 0.3f }, { Level, 0.4f } }),
            };
            break;

        case 5: // Minimal Tick -- short, clean, clicky
            bankA = {
                make (VT::Kick,      { { Wave, sine }, { Pitch, 58 }, { PEnvAmt, 20 }, { PEnvDecay, 12 }, { ToneDecay, 180 },
                                       { Click, 0.3f }, { Level, 0.95f } }),
                make (VT::Snare,     { { Wave, tri }, { Pitch, 330 }, { PEnvAmt, 4 }, { PEnvDecay, 8 }, { ToneDecay, 40 }, { ToneLevel, 0.6f },
                                       { NoiseLevel, 0.4f }, { NoiseDecay, 70 }, { FilterMode, HP }, { Cutoff, 4000 }, { Click, 0.3f }, { Level, 0.7f } }),
                make (VT::Clap,      { { ToneLevel, 0 }, { NoiseLevel, 1 }, { NoiseDecay, 120 }, { FilterMode, BP }, { Cutoff, 2200 },
                                       { Reso, 2 }, { Burst, 3 }, { Click, 0 }, { Level, 0.7f } }),
                make (VT::ClosedHat, { { ToneLevel, 0 }, { NoiseType, white }, { NoiseLevel, 1 }, { NoiseDecay, 20 },
                                       { FilterMode, HP }, { Cutoff, 11000 }, { Reso, 0.7f }, { Click, 0 }, { Level, 0.55f }, { Pan, 0.25f }, { Choke, 1 } }),
                make (VT::OpenHat,   { { ToneLevel, 0 }, { NoiseType, white }, { NoiseLevel, 1 }, { NoiseDecay, 180 },
                                       { FilterMode, HP }, { Cutoff, 10000 }, { Reso, 0.7f }, { Click, 0 }, { Level, 0.45f }, { Pan, 0.25f }, { Choke, 1 } }),
                make (VT::Perc,      { { Wave, sine }, { Pitch, 260 }, { PEnvAmt, 5 }, { PEnvDecay, 15 }, { ToneDecay, 120 },
                                       { Click, 0.2f }, { Level, 0.6f }, { Pan, -0.3f } }),
                make (VT::Rim,       { { Wave, tri }, { Pitch, 1200 }, { PEnvAmt, 0 }, { ToneDecay, 15 }, { Click, 0.8f }, { Level, 0.55f }, { Pan, 0.1f } }),
                make (VT::Perc,      { { Wave, sine }, { Pitch, 2200 }, { PEnvAmt, 0 }, { ToneDecay, 60 }, { FmAmt, 2 }, { FmRatio, 3.1f },
                                       { Click, 0 }, { Level, 0.35f }, { Pan, -0.1f } }),
            };
            break;

        //----------------------------------------------------------------------
        // Jazz x Phonk x Trap: a 16-pad 12-bit lo-fi kit in two banks of 8.
        // Both banks run through an SP-1200 style bus with the lows kept clean.
        //----------------------------------------------------------------------
        case 6: // Jazz Phonk -- 16 pads, bank 1: A01..A08
            bankA = {
                // A01 acoustic jazz kick thud layered over an 808-style sub, mild crush
                make (VT::Kick,      { { Wave, sine }, { Pitch, 45 }, { PEnvAmt, 24 }, { PEnvDecay, 30 }, { ToneDecay, 900 },
                                       { NoiseLevel, 0.12f }, { NoiseDecay, 60 }, { FilterMode, LP }, { Cutoff, 2500 },
                                       { Click, 0.35f }, { Drive, 0.2f }, { Level, 0.95f }, { LoFiSend, 0.5f } }),
                // A02 hollow, mid-biting DR-style snare with a hard attack
                make (VT::Snare,     { { Wave, tri }, { Pitch, 200 }, { PEnvAmt, 10 }, { PEnvDecay, 15 }, { ToneDecay, 90 }, { ToneLevel, 0.55f },
                                       { NoiseLevel, 1 }, { NoiseDecay, 180 }, { FilterMode, BP }, { Cutoff, 2200 }, { Reso, 1.2f },
                                       { Click, 0.45f }, { Drive, 0.55f }, { Level, 0.85f } }),
                // A03 maple rimshot with a short FM "wood" strike and a roomy noise tail
                make (VT::Rim,       { { Wave, tri }, { Pitch, 1250 }, { PEnvAmt, 4 }, { PEnvDecay, 4 }, { ToneDecay, 40 }, { ToneLevel, 0.8f },
                                       { FmAmt, 1.2f }, { FmRatio, 2.3f }, { FmDecay, 30 },
                                       { NoiseLevel, 0.3f }, { NoiseDecay, 60 }, { FilterMode, BP }, { Cutoff, 3500 }, { Reso, 1.5f },
                                       { Click, 0.5f }, { Level, 0.6f }, { Pan, -0.15f } }),
                // A04 808-style metallic hat pitched up +4 st, tight
                make (VT::ClosedHat, { { ToneLevel, 0 }, { NoiseType, metal }, { Pitch, 504 }, { NoiseLevel, 1 }, { NoiseDecay, 35 },
                                       { FilterMode, HP }, { Cutoff, 8500 }, { Reso, 1.2f }, { Click, 0 }, { Level, 0.55f }, { Pan, 0.2f }, { Choke, 1 } }),
                // A05 brushed open hat: a slow "chiff" swell into a soft wash
                make (VT::OpenHat,   { { ToneLevel, 0 }, { NoiseType, white }, { NoiseLevel, 1 }, { NoiseDecay, 380 },
                                       { FilterMode, BP }, { Cutoff, 7000 }, { Reso, 0.7f }, { Click, 0 }, { Attack, 6 },
                                       { Drive, 0.2f }, { Level, 0.45f }, { Pan, 0.2f }, { Choke, 1 } }),
                // A06 Memphis cowbell on F#: square, +12 st snap over 15 ms, +7 cent detune, driven hard
                make (VT::Cowbell,   { { Wave, square }, { Pitch, 739.99f }, { PEnvAmt, 12 }, { PEnvDecay, 15 }, { ToneDecay, 220 },
                                       { ToneLevel, 0.7f }, { FmAmt, 0.15f }, { FmRatio, 1.004f }, { Click, 0 },
                                       { Drive, 0.7f }, { Level, 0.4f }, { Pan, 0.3f } }),
                // A07 brush ghost: soft swirl, slow attack, rolled-off top
                make (VT::Snare,     { { Wave, tri }, { Pitch, 180 }, { PEnvAmt, 2 }, { ToneDecay, 40 }, { ToneLevel, 0.15f },
                                       { NoiseLevel, 1 }, { NoiseDecay, 220 }, { FilterMode, LP }, { Cutoff, 4500 }, { Reso, 0.6f },
                                       { Click, 0 }, { Attack, 15 }, { Level, 0.5f }, { Pan, -0.2f } }),
                // A08 three stacked claps with MPC-style crunch
                make (VT::Clap,      { { ToneLevel, 0 }, { NoiseLevel, 1 }, { NoiseDecay, 260 }, { FilterMode, BP }, { Cutoff, 1300 },
                                       { Reso, 1.4f }, { Burst, 3 }, { Click, 0 }, { Drive, 0.4f }, { Level, 0.8f } }),
            };
            namesA = { "Jazz Sub Kick", "Memphis Bite Snare", "Silk Rim Stab", "Rage Trap Hat Closed",
                             "Chiff Hat Open", "Phonk Cowbell F#", "Jazz Brush Ghost", "Trap Clap Stack" };
            // ---- bank 2: pads A09..A16 ----
            bankB = {
                // A09 deep tom with body resonance, driven
                make (VT::Tom,       { { Wave, sine }, { Pitch, 82 }, { PEnvAmt, 9 }, { PEnvDecay, 80 }, { ToneDecay, 650 },
                                       { NoiseLevel, 0.08f }, { NoiseDecay, 50 }, { Click, 0.15f }, { Drive, 0.45f }, { Level, 0.85f }, { Pan, -0.25f } }),
                // A10 drum-machine mid tom with a snappy attack
                make (VT::Tom,       { { Wave, tri }, { Pitch, 165 }, { PEnvAmt, 8 }, { PEnvDecay, 40 }, { ToneDecay, 260 },
                                       { Click, 0.4f }, { Drive, 0.3f }, { Level, 0.75f }, { Pan, 0.2f } }),
                // A11 vocal-ish stab: buzzy saw through a resonant "ah" formant (synth stand-in for a vocal chop)
                make (VT::Perc,      { { Wave, saw }, { Pitch, 185 }, { PEnvAmt, 0 }, { ToneDecay, 220 }, { ToneLevel, 0.6f },
                                       { FmAmt, 1.5f }, { FmRatio, 2.0f }, { FmDecay, 120 },
                                       { NoiseLevel, 0.6f }, { NoiseDecay, 200 }, { FilterMode, BP }, { Cutoff, 850 }, { Reso, 5 },
                                       { Click, 0 }, { Attack, 4 }, { Drive, 0.5f }, { Level, 0.5f } }),
                // A12 smooth-jazz FM chime: carrier 1x, modulator 1.414x, 300 ms index decay into a pure sine tail
                make (VT::Perc,      { { Wave, sine }, { Pitch, 1046.5f }, { PEnvAmt, 0 }, { ToneDecay, 1800 }, { ToneLevel, 0.8f },
                                       { FmAmt, 3 }, { FmRatio, 1.414f }, { FmDecay, 300 }, { Click, 0 },
                                       { Level, 0.4f }, { Pan, 0.25f }, { LoFiSend, 0.4f } }),
                // A13 808 sub with a downward glide, saturated, no crush at all
                make (VT::Sub808,      { { Wave, sine }, { Pitch, 43.65f }, { PEnvAmt, 7 }, { PEnvDecay, 180 }, { ToneDecay, 1800 },
                                       { Click, 0.05f }, { Drive, 0.25f }, { Level, 0.9f }, { LoFiSend, 0 } }),
                // A14 vinyl crackle hit: high-passed noise bursts
                make (VT::Perc,      { { ToneLevel, 0 }, { NoiseType, white }, { NoiseLevel, 1 }, { NoiseDecay, 900 },
                                       { FilterMode, HP }, { Cutoff, 3000 }, { Reso, 0.7f }, { Burst, 6 }, { Click, 0.3f },
                                       { Level, 0.3f } }),
                // A15 hand-dampened jazz crash
                make (VT::Cymbal,    { { ToneLevel, 0 }, { NoiseType, metal }, { Pitch, 470 }, { NoiseLevel, 1 }, { NoiseDecay, 650 },
                                       { FilterMode, HP }, { Cutoff, 4500 }, { Reso, 0.8f }, { Click, 0 }, { Level, 0.45f }, { Pan, -0.2f } }),
                // A16 egg shaker: soft swell, double grain
                make (VT::Shaker,      { { ToneLevel, 0 }, { NoiseType, white }, { NoiseLevel, 1 }, { NoiseDecay, 70 },
                                       { FilterMode, HP }, { Cutoff, 6000 }, { Reso, 0.7f }, { Burst, 2 }, { Click, 0 },
                                       { Attack, 12 }, { Level, 0.45f }, { Pan, 0.3f }, { LoFiSend, 0.6f } }),
            };
            namesB = { "Low Tom Drive", "Mid Tom Bite", "Phonk Vox Stab", "Smooth Chime Perc",
                             "Sub Glide 808", "Vinyl Crackle", "Crash Jazz Muted", "Shaker LoFi Groove" };
            haveBankB = true;
            k.bus = machineBus (1);
            k.bus[BLowKeep] = 100.0f;
            k.bus[BPunch] = 0.25f;   // about +1.5 dB on the attack
            k.bus[BVinyl] = 0.12f;
            break;
    }

    for (int t = 0; t < 8; ++t)
    {
        k.voices[(size_t) t] = bankA[(size_t) t];
        k.voiceNames[(size_t) t] = namesA[(size_t) t];
        if (haveBankB)
        {
            k.voices[(size_t) t + 8] = bankB[(size_t) t];
            k.voiceNames[(size_t) t + 8] = namesB[(size_t) t];
        }
    }
    if (! haveBankB)
        deriveBankTwo (k);

    for (int t = 0; t < numTracks; ++t)
        if (k.voiceNames[(size_t) t].isEmpty())
            k.voiceNames[(size_t) t] = voiceTypeNames()[(int) k.voices[(size_t) t][Type]];

    return k;
}

BusDef defaultBus()
{
    BusDef b {};
    for (int i = 0; i < NumBP; ++i)
        b[(size_t) i] = busSpec (i).def;
    return b;
}

BusDef machineBus (int machineIndex)
{
    auto b = machinePreset (machineIndex);
    BusDef out {};
    for (int i = 0; i < NumBP; ++i)
        out[(size_t) i] = b[(size_t) i];
    return out;
}

//==============================================================================
// Forging: each drum type has a "recipe" -- ranges inside which a convincing
// sound of that type lives. Forging rolls dice inside those ranges.
//==============================================================================
VoiceDef forgeVoice (VoiceType type, juce::Random& r)
{
    auto v = defaultVoice();
    v[Type] = (float) type;
    v[Pan] = uni (r, -0.3f, 0.3f);
    v[Choke] = 0;

    switch (type)
    {
        case VoiceType::Kick:
            v[Wave] = chance (r, 0.8f) ? sine : tri;
            v[Pitch] = logu (r, 38, 70);
            v[PEnvAmt] = uni (r, 12, 48);
            v[PEnvDecay] = logu (r, 12, 90);
            v[ToneDecay] = logu (r, 180, 1300);
            v[ToneLevel] = 1;
            v[FmAmt] = chance (r, 0.2f) ? uni (r, 0, 1.5f) : 0;
            v[FmRatio] = chance (r, 0.5f) ? 0.5f : 2.0f;
            v[NoiseLevel] = uni (r, 0, 0.15f);
            v[NoiseDecay] = logu (r, 10, 60);
            v[FilterMode] = LP;
            v[Cutoff] = logu (r, 1500, 8000);
            v[Click] = uni (r, 0.05f, 0.55f);
            v[Drive] = chance (r, 0.6f) ? uni (r, 0, 0.7f) : 0;
            v[Level] = uni (r, 0.85f, 1.0f);
            v[Pan] = 0;
            v[LoFiSend] = uni (r, 0.3f, 0.8f); // keep kicks fairly clean
            break;

        case VoiceType::Snare:
            v[Wave] = chance (r, 0.6f) ? tri : sine;
            v[Pitch] = logu (r, 150, 300);
            v[PEnvAmt] = uni (r, 3, 14);
            v[PEnvDecay] = logu (r, 8, 40);
            v[ToneDecay] = logu (r, 50, 200);
            v[ToneLevel] = uni (r, 0.4f, 0.9f);
            v[FmAmt] = chance (r, 0.25f) ? uni (r, 0, 4) : 0;
            v[FmRatio] = uni (r, 1.2f, 2.8f);
            v[NoiseType] = white;
            v[NoiseLevel] = uni (r, 0.5f, 1.0f);
            v[NoiseDecay] = logu (r, 110, 350);
            v[FilterMode] = chance (r, 0.5f) ? HP : BP;
            v[Cutoff] = logu (r, 1500, 7000);
            v[Reso] = uni (r, 0.5f, 1.5f);
            v[Click] = uni (r, 0.1f, 0.4f);
            v[Drive] = chance (r, 0.5f) ? uni (r, 0, 0.5f) : 0;
            v[Level] = uni (r, 0.7f, 0.9f);
            break;

        case VoiceType::Clap:
            v[ToneLevel] = 0;
            v[NoiseType] = white;
            v[NoiseLevel] = 1;
            v[NoiseDecay] = logu (r, 140, 420);
            v[FilterMode] = BP;
            v[Cutoff] = logu (r, 800, 2200);
            v[Reso] = uni (r, 1.0f, 3.0f);
            v[Burst] = (float) r.nextInt ({ 3, 6 });
            v[Click] = 0;
            v[Drive] = chance (r, 0.4f) ? uni (r, 0, 0.5f) : 0;
            v[Level] = uni (r, 0.7f, 0.85f);
            break;

        case VoiceType::ClosedHat:
        case VoiceType::OpenHat:
        {
            const bool open = type == VoiceType::OpenHat;
            v[ToneLevel] = 0;
            v[NoiseType] = chance (r, 0.65f) ? metal : white;
            v[Pitch] = logu (r, 280, 650);
            v[NoiseLevel] = 1;
            v[NoiseDecay] = open ? logu (r, 220, 750) : logu (r, 20, 90);
            v[FilterMode] = chance (r, 0.75f) ? HP : BP;
            v[Cutoff] = logu (r, 6000, 11500);
            v[Reso] = uni (r, 0.6f, 2.5f);
            v[Click] = 0;
            v[Drive] = chance (r, 0.3f) ? uni (r, 0, 0.4f) : 0;
            v[Level] = uni (r, 0.45f, 0.65f);
            v[Choke] = 1;
            break;
        }

        case VoiceType::Tom:
            v[Wave] = chance (r, 0.8f) ? sine : tri;
            v[Pitch] = logu (r, 80, 260);
            v[PEnvAmt] = uni (r, 4, 14);
            v[PEnvDecay] = logu (r, 30, 120);
            v[ToneDecay] = logu (r, 180, 650);
            v[NoiseLevel] = uni (r, 0, 0.2f);
            v[NoiseDecay] = logu (r, 20, 80);
            v[FilterMode] = LP;
            v[Cutoff] = logu (r, 2000, 6000);
            v[Click] = uni (r, 0.05f, 0.3f);
            v[Level] = uni (r, 0.7f, 0.85f);
            break;

        case VoiceType::Rim:
            v[Wave] = chance (r, 0.5f) ? tri : square;
            v[Pitch] = logu (r, 400, 1800);
            v[PEnvAmt] = uni (r, 0, 6);
            v[PEnvDecay] = logu (r, 2, 10);
            v[ToneDecay] = logu (r, 12, 50);
            v[ToneLevel] = uni (r, 0.5f, 0.9f);
            v[FmAmt] = chance (r, 0.4f) ? uni (r, 0, 3) : 0;
            v[FmRatio] = uni (r, 1.5f, 4.0f);
            v[FmDecay] = logu (r, 10, 80);
            v[NoiseLevel] = uni (r, 0.15f, 0.5f);
            v[NoiseDecay] = logu (r, 8, 30);
            v[FilterMode] = chance (r, 0.5f) ? HP : BP;
            v[Cutoff] = logu (r, 2000, 5000);
            v[Click] = uni (r, 0.4f, 0.8f);
            v[Level] = uni (r, 0.5f, 0.7f);
            break;

        case VoiceType::Cymbal:
            v[ToneLevel] = chance (r, 0.3f) ? uni (r, 0, 0.15f) : 0;
            v[Wave] = square;
            v[Pitch] = logu (r, 300, 700);
            v[FmAmt] = uni (r, 0, 4);
            v[FmRatio] = uni (r, 1.3f, 3.7f);
            v[ToneDecay] = logu (r, 300, 1500);
            v[NoiseType] = chance (r, 0.7f) ? metal : white;
            v[NoiseLevel] = 1;
            v[NoiseDecay] = logu (r, 700, 3000);
            v[FilterMode] = chance (r, 0.7f) ? HP : BP;
            v[Cutoff] = logu (r, 3500, 9000);
            v[Reso] = uni (r, 0.6f, 2.0f);
            v[Click] = uni (r, 0, 0.15f);
            v[Level] = uni (r, 0.4f, 0.55f);
            break;

        case VoiceType::Cowbell:
            v[Wave] = square;
            v[Pitch] = logu (r, 450, 900);
            v[PEnvAmt] = 0;
            v[ToneDecay] = logu (r, 100, 400);
            v[ToneLevel] = uni (r, 0.5f, 0.7f);
            v[FmAmt] = uni (r, 0.3f, 2.5f);
            v[FmRatio] = uni (r, 1.3f, 1.6f);
            v[NoiseLevel] = 0;
            v[Click] = 0;
            v[Level] = uni (r, 0.35f, 0.5f);
            break;

        case VoiceType::Sub808:
        {
            // Root notes E1..A#1 so the sub sits in a key.
            static const float roots[] = { 41.20f, 43.65f, 46.25f, 49.00f, 51.91f, 55.00f, 58.27f };
            v[Wave] = sine;
            v[Pitch] = roots[r.nextInt ((int) std::size (roots))];
            v[PEnvAmt] = chance (r, 0.6f) ? uni (r, 3, 12) : 0;   // glide from above
            v[PEnvDecay] = logu (r, 60, 260);
            v[ToneDecay] = logu (r, 800, 2600);
            v[ToneLevel] = 1;
            v[NoiseLevel] = 0;
            v[Click] = uni (r, 0, 0.12f);
            v[Drive] = chance (r, 0.6f) ? uni (r, 0.1f, 0.6f) : 0;  // saturate mids for audibility
            v[Level] = uni (r, 0.8f, 0.95f);
            v[Pan] = 0;
            v[LoFiSend] = 0;   // never crush the sub
            break;
        }

        case VoiceType::Shaker:
            v[ToneLevel] = 0;
            v[NoiseType] = white;
            v[NoiseLevel] = 1;
            v[NoiseDecay] = logu (r, 40, 130);
            v[FilterMode] = chance (r, 0.7f) ? HP : BP;
            v[Cutoff] = logu (r, 5000, 10000);
            v[Reso] = uni (r, 0.6f, 1.5f);
            v[Burst] = (float) r.nextInt ({ 1, 3 });
            v[Attack] = uni (r, 4, 20);
            v[Click] = 0;
            v[Level] = uni (r, 0.4f, 0.55f);
            break;

        case VoiceType::Perc:
            v[Wave] = (float) r.nextInt (4);
            v[Pitch] = logu (r, 100, 2500);
            v[PEnvAmt] = chance (r, 0.6f) ? uni (r, 0, 36) : 0;
            v[PEnvDecay] = logu (r, 5, 150);
            v[ToneDecay] = logu (r, 30, 600);
            v[ToneLevel] = uni (r, 0.4f, 1.0f);
            v[FmAmt] = chance (r, 0.6f) ? uni (r, 0, 8) : 0;
            v[FmRatio] = logu (r, 0.5f, 8.0f);
            v[FmDecay] = logu (r, 30, 4000);
            v[NoiseType] = chance (r, 0.5f) ? metal : white;
            v[NoiseLevel] = chance (r, 0.5f) ? uni (r, 0, 0.5f) : 0;
            v[NoiseDecay] = logu (r, 10, 400);
            v[FilterMode] = (float) r.nextInt (3);
            v[Cutoff] = logu (r, 500, 10000);
            v[Reso] = logu (r, 0.5f, 6.0f);
            v[Click] = uni (r, 0, 0.5f);
            v[Drive] = chance (r, 0.4f) ? uni (r, 0, 0.7f) : 0;
            v[Level] = uni (r, 0.45f, 0.7f);
            break;
    }
    return v;
}

juce::String forgeVoiceName (VoiceType type, juce::Random& r)
{
    static const char* words[] = { "Dusty", "Smoky", "Velvet", "Rusty", "Gritty", "Silk", "Tape", "Hollow",
                                   "Neon", "Basement", "Midnight", "Cassette", "Bitter", "Glass", "Murky", "Warm" };
    return juce::String (words[r.nextInt ((int) std::size (words))]) + " " + voiceTypeNames()[(int) type];
}

Kit forgeKit (juce::Random& r)
{
    using VT = VoiceType;
    static const VT layout[numTracks] = { VT::Kick, VT::Snare, VT::Clap, VT::ClosedHat,
                                          VT::OpenHat, VT::Tom, VT::Rim, VT::Cymbal,
                                          VT::Tom, VT::Tom, VT::Snare, VT::ClosedHat,
                                          VT::Sub808, VT::Cowbell, VT::Cymbal, VT::Shaker };

    static const char* adjectives[] = { "Molten", "Rusty", "Neon", "Hollow", "Crystal", "Grit", "Velvet",
                                        "Static", "Cobalt", "Feral", "Phantom", "Solar", "Brass", "Carbon" };
    static const char* nouns[]      = { "Anvil", "Engine", "Circuit", "Furnace", "Pulse", "Hammer", "Machine",
                                        "Reactor", "Signal", "Foundry", "Drift", "Motor", "Spark", "Vault" };

    Kit k;
    k.name = juce::String (adjectives[r.nextInt ((int) std::size (adjectives))]) + " "
           + nouns[r.nextInt ((int) std::size (nouns))];

    // Occasionally swap the extra slots for something more exotic.
    for (int t = 0; t < numTracks; ++t)
    {
        auto type = layout[t];
        if (t == 5 && chance (r, 0.3f)) type = VT::Perc;
        if (t == 6 && chance (r, 0.4f)) type = chance (r, 0.5f) ? VT::Cowbell : VT::Perc;
        if (t == 13 && chance (r, 0.5f)) type = VT::Perc;
        k.voices[(size_t) t] = forgeVoice (type, r);
        if (t == 8)  k.voices[8][Pitch]  = juce::jmin (k.voices[8][Pitch], 110.0f);  // keep a low tom
        if (t == 9)  k.voices[9][Pitch]  = juce::jmax (k.voices[9][Pitch], 150.0f);  // and a high tom
        k.voiceNames[(size_t) t] = forgeVoiceName (type, r);
    }
    return k;
}

//==============================================================================
Kit captureKit (juce::AudioProcessorValueTreeState& apvts, const juce::String& name,
                const std::array<juce::String, numTracks>& voiceNames)
{
    Kit k;
    k.name = name;
    k.voiceNames = voiceNames;
    for (int t = 0; t < numTracks; ++t)
        for (int vp = 0; vp < NumVP; ++vp)
            k.voices[(size_t) t][(size_t) vp] = getParamValue (apvts, voiceParamID (t, vp));

    k.hasBus = true;
    for (int bp = 0; bp < NumBP; ++bp)
        k.bus[(size_t) bp] = getParamValue (apvts, busParamID (bp));
    return k;
}

void applyBus (juce::AudioProcessorValueTreeState& apvts, const BusDef& bus, bool machineParamsOnly)
{
    for (int bp = 0; bp < NumBP; ++bp)
        if (! machineParamsOnly || isMachineParam (bp))
            setParamValue (apvts, busParamID (bp), bus[(size_t) bp]);
}

void applyVoice (juce::AudioProcessorValueTreeState& apvts, int track, const VoiceDef& voice)
{
    for (int vp = 0; vp < NumVP; ++vp)
        if (isKitParam (vp))
            setParamValue (apvts, voiceParamID (track, vp), voice[(size_t) vp]);
}

void applyKit (juce::AudioProcessorValueTreeState& apvts, const Kit& kit)
{
    for (int t = 0; t < numTracks; ++t)
        applyVoice (apvts, t, kit.voices[(size_t) t]);
    if (kit.hasBus)
        applyBus (apvts, kit.bus);
}

void mutateKit (juce::AudioProcessorValueTreeState& apvts, juce::Random& r, float amount)
{
    for (int t = 0; t < numTracks; ++t)
    {
        for (int vp = 0; vp < NumVP; ++vp)
        {
            const auto& spec = voiceSpec (vp);
            if (! isKitParam (vp) || spec.kind != Kind::Float || vp == Choke || vp == Burst)
                continue;

            const auto id = voiceParamID (t, vp);
            const float current = getParamValue (apvts, id);

            // Keep the character: don't bring silent layers to life.
            if ((vp == ToneLevel || vp == NoiseLevel || vp == FmAmt) && current <= 0.0f)
                continue;

            if (auto* p = apvts.getParameter (id))
            {
                float norm = p->convertTo0to1 (current);
                norm = juce::jlimit (0.0f, 1.0f, norm + (r.nextFloat() * 2.0f - 1.0f) * amount * 0.2f);
                p->beginChangeGesture();
                p->setValueNotifyingHost (norm);
                p->endChangeGesture();
            }
        }
    }
}

//==============================================================================
std::unique_ptr<juce::XmlElement> kitToXml (const Kit& kit)
{
    auto xml = std::make_unique<juce::XmlElement> ("KitForgeKit");
    xml->setAttribute ("name", kit.name);
    xml->setAttribute ("version", 1);

    for (int t = 0; t < numTracks; ++t)
    {
        auto* voice = xml->createNewChildElement ("Voice");
        voice->setAttribute ("index", t);
        voice->setAttribute ("name", kit.voiceNames[(size_t) t]);
        for (int vp = 0; vp < NumVP; ++vp)
            if (isKitParam (vp))
                voice->setAttribute (voiceSpec (vp).id, (double) kit.voices[(size_t) t][(size_t) vp]);
    }

    if (kit.hasBus)
    {
        auto* bus = xml->createNewChildElement ("Bus");
        for (int bp = 0; bp < NumBP; ++bp)
            bus->setAttribute (busSpec (bp).id, (double) kit.bus[(size_t) bp]);
    }
    return xml;
}

bool kitFromXml (const juce::XmlElement& xml, Kit& kit)
{
    if (! xml.hasTagName ("KitForgeKit"))
        return false;

    kit.name = xml.getStringAttribute ("name", "Untitled Kit");
    for (auto& v : kit.voices)
        v = defaultVoice();
    for (auto& n : kit.voiceNames)
        n = {};
    kit.hasBus = false;
    kit.bus = defaultBus();

    if (auto* bus = xml.getChildByName ("Bus"))
    {
        kit.hasBus = true;
        for (int bp = 0; bp < NumBP; ++bp)
        {
            const auto& s = busSpec (bp);
            if (bus->hasAttribute (s.id))
                kit.bus[(size_t) bp] = juce::jlimit (s.min, s.max, (float) bus->getDoubleAttribute (s.id));
        }
    }

    int maxIndex = -1;
    for (auto* voice : xml.getChildWithTagNameIterator ("Voice"))
    {
        const int t = voice->getIntAttribute ("index", -1);
        if (t < 0 || t >= numTracks)
            continue;
        kit.voiceNames[(size_t) t] = voice->getStringAttribute ("name");
        maxIndex = juce::jmax (maxIndex, t);

        for (int vp = 0; vp < NumVP; ++vp)
        {
            const auto& s = voiceSpec (vp);
            if (isKitParam (vp) && voice->hasAttribute (s.id))
                kit.voices[(size_t) t][(size_t) vp] = juce::jlimit (s.min, s.max, (float) voice->getDoubleAttribute (s.id));
        }
    }
    for (int t = 0; t < numTracks; ++t)
        if (kit.voiceNames[(size_t) t].isEmpty())
            kit.voiceNames[(size_t) t] = voiceTypeNames()[(int) kit.voices[(size_t) t][Type]];
    // Kits saved with 8 pads get a derived second bank.
    if (maxIndex < 8)
        deriveBankTwo (kit);

    return true;
}

//==============================================================================
// Pattern generation
//==============================================================================
void generatePattern (PatternBank& bank, int pattern, juce::AudioProcessorValueTreeState& apvts, juce::Random& r)
{
    constexpr uint8_t ghost = 45, normal = 100, accent = 127;
    bank.clear (pattern);

    // Pick a feel. Trap = half-time snare on beat 3, 808 sub, hat rolls.
    const float feel = r.nextFloat();
    const bool trap = feel < 0.4f;
    const bool fourOnFloor = ! trap && feel < 0.7f;
    const bool sixteenthHats = ! trap && chance (r, 0.4f);

    for (int t = 0; t < numTracks; ++t)
    {
        const auto type = (VoiceType) (int) getParamValue (apvts, voiceParamID (t, Type));
        auto put = [&] (int step, uint8_t vel) { bank.set (pattern, t, step, vel); };

        switch (type)
        {
            case VoiceType::Kick:
                if (trap)
                {
                    put (0, accent);
                    for (int s : { 3, 6, 7, 10, 11, 14 })
                        if (chance (r, 0.3f)) put (s, normal);
                }
                else if (fourOnFloor)
                {
                    for (int s = 0; s < numSteps; s += 4) put (s, accent);
                    if (chance (r, 0.3f)) put (14 + r.nextInt (2), normal);
                }
                else
                {
                    put (0, accent);
                    put (8, normal);
                    for (int s = 1; s < numSteps; ++s)
                        if (s != 4 && s != 12 && bank.get (pattern, t, s) == 0 && chance (r, s % 2 == 0 ? 0.25f : 0.1f))
                            put (s, chance (r, 0.5f) ? normal : ghost + 30);
                }
                break;

            case VoiceType::Snare:
                if (trap) { put (8, accent); break; }
                put (4, accent);
                put (12, accent);
                for (int s = 0; s < numSteps; ++s)
                    if (bank.get (pattern, t, s) == 0 && chance (r, 0.1f))
                        put (s, ghost);
                break;

            case VoiceType::Clap:
                if (trap) { put (8, accent); break; }
                if (chance (r, 0.6f)) { put (4, normal); put (12, normal); }
                else if (chance (r, 0.5f)) put (12, accent);
                break;

            case VoiceType::ClosedHat:
                if (trap)
                {
                    // 8ths with rolls: 1/32 (x2), 1/16-triplet roll (x3), 1/64 (x4)
                    for (int s = 0; s < numSteps; ++s)
                    {
                        if (s % 2 == 0 || chance (r, 0.35f))
                        {
                            put (s, s % 4 == 0 ? normal : (uint8_t) 80);
                            const float rollChance = s >= 12 ? 0.45f : 0.15f;
                            if (chance (r, rollChance))
                                bank.setRatchet (pattern, t, s, 2 + r.nextInt (maxRatchet - 1));
                        }
                    }
                    break;
                }
                for (int s = 0; s < numSteps; ++s)
                {
                    if (sixteenthHats)
                    {
                        if (chance (r, 0.9f))
                            put (s, s % 4 == 2 ? accent : (s % 2 == 0 ? normal : ghost + 20));
                    }
                    else if (s % 2 == 0)
                    {
                        put (s, s % 4 == 2 ? accent : normal);
                    }
                }
                break;

            case VoiceType::OpenHat:
                for (int s = 2; s < numSteps; s += 4)
                    if (chance (r, fourOnFloor ? 0.7f : 0.3f))
                        put (s, normal);
                break;

            case VoiceType::Cymbal:
                if (chance (r, 0.5f)) put (0, normal);
                break;

            case VoiceType::Sub808:
                // The sub doubles the kick and adds a pickup or two.
                put (0, accent);
                if (trap)
                {
                    for (int s : { 6, 10, 11, 14 })
                        if (chance (r, 0.35f)) put (s, normal);
                }
                else if (chance (r, 0.5f))
                {
                    put (8 + 2 * r.nextInt (3), normal);
                }
                break;

            case VoiceType::Shaker:
                for (int s = 0; s < numSteps; ++s)
                    if (chance (r, trap ? 0.25f : 0.8f))
                        put (s, s % 2 == 1 ? (uint8_t) 90 : ghost);
                break;

            case VoiceType::Tom:
            case VoiceType::Rim:
            case VoiceType::Cowbell:
            case VoiceType::Perc:
                for (int s = 0; s < numSteps; ++s)
                    if (chance (r, 0.07f))
                        put (s, chance (r, 0.5f) ? normal : ghost + 20);
                break;
        }
    }

    // An open hat and a closed hat on the same step would choke each other:
    // let the open hat win.
    for (int s = 0; s < numSteps; ++s)
    {
        bool openHere = false;
        for (int t = 0; t < numTracks; ++t)
            openHere |= (VoiceType) (int) getParamValue (apvts, voiceParamID (t, Type)) == VoiceType::OpenHat
                        && bank.get (pattern, t, s) > 0;
        if (openHere)
            for (int t = 0; t < numTracks; ++t)
                if ((VoiceType) (int) getParamValue (apvts, voiceParamID (t, Type)) == VoiceType::ClosedHat)
                    bank.set (pattern, t, s, 0);
    }
}

void writeDefaultPatterns (PatternBank& bank)
{
    bank.clearAll();

    auto row = [&bank] (int pattern, int track, std::initializer_list<std::pair<int, int>> hits)
    {
        for (auto [step, vel] : hits)
            bank.set (pattern, track, step, (uint8_t) vel);
    };

    // A: boom-bap
    row (0, 0, { { 0, 127 }, { 7, 80 }, { 8, 110 }, { 10, 100 } });
    row (0, 1, { { 4, 127 }, { 12, 127 }, { 15, 45 } });
    row (0, 3, { { 0, 100 }, { 2, 70 }, { 4, 100 }, { 6, 70 }, { 8, 100 }, { 10, 70 }, { 12, 100 } });
    row (0, 4, { { 14, 90 } });
    row (0, 7, { { 0, 70 } });

    // B: four on the floor
    row (1, 0, { { 0, 127 }, { 4, 127 }, { 8, 127 }, { 12, 127 } });
    row (1, 2, { { 4, 110 }, { 12, 110 } });
    row (1, 3, { { 0, 80 }, { 1, 50 }, { 3, 50 }, { 4, 80 }, { 5, 50 }, { 7, 50 }, { 8, 80 }, { 9, 50 }, { 11, 50 }, { 12, 80 }, { 13, 50 }, { 15, 50 } });
    row (1, 4, { { 2, 100 }, { 6, 100 }, { 10, 100 }, { 14, 100 } });
    row (1, 5, { { 15, 90 } });
    row (1, 6, { { 3, 60 }, { 11, 60 } });
    row (1, 15, { { 1, 50 }, { 3, 80 }, { 5, 50 }, { 7, 80 }, { 9, 50 }, { 11, 80 }, { 13, 50 }, { 15, 80 } });

    // C: trap -- half-time clap, 808 sub, hat rolls
    row (2, 0, { { 0, 127 }, { 7, 100 }, { 11, 90 } });
    row (2, 12, { { 0, 127 }, { 7, 110 }, { 11, 100 }, { 14, 90 } });
    row (2, 2, { { 8, 127 } });
    row (2, 3, { { 0, 100 }, { 2, 80 }, { 4, 100 }, { 6, 80 }, { 8, 100 }, { 10, 80 }, { 12, 100 }, { 13, 80 }, { 14, 90 }, { 15, 90 } });
    bank.setRatchet (2, 3, 6, 2);   // 1/32
    bank.setRatchet (2, 3, 13, 3);  // triplet roll
    bank.setRatchet (2, 3, 14, 4);  // 1/64
    bank.setRatchet (2, 3, 15, 2);
    row (2, 4, { { 10, 90 } });
    row (2, 11, { { 3, 60 } });
    bank.setRatchet (2, 11, 3, 4);
}
} // namespace kf::KitFactory
