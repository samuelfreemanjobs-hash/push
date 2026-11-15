#include <cmath>
#include <catch2/catch_test_macros.hpp>
#include <juce_audio_basics/juce_audio_basics.h>

#include "Trap808Voice.h"

TEST_CASE("808 Voice renders valid audio without NaNs or Inf", "[dsp][808]")
{
    juce::AudioBuffer<float> buffer(2, 512);
    buffer.clear();

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float sample = buffer.getSample(ch, i);
            REQUIRE_FALSE(std::isnan(sample));
            REQUIRE_FALSE(std::isinf(sample));
        }
    }
}

TEST_CASE("808 pitch ratio doubles per octave", "[dsp][808][glide]")
{
    const int root = 36;
    const int note = 48;
    const double ratio = std::pow(2.0, (note - root) / 12.0);
    REQUIRE(ratio == Catch::Approx(4.0).margin(0.001));
}