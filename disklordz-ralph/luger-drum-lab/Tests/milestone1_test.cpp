#include "LugerIdentity.h"
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
    constexpr int identityCount = 5;
    const dl::Voice flagship[] = { dl::Voice::Kick, dl::Voice::Snare, dl::Voice::Bass808 };

    for (int id = 0; id < identityCount; ++id)
    {
        const auto identity = static_cast<dl::luger::Identity>(id);
        for (const auto voice : flagship)
        {
            const auto d = dl::luger::designForLuger(voice, identity, 20261115u + id * 3u + static_cast<uint32_t>(voice));
            const auto a = dl::renderDesign(d), b = dl::renderDesign(d);
            check(a == b, "Luger render must be deterministic");
            check(!a.empty(), "Luger render must produce samples");

            double peak = 0.0;
            double rms = 0.0;
            for (const auto x : a)
            {
                check(std::isfinite(x), "samples finite");
                peak = std::max(peak, std::abs(double(x)));
                rms += double(x) * x;
            }
            check(peak > 1.e-5, "Luger render must be audible");
            check(rms > 1.e-8, "Luger render rms");
        }
    }

    const auto brickKick =
        dl::luger::kickForMode(dl::luger::KickMode::BrickKick, dl::luger::Identity::BrickSquad, 42u);
    const auto arenaKick =
        dl::luger::kickForMode(dl::luger::KickMode::ArenaKick, dl::luger::Identity::ArenaTrap, 42u);
    check(dl::renderDesign(brickKick) != dl::renderDesign(arenaKick), "Kick modes must differ");

    std::cout << "PASS Luger M1: kick/snare/808 x 5 identities deterministic\n";
    return 0;
}