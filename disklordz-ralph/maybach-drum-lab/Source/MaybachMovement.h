#pragma once

#include "MaybachIdentity.h"

namespace dl::maybach {

// M5+ — luxury room / groove modulation (placeholder state for M1 compile).
struct MovementEngineState {
    float roomWet = 0.12f;
    float swing = 0.0f;
};

inline void applyDefaultMovementForIdentity(MovementEngineState& state, Identity id)
{
    switch (id)
    {
    case Identity::MaybachSoul:
        state.roomWet = 0.14f;
        state.swing = 0.04f;
        break;
    case Identity::CinematicEmpire:
        state.roomWet = 0.28f;
        break;
    case Identity::LuxuryTrap:
        state.roomWet = 0.08f;
        break;
    case Identity::YachtClub:
        state.roomWet = 0.18f;
        state.swing = 0.06f;
        break;
    case Identity::TeflonDrums:
        state.roomWet = 0.06f;
        break;
    case Identity::RoyalOrchestra:
        state.roomWet = 0.35f;
        break;
    }
}

} // namespace dl::maybach