#pragma once

#include <cstdint>

#include "ThreeLayerDrum.h"

namespace dl::luger {

// Original DiskLordz genre recipes — morph modifiers on top of Cinematic Voice + Mood base.
enum class Identity : std::uint8_t {
    BrickSquad = 0,
    OrchestralWarfare,
    TrapAnthem,
    DarkSouthern,
    ArenaTrap,
};

inline const char* identityName(Identity id) noexcept
{
    switch (id)
    {
    case Identity::BrickSquad: return "Brick Squad";
    case Identity::OrchestralWarfare: return "Orchestral Warfare";
    case Identity::TrapAnthem: return "Trap Anthem";
    case Identity::DarkSouthern: return "Dark Southern";
    case Identity::ArenaTrap: return "Arena Trap";
    default: return "Brick Squad";
    }
}

// Luger Kick Cannon modes (M2 expands parameters; M1 uses recipe offsets).
enum class KickMode : std::uint8_t {
    BrickKick,
    ArenaKick,
    CannonKick,
    PunchKick,
    DarkKick,
};

inline DrumDesign designForLuger(Voice voice, Identity identity, uint32_t seed = 2'026'1115u)
{
  const auto baseMood = [](Identity id) {
    switch (id)
    {
    case Identity::BrickSquad: return Mood::Aggressive;
    case Identity::OrchestralWarfare: return Mood::Atmospheric;
    case Identity::TrapAnthem: return Mood::Luxury;
    case Identity::DarkSouthern: return Mood::Dark;
    case Identity::ArenaTrap: return Mood::Haunted;
    }
    return Mood::Aggressive;
  };

  DrumDesign d = designFor(voice, baseMood(identity), seed);

  switch (identity)
  {
  case Identity::BrickSquad:
    d.attack.level = std::min(1.0f, d.attack.level + 0.12f);
    d.body.level = std::min(1.2f, d.body.level + 0.06f);
    d.original.drive = std::min(10.0, d.original.drive * 1.15);
    d.original.pitchTauMs = std::max(1.0, d.original.pitchTauMs * 0.88);
    break;
  case Identity::OrchestralWarfare:
    d.texture.level = std::min(1.0f, d.texture.level + 0.18f);
    d.texture.decayMs = std::min(5000.f, d.texture.decayMs * 2.2f);
    d.original.amplitudeTauMs = std::min(5000.0, d.original.amplitudeTauMs * 1.55);
    d.attackToneHz = std::min(12000.f, d.attackToneHz * 0.75f);
    break;
  case Identity::TrapAnthem:
    d.original.drive = std::clamp(d.original.drive * 1.05, 0.25, 10.0);
    d.attack.level = std::min(1.0f, d.attack.level + 0.06f);
    d.outputGain = std::min(1.f, d.outputGain + 0.04f);
    break;
  case Identity::DarkSouthern:
    d.original.brightness = std::max(0.0, d.original.brightness * 0.72);
    d.texture.brightness = std::max(0.f, d.texture.brightness * 0.65f);
    d.original.drive = std::min(10.0, d.original.drive * 1.35);
    d.body.decayMs = std::min(5000.f, d.body.decayMs * 1.12f);
    break;
  case Identity::ArenaTrap:
    d.attack.level = std::min(1.0f, d.attack.level + 0.22f);
    d.attack.decayMs = std::min(5000.f, d.attack.decayMs * 1.35f);
    d.original.pitchStartHz = std::min(8000.0, d.original.pitchStartHz * 1.12);
    d.original.drive = std::min(10.0, d.original.drive * 1.45);
    break;
  }

  if (voice == Voice::Kick)
  {
    switch (identity)
    {
    case Identity::BrickSquad:
      d.original.rootHz = 52;
      d.original.pitchStartHz = 210;
      d.original.pitchTauMs = 14;
      d.original.amplitudeTauMs = 95;
      break;
    case Identity::OrchestralWarfare:
      d.original.rootHz = 48;
      d.original.amplitudeTauMs = 220;
      d.texture.decayMs = 280.f;
      break;
    case Identity::ArenaTrap:
      d.original.pitchStartHz = 280;
      d.original.pitchTauMs = 8;
      d.attack.level = std::min(1.f, d.attack.level + 0.1f);
      break;
    default:
      break;
    }
  }

  if (voice == Voice::Bass808)
  {
    switch (identity)
    {
    case Identity::DarkSouthern:
      d.original.rootHz = 44;
      d.original.amplitudeTauMs = 520;
      break;
    case Identity::BrickSquad:
      d.original.drive = std::min(10.0, d.original.drive * 1.2);
      d.original.pitchTauMs = 22;
      break;
    case Identity::ArenaTrap:
      d.original.amplitudeTauMs = 380;
      d.original.drive = std::min(10.0, d.original.drive * 1.55);
      break;
    default:
      break;
    }
  }

  return d;
}

inline DrumDesign kickForMode(KickMode mode, Identity identity, uint32_t seed)
{
  auto d = designForLuger(Voice::Kick, identity, seed);
  switch (mode)
  {
  case KickMode::BrickKick:
    d.original.pitchStartHz = 205;
    d.attack.level = std::min(1.f, d.attack.level + 0.08f);
    break;
  case KickMode::ArenaKick:
    d.original.pitchStartHz = 265;
    d.original.pitchTauMs = 7;
    break;
  case KickMode::CannonKick:
    d.original.amplitudeTauMs = 160;
    d.texture.decayMs = 200.f;
    break;
  case KickMode::PunchKick:
    d.original.pitchTauMs = 10;
    d.original.amplitudeTauMs = 72;
    break;
  case KickMode::DarkKick:
    d.original.rootHz = 46;
    d.original.brightness = 0.28;
    break;
  }
  return d;
}

} // namespace dl::luger