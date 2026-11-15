#include "Tm88Identity.h"
#include "Tm88Movement.h"
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
    const dl::Voice flagship[] = { dl::Voice::Kick, dl::Voice::Snare, dl::Voice::Bass808 };

    for (int id = 0; id < identityCount; ++id)
    {
        const auto identity = static_cast<dl::tm88::Identity>(id);
        dl::tm88::MovementEngineState move;
        dl::tm88::applyDefaultMovementForIdentity(move, identity);

        for (const auto voice : flagship)
        {
            const auto d = dl::tm88::designForTm88(voice, identity, 20261115u + id * 5u + static_cast<uint32_t>(voice));
            const auto a = dl::renderDesign(d), b = dl::renderDesign(d);
            check(a == b, "TM88 render must be deterministic");
            check(!a.empty(), "TM88 render must produce samples");

            double peak = 0.0;
            double rms = 0.0;
            for (const auto x : a)
            {
                check(std::isfinite(x), "samples finite");
                peak = std::max(peak, std::abs(double(x)));
                rms += double(x) * x;
            }
            check(peak > 1.e-5, "TM88 render must be audible");
            check(rms > 1.e-8, "TM88 render rms");
        }
    }

    const auto emo =
        dl::tm88::bass808ForPreset(dl::tm88::Bass808Preset::GhostSlide808, dl::tm88::Identity::EmoTrap, 42u);
    const auto mafia =
        dl::tm88::bass808ForPreset(dl::tm88::Bass808Preset::Mafia808, dl::tm88::Identity::FiftySixNights, 42u);
    check(dl::renderDesign(emo) != dl::renderDesign(mafia), "808 presets must differ");

    std::cout << "PASS TM88 M1: kick/snare/808 x 6 identities deterministic\n";
    return 0;
}