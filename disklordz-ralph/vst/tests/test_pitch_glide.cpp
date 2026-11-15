#include <cmath>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

// Minimal math contract test before wiring full JUCE voice in CI.
// Full render tests run in DetroitPhonkTrapVST3 workspace with JUCE linked.

TEST_CASE("808 pitch ratio targets semitone spacing", "[glide][trap-core]")
{
    const int root = 36;
    const int note = 48;
    const double ratio =
        std::pow(2.0, (note - root) / 12.0);
    REQUIRE_THAT(ratio, Catch::Matchers::WithinRel(4.0, 0.001));
}

TEST_CASE("Soft clip bounds output", "[clip][trap-core]")
{
    auto clip = [](float x) {
        x = std::min(1.5f, std::max(-1.5f, x));
        return std::tanh(x - (0.15f * x * x * x));
    };
    REQUIRE(clip(10.0f) <= 1.0f);
    REQUIRE(clip(-10.0f) >= -1.0f);
    REQUIRE(std::isfinite(clip(0.0f)));
}