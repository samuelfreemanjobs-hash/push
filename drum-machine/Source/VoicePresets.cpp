#include "VoicePresets.h"
#include "Exporter.h"

namespace kf::Presets
{
namespace
{
    constexpr float sine = 0, tri = 1, saw = 2, square = 3;
    constexpr float white = 0, metal = 1;
    constexpr float LP = 0, BP = 1, HP = 2;
    using VT = VoiceType;

    Preset p (const char* name, VT type, std::initializer_list<std::pair<VP, float>> o)
    {
        return { name, type, KitFactory::makeVoice (type, o), "Factory" };
    }

    std::vector<Preset> buildFactory()
    {
        std::vector<Preset> v;

        //------------------------------------------------------------------ Kick
        v.push_back (p ("808 Long Boom", VT::Kick, { { Wave, sine }, { Pitch, 48 }, { PEnvAmt, 24 }, { PEnvDecay, 40 }, { ToneDecay, 2500 },
                                                    { Hold, 300 }, { Click, 0.1f }, { Drive, 0.15f }, { Level, 0.95f } }));
        v.push_back (p ("909 Punch", VT::Kick, { { Wave, sine }, { Pitch, 55 }, { PEnvAmt, 36 }, { PEnvDecay, 30 }, { ToneDecay, 420 },
                                                { Click, 0.5f }, { Drive, 0.35f }, { Punch, 0.4f }, { Level, 0.95f } }));
        v.push_back (p ("Phonk Distorted Kick", VT::Kick, { { Wave, sine }, { Pitch, 50 }, { PEnvAmt, 40 }, { PEnvDecay, 35 }, { ToneDecay, 600 },
                                                           { Click, 0.4f }, { Drive, 0.85f }, { Punch, 0.5f }, { Level, 0.9f } }));
        v.push_back (p ("Boom Bap Thud", VT::Kick, { { Wave, tri }, { Pitch, 52 }, { PEnvAmt, 18 }, { PEnvDecay, 45 }, { ToneDecay, 320 },
                                                    { NoiseLevel, 0.15f }, { NoiseDecay, 50 }, { FilterMode, LP }, { Cutoff, 2000 },
                                                    { Click, 0.25f }, { Attack, 1 }, { Drive, 0.3f }, { Level, 0.95f } }));
        v.push_back (p ("Trap Knock", VT::Kick, { { Wave, sine }, { Pitch, 58 }, { PEnvAmt, 30 }, { PEnvDecay, 20 }, { ToneDecay, 250 },
                                                 { Click, 0.7f }, { Punch, 0.8f }, { PunchTime, 10 }, { Drive, 0.5f }, { Level, 0.95f } }));
        v.push_back (p ("Hard Techno", VT::Kick, { { Wave, sine }, { Pitch, 50 }, { PEnvAmt, 48 }, { PEnvDecay, 22 }, { ToneDecay, 380 },
                                                  { Click, 0.6f }, { Drive, 0.9f }, { Punch, 0.6f }, { Level, 0.85f } }));
        v.push_back (p ("Soft Jazz Kick", VT::Kick, { { Wave, sine }, { Pitch, 60 }, { PEnvAmt, 12 }, { PEnvDecay, 30 }, { ToneDecay, 260 },
                                                     { NoiseLevel, 0.1f }, { NoiseDecay, 70 }, { FilterMode, LP }, { Cutoff, 1500 },
                                                     { Click, 0.1f }, { Punch, -0.2f }, { Level, 0.95f } }));
        v.push_back (p ("Lo-Fi Pillow", VT::Kick, { { Wave, tri }, { Pitch, 48 }, { PEnvAmt, 14 }, { PEnvDecay, 50 }, { ToneDecay, 450 },
                                                   { Attack, 3 }, { Drive, 0.4f }, { Punch, -0.4f }, { Level, 0.95f } }));

        //------------------------------------------------------------------ 808 Sub
        auto sub = [&] (const char* name, std::initializer_list<std::pair<VP, float>> o)
        {
            auto pr = p (name, VT::Sub808, o);
            pr.voice[LoFiSend] = 0.0f;
            pr.voice[Pan] = 0.0f;
            v.push_back (pr);
        };
        sub ("808 Long Sub", { { Wave, sine }, { Pitch, 43.65f }, { PEnvAmt, 5 }, { PEnvDecay, 120 }, { ToneDecay, 4000 }, { Hold, 600 },
                               { Click, 0.05f }, { Drive, 0.2f }, { Level, 0.9f } });
        sub ("808 Glide Slide", { { Wave, sine }, { Pitch, 43.65f }, { PEnvAmt, 7 }, { PEnvDecay, 150 }, { ToneDecay, 3000 }, { Hold, 400 },
                                  { Glide, 120 }, { Click, 0.05f }, { Drive, 0.3f }, { Level, 0.9f } });
        sub ("Phonk Distorted 808", { { Wave, sine }, { Pitch, 46.25f }, { PEnvAmt, 12 }, { PEnvDecay, 80 }, { ToneDecay, 3500 }, { Hold, 300 },
                                      { Glide, 80 }, { Click, 0.1f }, { Drive, 0.9f }, { Level, 0.85f } });
        sub ("Drift 808 Growl", { { Wave, tri }, { Pitch, 41.2f }, { PEnvAmt, 10 }, { PEnvDecay, 90 }, { ToneDecay, 2800 }, { Hold, 500 },
                                  { Glide, 100 }, { FmAmt, 0.8f }, { FmRatio, 2 }, { FmDecay, 600 }, { Drive, 0.75f }, { Level, 0.85f } });
        sub ("Memphis Boom", { { Wave, sine }, { Pitch, 46.25f }, { PEnvAmt, 24 }, { PEnvDecay, 60 }, { ToneDecay, 2200 }, { Hold, 200 },
                               { Click, 0.2f }, { Drive, 0.5f }, { Level, 0.9f } });
        sub ("Short 808 Punch", { { Wave, sine }, { Pitch, 49 }, { PEnvAmt, 18 }, { PEnvDecay, 50 }, { ToneDecay, 700 },
                                  { Click, 0.25f }, { Drive, 0.4f }, { Punch, 0.4f }, { Level, 0.9f } });
        sub ("Deep Sine Sub", { { Wave, sine }, { Pitch, 41.2f }, { PEnvAmt, 0 }, { ToneDecay, 6000 }, { Hold, 1200 },
                                { Attack, 2 }, { Click, 0 }, { Level, 0.9f } });

        //------------------------------------------------------------------ Snare
        v.push_back (p ("Phonk Clap-Snare", VT::Snare, { { Wave, tri }, { Pitch, 220 }, { PEnvAmt, 12 }, { PEnvDecay, 15 }, { ToneDecay, 80 },
                                                        { ToneLevel, 0.5f }, { NoiseLevel, 1 }, { NoiseDecay, 220 }, { FilterMode, BP },
                                                        { Cutoff, 2400 }, { Reso, 1.3f }, { Click, 0.5f }, { Drive, 0.65f }, { Punch, 0.5f }, { Level, 0.8f } }));
        v.push_back (p ("909 Snare", VT::Snare, { { Wave, tri }, { Pitch, 210 }, { PEnvAmt, 12 }, { PEnvDecay, 20 }, { ToneDecay, 90 },
                                                 { ToneLevel, 0.7f }, { NoiseLevel, 0.9f }, { NoiseDecay, 230 }, { FilterMode, BP },
                                                 { Cutoff, 4500 }, { Reso, 0.8f }, { Click, 0.35f }, { Drive, 0.2f } }));
        v.push_back (p ("808 Snare", VT::Snare, { { Wave, sine }, { Pitch, 185 }, { PEnvAmt, 8 }, { PEnvDecay, 15 }, { ToneDecay, 110 },
                                                 { ToneLevel, 0.6f }, { NoiseLevel, 0.8f }, { NoiseDecay, 200 }, { FilterMode, HP },
                                                 { Cutoff, 1800 }, { Click, 0.2f } }));
        v.push_back (p ("Boom Bap Crack", VT::Snare, { { Wave, tri }, { Pitch, 200 }, { PEnvAmt, 10 }, { PEnvDecay, 18 }, { ToneDecay, 120 },
                                                      { ToneLevel, 0.6f }, { NoiseLevel, 1 }, { NoiseDecay, 260 }, { FilterMode, BP },
                                                      { Cutoff, 3000 }, { Reso, 1 }, { Click, 0.4f }, { Drive, 0.45f }, { Punch, 0.3f } }));
        v.push_back (p ("Trap Snare", VT::Snare, { { Wave, tri }, { Pitch, 240 }, { PEnvAmt, 14 }, { PEnvDecay, 12 }, { ToneDecay, 70 },
                                                  { ToneLevel, 0.55f }, { NoiseLevel, 1 }, { NoiseDecay, 180 }, { FilterMode, HP },
                                                  { Cutoff, 2200 }, { Click, 0.45f }, { Punch, 0.6f } }));
        v.push_back (p ("Lo-Fi Snare", VT::Snare, { { Wave, sine }, { Pitch, 170 }, { PEnvAmt, 5 }, { PEnvDecay, 20 }, { ToneDecay, 80 },
                                                   { ToneLevel, 0.7f }, { NoiseLevel, 0.6f }, { NoiseDecay, 160 }, { FilterMode, LP },
                                                   { Cutoff, 3500 }, { Reso, 0.9f }, { Click, 0.1f }, { Drive, 0.4f }, { Punch, -0.2f } }));
        v.push_back (p ("Brush Ghost", VT::Snare, { { Wave, tri }, { Pitch, 180 }, { PEnvAmt, 2 }, { ToneDecay, 40 }, { ToneLevel, 0.15f },
                                                   { NoiseLevel, 1 }, { NoiseDecay, 220 }, { FilterMode, LP }, { Cutoff, 4500 },
                                                   { Reso, 0.6f }, { Click, 0 }, { Attack, 15 }, { Level, 0.5f } }));

        //------------------------------------------------------------------ Clap
        auto clap = [&] (const char* name, float cutoff, float reso, float bursts, float decay, float drive, float punch)
        {
            v.push_back (p (name, VT::Clap, { { ToneLevel, 0 }, { NoiseLevel, 1 }, { NoiseDecay, decay }, { FilterMode, BP },
                                              { Cutoff, cutoff }, { Reso, reso }, { Burst, bursts }, { Click, 0 },
                                              { Drive, drive }, { Punch, punch }, { Level, 0.8f } }));
        };
        clap ("808 Clap", 1100, 1.6f, 4, 260, 0, 0);
        clap ("Trap Clap Stack", 1300, 1.4f, 3, 260, 0.4f, 0.2f);
        clap ("Phonk Crunch Clap", 1500, 2.0f, 4, 300, 0.7f, 0.4f);
        clap ("Tight Clap", 2200, 2.0f, 3, 120, 0, 0.3f);
        clap ("Big Room Clap", 1000, 1.2f, 5, 450, 0.2f, 0);

        //------------------------------------------------------------------ Hats
        auto hat = [&] (const char* name, VT type, float noise, float pitch, float decay, float mode, float cutoff,
                        float reso, float drive, float level)
        {
            v.push_back (p (name, type, { { ToneLevel, 0 }, { NoiseType, noise }, { Pitch, pitch }, { NoiseLevel, 1 },
                                          { NoiseDecay, decay }, { FilterMode, mode }, { Cutoff, cutoff }, { Reso, reso },
                                          { Click, 0 }, { Drive, drive }, { Level, level }, { Choke, 1 } }));
        };
        hat ("808 Hat",          VT::ClosedHat, metal, 400, 45, HP, 7500, 1.0f, 0, 0.55f);
        hat ("Phonk Hat",        VT::ClosedHat, metal, 460, 35, HP, 8500, 1.4f, 0.3f, 0.55f);
        hat ("Trap Roll Hat",    VT::ClosedHat, metal, 520, 25, HP, 9000, 1.2f, 0, 0.5f);
        hat ("909 Hat",          VT::ClosedHat, white, 400, 55, HP, 9000, 0.7f, 0, 0.55f);
        hat ("Lo-Fi Dusty Hat",  VT::ClosedHat, white, 400, 35, BP, 6000, 0.8f, 0.2f, 0.5f);
        hat ("Tick Hat",         VT::ClosedHat, white, 400, 18, HP, 11000, 0.7f, 0, 0.55f);
        hat ("808 Open Hat",     VT::OpenHat, metal, 400, 420, HP, 7500, 1.0f, 0, 0.5f);
        hat ("Phonk Open Hat",   VT::OpenHat, metal, 460, 350, HP, 8000, 1.4f, 0.3f, 0.5f);
        hat ("909 Open Hat",     VT::OpenHat, white, 400, 500, HP, 8500, 0.7f, 0, 0.5f);
        hat ("Sizzle",           VT::OpenHat, metal, 600, 700, HP, 9000, 2.0f, 0, 0.45f);
        v.push_back (p ("Brush Swish", VT::OpenHat, { { ToneLevel, 0 }, { NoiseType, white }, { NoiseLevel, 1 }, { NoiseDecay, 380 },
                                                     { FilterMode, BP }, { Cutoff, 7000 }, { Reso, 0.7f }, { Click, 0 }, { Attack, 6 },
                                                     { Level, 0.45f }, { Choke, 1 } }));

        //------------------------------------------------------------------ Toms
        auto tom = [&] (const char* name, float wave, float pitch, float penv, float pdec, float decay, float click,
                        float drive, std::initializer_list<std::pair<VP, float>> extra = {})
        {
            auto pr = p (name, VT::Tom, { { Wave, wave }, { Pitch, pitch }, { PEnvAmt, penv }, { PEnvDecay, pdec },
                                          { ToneDecay, decay }, { Click, click }, { Drive, drive }, { Level, 0.8f } });
            for (auto& [vp, val] : extra) pr.voice[vp] = val;
            v.push_back (pr);
        };
        tom ("808 Low Tom", sine, 90, 7, 60, 450, 0.1f, 0);
        tom ("808 Mid Tom", sine, 130, 7, 50, 380, 0.1f, 0);
        tom ("808 High Tom", sine, 180, 7, 40, 300, 0.1f, 0);
        tom ("909 Tom", sine, 140, 10, 50, 300, 0.25f, 0);
        tom ("Electro Tom", sine, 110, 24, 120, 400, 0.1f, 0.2f, { { FmAmt, 1 }, { FmRatio, 1.5f } });
        tom ("Driven Floor Tom", sine, 75, 9, 80, 650, 0.15f, 0.5f);

        //------------------------------------------------------------------ Rim
        v.push_back (p ("808 Rim", VT::Rim, { { Wave, tri }, { Pitch, 1700 }, { PEnvAmt, 3 }, { PEnvDecay, 5 }, { ToneDecay, 25 },
                                             { NoiseLevel, 0.3f }, { NoiseDecay, 15 }, { FilterMode, HP }, { Cutoff, 3000 },
                                             { Click, 0.6f }, { Level, 0.6f } }));
        v.push_back (p ("Side Stick", VT::Rim, { { Wave, tri }, { Pitch, 1250 }, { PEnvAmt, 4 }, { PEnvDecay, 4 }, { ToneDecay, 40 },
                                                { FmAmt, 1.2f }, { FmRatio, 2.3f }, { FmDecay, 30 }, { NoiseLevel, 0.3f }, { NoiseDecay, 60 },
                                                { FilterMode, BP }, { Cutoff, 3500 }, { Reso, 1.5f }, { Click, 0.5f }, { Level, 0.6f } }));
        v.push_back (p ("Wood Block", VT::Rim, { { Wave, sine }, { Pitch, 900 }, { PEnvAmt, 2 }, { PEnvDecay, 5 }, { ToneDecay, 60 },
                                                { FmAmt, 1.2f }, { FmRatio, 2.3f }, { FmDecay, 30 }, { Click, 0.3f }, { Level, 0.6f } }));
        v.push_back (p ("Click Rim", VT::Rim, { { Wave, tri }, { Pitch, 1200 }, { PEnvAmt, 0 }, { ToneDecay, 15 }, { Click, 0.8f }, { Level, 0.55f } }));

        //------------------------------------------------------------------ Cymbal
        v.push_back (p ("808 Crash", VT::Cymbal, { { ToneLevel, 0 }, { NoiseType, metal }, { Pitch, 450 }, { NoiseLevel, 1 }, { NoiseDecay, 1800 },
                                                  { FilterMode, HP }, { Cutoff, 5500 }, { Reso, 0.8f }, { Click, 0 }, { Level, 0.5f } }));
        v.push_back (p ("Ride Ping", VT::Cymbal, { { Wave, square }, { Pitch, 620 }, { ToneLevel, 0.2f }, { PEnvAmt, 0 }, { FmAmt, 3 },
                                                  { FmRatio, 2.7f }, { ToneDecay, 800 }, { NoiseType, metal }, { NoiseLevel, 1 },
                                                  { NoiseDecay, 1500 }, { FilterMode, BP }, { Cutoff, 8000 }, { Reso, 1.5f }, { Click, 0.1f }, { Level, 0.45f } }));
        v.push_back (p ("Muted Crash", VT::Cymbal, { { ToneLevel, 0 }, { NoiseType, metal }, { Pitch, 470 }, { NoiseLevel, 1 }, { NoiseDecay, 650 },
                                                    { FilterMode, HP }, { Cutoff, 4500 }, { Reso, 0.8f }, { Click, 0 }, { Level, 0.45f } }));
        v.push_back (p ("Trash Crash", VT::Cymbal, { { ToneLevel, 0 }, { NoiseType, white }, { NoiseLevel, 1 }, { NoiseDecay, 1200 },
                                                    { FilterMode, HP }, { Cutoff, 3000 }, { Reso, 0.9f }, { Click, 0.1f }, { Drive, 0.6f }, { Level, 0.4f } }));

        //------------------------------------------------------------------ Cowbell
        auto bell = [&] (const char* name, std::initializer_list<std::pair<VP, float>> o)
        {
            auto pr = p (name, VT::Cowbell, { { Wave, square }, { PEnvAmt, 0 }, { Click, 0 }, { Level, 0.45f } });
            for (auto& [vp, val] : o) pr.voice[vp] = val;
            v.push_back (pr);
        };
        bell ("808 Cowbell", { { Pitch, 560 }, { ToneDecay, 250 }, { ToneLevel, 0.6f }, { FmAmt, 0.8f }, { FmRatio, 1.48f } });
        bell ("Phonk Cowbell F#", { { Pitch, 739.99f }, { PEnvAmt, 12 }, { PEnvDecay, 15 }, { ToneDecay, 220 }, { ToneLevel, 0.7f },
                                    { FmAmt, 0.15f }, { FmRatio, 1.004f }, { Drive, 0.7f }, { Level, 0.4f } });
        bell ("Drift Cowbell Ring", { { Pitch, 554.37f }, { PEnvAmt, 6 }, { PEnvDecay, 10 }, { ToneDecay, 600 }, { Hold, 60 },
                                      { ToneLevel, 0.7f }, { FmAmt, 0.4f }, { FmRatio, 1.48f }, { Drive, 0.55f }, { Level, 0.4f } });
        bell ("Memphis Tape Bell", { { Pitch, 587.33f }, { ToneDecay, 300 }, { ToneLevel, 0.7f }, { FmAmt, 0.8f }, { FmRatio, 1.48f },
                                     { Drive, 0.4f }, { Punch, 0.3f } });
        bell ("Distorted Bell Lead", { { Pitch, 659.25f }, { PEnvAmt, 3 }, { PEnvDecay, 8 }, { ToneDecay, 900 }, { Hold, 120 },
                                       { ToneLevel, 0.6f }, { FmAmt, 1 }, { FmRatio, 1.505f }, { Drive, 0.9f }, { Level, 0.35f } });
        bell ("Glassy Cowbell", { { Wave, sine }, { Pitch, 800 }, { ToneDecay, 300 }, { FmAmt, 2 }, { FmRatio, 1.33f }, { FmDecay, 150 } });

        //------------------------------------------------------------------ Perc
        v.push_back (p ("FM Chime", VT::Perc, { { Wave, sine }, { Pitch, 1046.5f }, { PEnvAmt, 0 }, { ToneDecay, 1800 }, { ToneLevel, 0.8f },
                                               { FmAmt, 3 }, { FmRatio, 1.414f }, { FmDecay, 300 }, { Click, 0 }, { Level, 0.4f } }));
        v.push_back (p ("Vox Stab", VT::Perc, { { Wave, saw }, { Pitch, 185 }, { PEnvAmt, 0 }, { ToneDecay, 220 }, { ToneLevel, 0.6f },
                                               { FmAmt, 1.5f }, { FmRatio, 2 }, { FmDecay, 120 }, { NoiseLevel, 0.6f }, { NoiseDecay, 200 },
                                               { FilterMode, BP }, { Cutoff, 850 }, { Reso, 5 }, { Click, 0 }, { Attack, 4 },
                                               { Drive, 0.5f }, { Level, 0.5f } }));
        v.push_back (p ("Blip", VT::Perc, { { Wave, sine }, { Pitch, 2200 }, { PEnvAmt, 0 }, { ToneDecay, 60 }, { FmAmt, 2 }, { FmRatio, 3.1f },
                                           { Click, 0 }, { Level, 0.35f } }));
        v.push_back (p ("Conga", VT::Perc, { { Wave, sine }, { Pitch, 330 }, { PEnvAmt, 7 }, { PEnvDecay, 15 }, { ToneDecay, 180 },
                                            { Click, 0.3f }, { Level, 0.6f } }));
        v.push_back (p ("Triangle", VT::Perc, { { Wave, sine }, { Pitch, 2700 }, { PEnvAmt, 0 }, { ToneDecay, 1500 }, { FmAmt, 0.5f },
                                               { FmRatio, 2.76f }, { Click, 0 }, { Level, 0.3f } }));
        v.push_back (p ("Metal Hit", VT::Perc, { { Wave, square }, { Pitch, 300 }, { PEnvAmt, 0 }, { ToneDecay, 400 }, { ToneLevel, 0.4f },
                                                { FmAmt, 8 }, { FmRatio, 3.7f }, { NoiseType, metal }, { NoiseLevel, 0.3f }, { NoiseDecay, 400 },
                                                { FilterMode, BP }, { Cutoff, 3000 }, { Reso, 6 }, { Click, 0.2f }, { Level, 0.45f } }));

        //------------------------------------------------------------------ Shaker
        auto shaker = [&] (const char* name, float noise, float pitch, float decay, float cutoff, float attack, float bursts)
        {
            v.push_back (p (name, VT::Shaker, { { ToneLevel, 0 }, { NoiseType, noise }, { Pitch, pitch }, { NoiseLevel, 1 },
                                                { NoiseDecay, decay }, { FilterMode, HP }, { Cutoff, cutoff }, { Reso, 0.7f },
                                                { Burst, bursts }, { Click, 0 }, { Attack, attack }, { Level, 0.45f } }));
        };
        shaker ("Egg Shaker", white, 400, 70, 6000, 12, 2);
        shaker ("Tight Shaker", white, 400, 40, 9000, 4, 1);
        shaker ("Tambourine", metal, 900, 180, 7000, 1, 2);
        shaker ("Cabasa", white, 400, 110, 6000, 15, 1);

        return v;
    }

    juce::File typeFolder (VoiceType type)
    {
        return userFolder().getChildFile (Exporter::sanitise (voiceTypeNames()[(int) type]));
    }
}

const std::vector<Preset>& factory()
{
    static const std::vector<Preset> presets = buildFactory();
    return presets;
}

const Preset* findFactory (const juce::String& name)
{
    for (auto& pr : factory())
        if (pr.name == name)
            return &pr;
    jassertfalse;
    return nullptr;
}

std::vector<Preset> fromKits()
{
    std::vector<Preset> out;
    for (int k = 0; k < KitFactory::factoryKitNames().size(); ++k)
    {
        const auto kit = KitFactory::factoryKit (k);
        for (int t = 0; t < numTracks; ++t)
            out.push_back ({ kit.voiceNames[(size_t) t], (VoiceType) (int) kit.voices[(size_t) t][Type],
                             kit.voices[(size_t) t], kit.name.upToFirstOccurrenceOf (" (", false, false) });
    }
    return out;
}

juce::File userFolder()
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("KitForge Voices");
}

std::vector<Preset> user()
{
    std::vector<Preset> out;
    for (const auto& f : userFolder().findChildFiles (juce::File::findFiles, true, "*.kfvoice"))
    {
        Preset pr;
        if (auto xml = juce::parseXML (f); xml != nullptr && voiceFromXml (*xml, pr))
        {
            pr.source = "User";
            out.push_back (pr);
        }
    }
    std::sort (out.begin(), out.end(), [] (const Preset& a, const Preset& b) { return a.name.compareNatural (b.name) < 0; });
    return out;
}

std::vector<Preset> all()
{
    auto out = factory();
    for (auto& pr : fromKits()) out.push_back (pr);
    for (auto& pr : user()) out.push_back (pr);
    return out;
}

std::vector<Preset> forType (const std::vector<Preset>& list, VoiceType type)
{
    std::vector<Preset> out;
    for (auto& pr : list)
        if (pr.type == type)
            out.push_back (pr);
    return out;
}

std::unique_ptr<juce::XmlElement> voiceToXml (const juce::String& name, const VoiceDef& voice)
{
    auto xml = std::make_unique<juce::XmlElement> ("KitForgeVoice");
    xml->setAttribute ("name", name);
    xml->setAttribute ("version", 1);
    for (int vp = 0; vp < NumVP; ++vp)
        if (isKitParam (vp))
            xml->setAttribute (voiceSpec (vp).id, (double) voice[(size_t) vp]);
    return xml;
}

bool voiceFromXml (const juce::XmlElement& xml, Preset& out)
{
    if (! xml.hasTagName ("KitForgeVoice"))
        return false;
    out.name = xml.getStringAttribute ("name", "Untitled");
    out.voice = KitFactory::defaultVoice();
    for (int vp = 0; vp < NumVP; ++vp)
    {
        const auto& s = voiceSpec (vp);
        if (isKitParam (vp) && xml.hasAttribute (s.id))
            out.voice[(size_t) vp] = juce::jlimit (s.min, s.max, (float) xml.getDoubleAttribute (s.id));
    }
    out.type = (VoiceType) (int) out.voice[Type];
    return true;
}

bool saveUser (const juce::String& name, const VoiceDef& voice, juce::File* written)
{
    const auto dir = typeFolder ((VoiceType) (int) voice[Type]);
    if (! dir.createDirectory())
        return false;
    const auto file = dir.getChildFile (juce::File::createLegalFileName (name) + ".kfvoice");
    const bool ok = voiceToXml (name, voice)->writeTo (file);
    if (ok && written != nullptr)
        *written = file;
    return ok;
}
} // namespace kf::Presets
