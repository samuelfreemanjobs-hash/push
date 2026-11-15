#pragma once

#include <algorithm>
#include <cstdint>

#include "GoodMoneyPlayerMode.h"
#include "ThreeLayerDrum.h"

namespace dl::goodmoney {

// West Coast *inspired* DNA — original DiskLordz identities (not artist sample banks).
enum class Identity : std::uint8_t {
    GoodMoney = 0,
    MidnightPlayer,
    OrangeGrove,
    LuxurySoulBank,
    GreatEscape,
    BayAreaFunk,
    NightDrive,
    PremiumStreet,
};

inline const char* identityName(Identity id) noexcept
{
    switch (id)
    {
    case Identity::GoodMoney:
        return "Good Money";
    case Identity::MidnightPlayer:
        return "Midnight Player";
    case Identity::OrangeGrove:
        return "Orange Grove";
    case Identity::LuxurySoulBank:
        return "Luxury Soul";
    case Identity::GreatEscape:
        return "Great Escape";
    case Identity::BayAreaFunk:
        return "Bay Area Funk";
    case Identity::NightDrive:
        return "Night Drive";
    case Identity::PremiumStreet:
        return "Premium Street";
    }
    return "Good Money";
}

inline Mood moodForIdentity(Identity id) noexcept
{
    switch (id)
    {
    case Identity::GoodMoney:
    case Identity::MidnightPlayer:
    case Identity::PremiumStreet:
        return Mood::Luxury;
    case Identity::OrangeGrove:
    case Identity::NightDrive:
        return Mood::Atmospheric;
    case Identity::LuxurySoulBank:
    case Identity::GreatEscape:
        return Mood::Dark;
    case Identity::BayAreaFunk:
        return Mood::Aggressive;
    }
    return Mood::Luxury;
}

inline void applyIdentityLayering(DrumDesign& d, Identity id, Voice voice)
{
    switch (id)
    {
    case Identity::GoodMoney:
        d.original.rootHz = 50.0;
        d.original.pitchStartHz = 92.0;
        d.attack.level *= 0.88f;
        d.original.amplitudeTauMs = std::min(5000.0, d.original.amplitudeTauMs * 1.05);
        break;
    case Identity::MidnightPlayer:
        d.original.pitchStartHz = 105.0;
        d.original.pitchTauMs = std::max(5.0, d.original.pitchTauMs * 1.1);
        d.attack.level = std::min(1.0f, d.attack.level + 0.08f);
        break;
    case Identity::OrangeGrove:
        d.attack.level *= 0.65f;
        d.texture.level *= 0.6f;
        d.original.amplitudeTauMs = std::min(5000.0, d.original.amplitudeTauMs * 1.3);
        break;
    case Identity::LuxurySoulBank:
        d.textureNoise = std::min(1.f, d.textureNoise * 0.7f);
        d.body.decayMs = std::min(5000.f, d.body.decayMs * 1.4f);
        d.original.brightness = std::max(0.0, d.original.brightness * 0.8);
        break;
    case Identity::GreatEscape:
        d.attack.level *= 0.55f;
        d.texture.level *= 0.5f;
        d.body.level *= 0.85f;
        d.original.amplitudeTauMs = std::min(5000.0, d.original.amplitudeTauMs * 1.55);
        break;
    case Identity::BayAreaFunk:
        d.attack.level = std::min(1.0f, d.attack.level + 0.18f);
        d.original.drive = std::clamp(d.original.drive * 1.15, 0.2, 10.0);
        break;
    case Identity::NightDrive:
        d.original.rootHz = 44.0;
        d.texture.level *= 0.65f;
        d.attackToneHz = std::min(12000.f, d.attackToneHz * 0.78f);
        break;
    case Identity::PremiumStreet:
        d.attack.level = std::min(1.0f, d.attack.level + 0.15f);
        d.original.pitchTauMs = std::max(3.0, d.original.pitchTauMs * 0.78);
        d.body.level = std::min(1.2f, d.body.level + 0.1f);
        break;
    }

    if (voice == Voice::Bass808)
    {
        switch (id)
        {
        case Identity::MidnightPlayer:
            d.original.rootHz = 42.0;
            d.original.amplitudeTauMs = std::min(5000.0, d.original.amplitudeTauMs * 1.35);
            break;
        case Identity::NightDrive:
            d.original.rootHz = 40.0;
            d.original.drive = std::clamp(d.original.drive * 0.75, 0.1, 10.0);
            break;
        default:
            break;
        }
    }

    if (voice == Voice::Snare)
    {
        if (id == Identity::LuxurySoulBank || id == Identity::GreatEscape)
            d.textureNoise = std::min(1.f, d.textureNoise + 0.08f);
        if (id == Identity::PremiumStreet)
            d.attack.level = std::min(1.0f, d.attack.level + 0.12f);
    }
}

inline DrumDesign designForGoodMoney(Voice voice,
                                     Identity identity,
                                     PlayerMode mode = PlayerMode::PlayerBounce,
                                     uint32_t seed = 4'061'115u)
{
    auto d = designFor(voice, moodForIdentity(identity), seed);
    applyIdentityLayering(d, identity, voice);
    applyPlayerMode(d, mode);
    d.seed = seed;
    return d;
}

} // namespace dl::goodmoney