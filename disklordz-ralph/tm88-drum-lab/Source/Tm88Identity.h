#pragma once

#include <cstdint>

#include "ThreeLayerDrum.h"

namespace dl::tm88 {

// Dark melodic / space trap *inspired* recipes — original synthesis only (not TM88 products).
enum class Identity : std::uint8_t {
    Nightmare88 = 0,
    EmoTrap,
    SpaceMafia,
    FiftySixNights,
    CrashDummy,
    AfterHours,
};

inline const char* identityName(Identity id) noexcept
{
    switch (id)
    {
    case Identity::Nightmare88: return "Nightmare 88";
    case Identity::EmoTrap: return "Emo Trap";
    case Identity::SpaceMafia: return "Space Mafia";
    case Identity::FiftySixNights: return "56 Nights";
    case Identity::CrashDummy: return "Crash Dummy";
    case Identity::AfterHours: return "After Hours";
    default: return "Nightmare 88";
    }
}

enum class Bass808Preset : std::uint8_t {
    Nightmare808,
    GhostSlide808,
    SpaceBass808,
    DeepEmotion808,
    Mafia808,
};

inline DrumDesign designForTm88(Voice voice, Identity identity, uint32_t seed = 2'026'1115u)
{
    const auto baseMood = [](Identity id) {
        switch (id)
        {
        case Identity::Nightmare88: return Mood::Haunted;
        case Identity::EmoTrap: return Mood::Atmospheric;
        case Identity::SpaceMafia: return Mood::Luxury;
        case Identity::FiftySixNights: return Mood::Aggressive;
        case Identity::CrashDummy: return Mood::Aggressive;
        case Identity::AfterHours: return Mood::Dark;
        }
        return Mood::Dark;
    };

    DrumDesign d = designFor(voice, baseMood(identity), seed);

    switch (identity)
    {
    case Identity::Nightmare88:
        d.original.brightness = std::max(0.0, d.original.brightness * 0.68);
        d.texture.level = std::min(1.0f, d.texture.level + 0.14f);
        d.texture.decayMs = std::min(5000.f, d.texture.decayMs * 1.4f);
        d.attackToneHz = std::min(12000.f, d.attackToneHz * 0.82f);
        break;
    case Identity::EmoTrap:
        d.original.pitchTauMs = std::min(120.0, d.original.pitchTauMs * 1.35);
        d.original.amplitudeTauMs = std::min(5000.0, d.original.amplitudeTauMs * 1.25);
        d.attack.level = std::min(1.0f, d.attack.level + 0.04f);
        d.textureNoise = std::min(1.f, d.textureNoise * 0.85f);
        break;
    case Identity::SpaceMafia:
        d.texture.level = std::min(1.0f, d.texture.level + 0.2f);
        d.textureToneHz = std::min(12000.f, d.textureToneHz * 1.35f);
        d.original.drive = std::clamp(d.original.drive * 1.08, 0.2, 10.0);
        d.textureNoise = std::min(1.f, d.textureNoise + 0.08f);
        break;
    case Identity::FiftySixNights:
        d.attack.level = std::min(1.0f, d.attack.level + 0.18f);
        d.original.pitchTauMs = std::max(4.0, d.original.pitchTauMs * 0.75);
        d.original.amplitudeTauMs = std::min(5000.0, d.original.amplitudeTauMs * 0.82);
        break;
    case Identity::CrashDummy:
        d.original.drive = std::min(10.0, d.original.drive * 1.65);
        d.attack.level = std::min(1.0f, d.attack.level + 0.26f);
        d.textureNoise = std::min(1.f, d.textureNoise + 0.15f);
        break;
    case Identity::AfterHours:
        d.body.level = std::min(1.2f, d.body.level + 0.08f);
        d.attack.level *= 0.72f;
        d.texture.level *= 0.65f;
        d.original.amplitudeTauMs = std::min(5000.0, d.original.amplitudeTauMs * 1.45);
        d.original.brightness = std::max(0.0, d.original.brightness * 0.78);
        break;
    }

    if (voice == Voice::Kick)
    {
        switch (identity)
        {
        case Identity::Nightmare88:
            d.original.rootHz = 50;
            d.original.pitchStartHz = 198;
            d.original.pitchTauMs = 16;
            break;
        case Identity::EmoTrap:
            d.original.rootHz = 47;
            d.original.pitchTauMs = 22;
            break;
        case Identity::FiftySixNights:
            d.original.pitchStartHz = 240;
            d.original.pitchTauMs = 9;
            d.original.amplitudeTauMs = 88;
            break;
        case Identity::AfterHours:
            d.original.rootHz = 44;
            d.original.amplitudeTauMs = 130;
            break;
        default:
            break;
        }
    }

    if (voice == Voice::Snare)
    {
        switch (identity)
        {
        case Identity::Nightmare88:
            d.texture.level = std::min(1.0f, d.texture.level + 0.1f);
            d.original.brightness = std::max(0.0, d.original.brightness * 0.85);
            break;
        case Identity::EmoTrap:
            d.attack.level = std::min(1.0f, d.attack.level + 0.06f);
            d.texture.decayMs = std::min(5000.f, d.texture.decayMs * 1.15f);
            break;
        case Identity::SpaceMafia:
            d.textureToneHz = std::min(12000.f, d.textureToneHz * 1.2f);
            break;
        default:
            break;
        }
    }

    if (voice == Voice::Bass808)
    {
        switch (identity)
        {
        case Identity::EmoTrap:
            d.original.rootHz = 42;
            d.original.pitchTauMs = 38;
            d.original.amplitudeTauMs = 680;
            d.original.drive = std::clamp(d.original.drive * 0.92, 0.2, 10.0);
            break;
        case Identity::SpaceMafia:
            d.original.pitchTauMs = 28;
            d.texture.level = std::min(1.0f, d.texture.level + 0.12f);
            d.original.drive = std::min(10.0, d.original.drive * 1.22);
            break;
        case Identity::Nightmare88:
            d.original.rootHz = 40;
            d.original.amplitudeTauMs = 720;
            d.original.brightness = std::max(0.0, d.original.brightness * 0.7);
            break;
        case Identity::FiftySixNights:
            d.original.drive = std::min(10.0, d.original.drive * 1.4);
            d.original.pitchTauMs = 18;
            break;
        default:
            break;
        }
    }

    return d;
}

inline DrumDesign bass808ForPreset(Bass808Preset preset, Identity identity, uint32_t seed)
{
    auto d = designForTm88(Voice::Bass808, identity, seed);
    switch (preset)
    {
    case Bass808Preset::Nightmare808:
        d.original.rootHz = 39;
        d.original.amplitudeTauMs = 780;
        break;
    case Bass808Preset::GhostSlide808:
        d.original.pitchTauMs = 52;
        d.original.amplitudeTauMs = 920;
        d.texture.level *= 0.5f;
        break;
    case Bass808Preset::SpaceBass808:
        d.texture.level = std::min(1.f, d.texture.level + 0.15f);
        d.original.drive = std::min(10.0, d.original.drive * 1.3);
        break;
    case Bass808Preset::DeepEmotion808:
        d.original.rootHz = 41;
        d.original.pitchTauMs = 44;
        d.original.drive *= 0.85;
        break;
    case Bass808Preset::Mafia808:
        d.original.pitchTauMs = 20;
        d.original.drive = std::min(10.0, d.original.drive * 1.55);
        break;
    }
    return d;
}

} // namespace dl::tm88