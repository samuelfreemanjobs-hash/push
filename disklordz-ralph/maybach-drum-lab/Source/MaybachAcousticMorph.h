#pragma once

#include "ThreeLayerDrum.h"

namespace dl::maybach {

/** Signature MAYBACH axis: modeled warmth ↔ electronic punch (M1 — parameter blend only). */
enum class AcousticMorph : std::uint8_t { Live = 0, Hybrid, Trap };

inline const char* morphName(AcousticMorph m) noexcept
{
    switch (m)
    {
    case AcousticMorph::Live: return "Live";
    case AcousticMorph::Hybrid: return "Hybrid";
    case AcousticMorph::Trap: return "Trap";
    }
    return "Hybrid";
}

inline void applyAcousticMorph(DrumDesign& d, AcousticMorph morph)
{
    switch (morph)
    {
    case AcousticMorph::Live:
        d.attack.level *= 0.72f;
        d.texture.level *= 0.78f;
        d.body.level = std::min(1.25f, d.body.level + 0.12f);
        d.body.decayMs = std::min(5000.f, d.body.decayMs * 1.35f);
        d.original.drive = std::clamp(d.original.drive * 0.88, 0.15, 10.0);
        d.original.brightness = std::max(0.0, d.original.brightness * 0.82);
        break;
    case AcousticMorph::Hybrid:
        break;
    case AcousticMorph::Trap:
        d.attack.level = std::min(1.0f, d.attack.level + 0.14f);
        d.original.pitchTauMs = std::max(3.0, d.original.pitchTauMs * 0.82);
        d.original.drive = std::clamp(d.original.drive * 1.12, 0.2, 10.0);
        d.textureNoise = std::min(1.f, d.textureNoise + 0.06f);
        break;
    }
}

} // namespace dl::maybach