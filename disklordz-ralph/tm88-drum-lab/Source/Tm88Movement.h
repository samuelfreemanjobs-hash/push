#pragma once

#include <cstdint>

#include "Tm88Identity.h"

namespace dl::tm88 {

/** M5 — 808 Movement & Percussion Intelligence (signature vs South/Luger labs). */
enum class RhythmicDensity : std::uint8_t {
    Sparse = 0,
    Balanced,
    Energetic,
    Complex,
};

enum class PercussionProminence : std::uint8_t {
    Support = 0,
    Balanced,
    Featured,
};

struct MovementEngineState
{
    RhythmicDensity densityTarget = RhythmicDensity::Balanced;
    PercussionProminence percussion = PercussionProminence::Balanced;
    float glideSeconds = 0.12f;
    bool legatoGlide = true;
};

// M1: state holder only — MIDI groove engine lands in M5.
inline void applyDefaultMovementForIdentity(MovementEngineState& s, Identity identity)
{
    switch (identity)
    {
    case Identity::AfterHours:
        s.densityTarget = RhythmicDensity::Sparse;
        s.percussion = PercussionProminence::Support;
        s.glideSeconds = 0.18f;
        break;
    case Identity::FiftySixNights:
        s.densityTarget = RhythmicDensity::Energetic;
        s.glideSeconds = 0.06f;
        break;
    case Identity::EmoTrap:
        s.densityTarget = RhythmicDensity::Balanced;
        s.glideSeconds = 0.22f;
        s.legatoGlide = true;
        break;
    case Identity::SpaceMafia:
        s.percussion = PercussionProminence::Featured;
        s.glideSeconds = 0.14f;
        break;
    default:
        break;
    }
}

} // namespace dl::tm88