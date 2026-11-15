#pragma once

#include "GoodMoneyIdentity.h"

namespace dl::goodmoney {

enum class GoodMoneyKickPreset : std::uint8_t {
    GoodMoneyKick,
    PlayerKnock,
    BayAreaThump,
    CleanTrunkKick,
    MidnightKick,
    VintageSoulKick,
    DustyPocketKick,
    LuxuryStreetKick,
};

inline DrumDesign goodMoneyKickForPreset(GoodMoneyKickPreset preset, Identity identity, uint32_t seed = 48u)
{
    auto d = designForGoodMoney(Voice::Kick, identity, PlayerMode::PlayerBounce, seed);
    switch (preset)
    {
    case GoodMoneyKickPreset::GoodMoneyKick:
        d.original.rootHz = 48.0;
        d.original.pitchStartHz = 94.0;
        break;
    case GoodMoneyKickPreset::PlayerKnock:
        d.attack.level = std::min(1.0f, d.attack.level + 0.14f);
        d.original.pitchTauMs = std::max(4.0, d.original.pitchTauMs * 0.85);
        break;
    case GoodMoneyKickPreset::BayAreaThump:
        d.original.rootHz = 46.0;
        d.body.level = std::min(1.25f, d.body.level + 0.15f);
        break;
    case GoodMoneyKickPreset::CleanTrunkKick:
        applyPlayerMode(d, PlayerMode::LuxurySoul);
        d.original.drive = std::clamp(d.original.drive * 0.7, 0.1, 10.0);
        break;
    case GoodMoneyKickPreset::MidnightKick:
        applyIdentityLayering(d, Identity::MidnightPlayer, Voice::Kick);
        break;
    case GoodMoneyKickPreset::VintageSoulKick:
        applyPlayerMode(d, PlayerMode::LuxurySoul);
        d.textureNoise = std::min(1.f, d.textureNoise + 0.05f);
        break;
    case GoodMoneyKickPreset::DustyPocketKick:
        d.attack.level *= 0.6f;
        d.original.brightness *= 0.75;
        break;
    case GoodMoneyKickPreset::LuxuryStreetKick:
        applyIdentityLayering(d, Identity::PremiumStreet, Voice::Kick);
        break;
    }
    return d;
}

} // namespace dl::goodmoney