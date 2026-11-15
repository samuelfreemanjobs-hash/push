#pragma once

#include "ThreeLayerDrum.h"

namespace dl::goodmoney {

/** Primary product axis: warm funky bounce vs dusty minimalist soul. */
enum class PlayerMode : std::uint8_t { PlayerBounce = 0, LuxurySoul };

inline const char* playerModeName(PlayerMode m) noexcept
{
    switch (m)
    {
    case PlayerMode::PlayerBounce:
        return "Player Bounce";
    case PlayerMode::LuxurySoul:
        return "Luxury Soul";
    }
    return "Player Bounce";
}

inline void applyPlayerMode(DrumDesign& d, PlayerMode mode)
{
    switch (mode)
    {
    case PlayerMode::PlayerBounce:
        d.attack.level = std::min(1.0f, d.attack.level + 0.1f);
        d.original.pitchTauMs = std::max(4.0, d.original.pitchTauMs * 0.88);
        d.body.level = std::min(1.2f, d.body.level + 0.06f);
        d.original.drive = std::clamp(d.original.drive * 1.05, 0.15, 10.0);
        break;
    case PlayerMode::LuxurySoul:
        d.attack.level *= 0.7f;
        d.texture.level *= 0.72f;
        d.body.level = std::min(1.25f, d.body.level + 0.08f);
        d.body.decayMs = std::min(5000.f, d.body.decayMs * 1.25f);
        d.textureNoise = std::min(1.f, d.textureNoise * 0.78f);
        d.original.drive = std::clamp(d.original.drive * 0.82, 0.12, 10.0);
        d.original.brightness = std::max(0.0, d.original.brightness * 0.86);
        break;
    }
}

} // namespace dl::goodmoney