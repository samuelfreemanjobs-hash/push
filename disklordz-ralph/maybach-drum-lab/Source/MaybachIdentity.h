#pragma once

#include <algorithm>
#include <cstdint>

#include "MaybachAcousticMorph.h"
#include "ThreeLayerDrum.h"

namespace dl::maybach {

// Luxury cinematic hip-hop *inspired* DNA — original synthesis only (not artist products).
enum class Identity : std::uint8_t {
    MaybachSoul = 0,
    CinematicEmpire,
    LuxuryTrap,
    YachtClub,
    TeflonDrums,
    RoyalOrchestra,
};

inline const char* identityName(Identity id) noexcept
{
    switch (id)
    {
    case Identity::MaybachSoul: return "Maybach Soul";
    case Identity::CinematicEmpire: return "Cinematic Empire";
    case Identity::LuxuryTrap: return "Luxury Trap";
    case Identity::YachtClub: return "Yacht Club";
    case Identity::TeflonDrums: return "Teflon Drums";
    case Identity::RoyalOrchestra: return "Royal Orchestra";
    }
    return "Maybach Soul";
}

enum class LuxuryKickPreset : std::uint8_t {
    MaybachKick,
    VelvetKick,
    SoulPunch,
    CinematicKick,
    ArenaKick,
};

inline Mood moodForIdentity(Identity id) noexcept
{
    switch (id)
    {
    case Identity::MaybachSoul: return Mood::Luxury;
    case Identity::CinematicEmpire: return Mood::Atmospheric;
    case Identity::LuxuryTrap: return Mood::Luxury;
    case Identity::YachtClub: return Mood::Atmospheric;
    case Identity::TeflonDrums: return Mood::Aggressive;
    case Identity::RoyalOrchestra: return Mood::Haunted;
    }
    return Mood::Luxury;
}

inline void applyIdentityLayering(DrumDesign& d, Identity id, Voice voice)
{
    switch (id)
    {
    case Identity::MaybachSoul:
        d.body.level = std::min(1.2f, d.body.level + 0.1f);
        d.attack.level *= 0.85f;
        d.texture.level *= 0.7f;
        d.original.amplitudeTauMs = std::min(5000.0, d.original.amplitudeTauMs * 1.2);
        d.original.brightness = std::max(0.0, d.original.brightness * 0.88);
        break;
    case Identity::CinematicEmpire:
        d.body.decayMs = std::min(5000.f, d.body.decayMs * 1.55f);
        d.texture.decayMs = std::min(5000.f, d.texture.decayMs * 1.8f);
        d.texture.level = std::min(1.0f, d.texture.level + 0.22f);
        d.attackToneHz = std::min(12000.f, d.attackToneHz * 0.72f);
        break;
    case Identity::LuxuryTrap:
        d.original.pitchTauMs = std::max(4.0, d.original.pitchTauMs * 0.9);
        d.attack.level = std::min(1.0f, d.attack.level + 0.08f);
        d.original.drive = std::clamp(d.original.drive * 1.05, 0.2, 10.0);
        break;
    case Identity::YachtClub:
        d.attack.level *= 0.68f;
        d.textureNoise = std::min(1.f, d.textureNoise * 0.75f);
        d.original.amplitudeTauMs = std::min(5000.0, d.original.amplitudeTauMs * 1.35);
        d.textureToneHz = std::min(12000.f, d.textureToneHz * 0.85f);
        break;
    case Identity::TeflonDrums:
        d.attack.level = std::min(1.0f, d.attack.level + 0.2f);
        d.original.drive = std::clamp(d.original.drive * 1.35, 0.25, 10.0);
        d.body.level = std::min(1.2f, d.body.level + 0.06f);
        break;
    case Identity::RoyalOrchestra:
        d.texture.level = std::min(1.0f, d.texture.level + 0.28f);
        d.texture.decayMs = std::min(5000.f, d.texture.decayMs * 2.1f);
        d.attack.decayMs = std::min(5000.f, d.attack.decayMs * 1.4f);
        d.textureToneHz = std::min(12000.f, d.textureToneHz * 1.25f);
        break;
    }

    if (voice == Voice::Kick)
    {
        switch (id)
        {
        case Identity::MaybachSoul:
            d.original.rootHz = 48.0;
            d.original.pitchStartHz = 95.0;
            break;
        case Identity::CinematicEmpire:
            d.original.rootHz = 42.0;
            d.original.amplitudeTauMs = std::min(5000.0, d.original.amplitudeTauMs * 1.6);
            break;
        case Identity::TeflonDrums:
            d.original.pitchStartHz = 125.0;
            d.original.pitchTauMs = std::max(3.0, d.original.pitchTauMs * 0.78);
            break;
        default:
            break;
        }
    }

    if (voice == Voice::Snare)
    {
        d.textureNoise = std::min(1.f, d.textureNoise + (id == Identity::RoyalOrchestra ? 0.12f : 0.04f));
    }

    if (voice == Voice::Clap && id == Identity::YachtClub)
        d.textureNoise = std::min(1.f, d.textureNoise * 0.6f);
}

inline DrumDesign designForMaybach(Voice voice,
                                 Identity identity,
                                 AcousticMorph morph = AcousticMorph::Hybrid,
                                 uint32_t seed = 3'026'0115u)
{
    auto d = designFor(voice, moodForIdentity(identity), seed);
    applyIdentityLayering(d, identity, voice);
    applyAcousticMorph(d, morph);
    d.seed = seed;
    return d;
}

inline DrumDesign luxuryKickForPreset(LuxuryKickPreset preset, Identity identity, uint32_t seed = 88u)
{
    auto d = designForMaybach(Voice::Kick, identity, AcousticMorph::Hybrid, seed);
    switch (preset)
    {
    case LuxuryKickPreset::MaybachKick:
        d.original.rootHz = 47.0;
        d.original.pitchStartHz = 98.0;
        applyAcousticMorph(d, AcousticMorph::Hybrid);
        break;
    case LuxuryKickPreset::VelvetKick:
        applyAcousticMorph(d, AcousticMorph::Live);
        d.original.amplitudeTauMs = std::min(5000.0, d.original.amplitudeTauMs * 1.4);
        break;
    case LuxuryKickPreset::SoulPunch:
        d.attack.level = std::min(1.0f, d.attack.level + 0.12f);
        break;
    case LuxuryKickPreset::CinematicKick:
        applyIdentityLayering(d, Identity::CinematicEmpire, Voice::Kick);
        break;
    case LuxuryKickPreset::ArenaKick:
        d.outputGain = std::min(0.95f, d.outputGain + 0.08f);
        applyAcousticMorph(d, AcousticMorph::Trap);
        break;
    }
    return d;
}

} // namespace dl::maybach