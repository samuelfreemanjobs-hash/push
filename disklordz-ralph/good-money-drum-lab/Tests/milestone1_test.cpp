#include "GoodMoneyIdentity.h"
#include "GoodMoneyKickForge.h"
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
    constexpr int identityCount = 8;
    const dl::Voice flagship[] = { dl::Voice::Kick, dl::Voice::Bass808, dl::Voice::Snare };

    for (int id = 0; id < identityCount; ++id)
    {
        const auto identity = static_cast<dl::goodmoney::Identity>(id);

        for (const auto voice : flagship)
        {
            for (const auto mode : { dl::goodmoney::PlayerMode::PlayerBounce,
                                     dl::goodmoney::PlayerMode::LuxurySoul })
            {
                const auto d = dl::goodmoney::designForGoodMoney(
                    voice, identity, mode, 4'061'115u + static_cast<uint32_t>(id) * 11u + static_cast<uint32_t>(voice));
                const auto a = dl::renderDesign(d), b = dl::renderDesign(d);
                check(a == b, "Good Money render must be deterministic");
                check(!a.empty(), "Good Money render must produce samples");

                double peak = 0.0;
                for (const auto x : a)
                {
                    check(std::isfinite(x), "samples finite");
                    peak = std::max(peak, std::abs(double(x)));
                }
                check(peak > 1.e-5, "Good Money render must be audible");
            }
        }
    }

    const auto gm = dl::goodmoney::goodMoneyKickForPreset(
        dl::goodmoney::GoodMoneyKickPreset::GoodMoneyKick, dl::goodmoney::Identity::GoodMoney, 1u);
    const auto esc = dl::goodmoney::goodMoneyKickForPreset(
        dl::goodmoney::GoodMoneyKickPreset::VintageSoulKick, dl::goodmoney::Identity::GreatEscape, 1u);
    check(dl::renderDesign(gm) != dl::renderDesign(esc), "Kick forge presets must differ");

    std::cout << "PASS Good Money M1: kick/808/snare x 8 DNA x Player/Luxury\n";
    return 0;
}