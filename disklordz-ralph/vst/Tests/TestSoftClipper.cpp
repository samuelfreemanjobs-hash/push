#include <cmath>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "SoftClipper.h"

TEST_CASE("Soft clipper bounds extreme input", "[dsp][clip]")
{
    const float out = SoftClipper::processSample(10.0f, 2.5f);
    REQUIRE(out <= 1.0f);
    REQUIRE(out >= -1.0f);
    REQUIRE(std::isfinite(out));
}

TEST_CASE("Soft clipper is identity near zero", "[dsp][clip]")
{
    const float out = SoftClipper::processSample(0.0f, 2.5f);
    REQUIRE(out == Catch::Approx(0.0f).margin(0.001f));
}