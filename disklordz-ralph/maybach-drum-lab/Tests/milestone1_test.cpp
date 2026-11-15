#include "MaybachIdentity.h"
#include "MaybachMovement.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

static void check(bool v, const char* msg)
{
    if (!v)
        throw std::runtime_error(msg);
}

int main()
{
    constexpr int identityCount = 6;
    const dl::Voice flagship[] = { dl::Voice::Kick, dl::Voice::Snare, dl::Voice::Clap };

    for (int id = 0; id < identityCount; ++id)
    {
        const auto identity = static_cast<dl::maybach::Identity>(id);
        dl::maybach::MovementEngineState move;
        dl::maybach::applyDefaultMovementForIdentity(move, identity);

        for (const auto voice : flagship)
        {
            for (const auto morph : { dl::maybach::AcousticMorph::Live,
                                      dl::maybach::AcousticMorph::Hybrid,
                                      dl::maybach::AcousticMorph::Trap })
            {
                const auto d = dl::maybach::designForMaybach(voice, identity, morph, 30260115u + id * 7u + static_cast<uint32_t>(voice));
                const auto a = dl::renderDesign(d), b = dl::renderDesign(d);
                check(a == b, "Maybach render must be deterministic");
                check(!a.empty(), "Maybach render must produce samples");

                double peak = 0.0;
                for (const auto x : a)
                {
                    check(std::isfinite(x), "samples finite");
                    peak = std::max(peak, std::abs(double(x)));
                }
                check(peak > 1.e-5, "Maybach render must be audible");
            }
        }
    }

    const auto velvet = dl::maybach::luxuryKickForPreset(
        dl::maybach::LuxuryKickPreset::VelvetKick, dl::maybach::Identity::MaybachSoul, 42u);
    const auto arena = dl::maybach::luxuryKickForPreset(
        dl::maybach::LuxuryKickPreset::ArenaKick, dl::maybach::Identity::TeflonDrums, 42u);
    check(dl::renderDesign(velvet) != dl::renderDesign(arena), "Luxury kick presets must differ");

    std::cout << "PASS Maybach M1: kick/snare/clap x 6 identities x Live/Hybrid/Trap\n";
    return 0;
}