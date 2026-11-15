#pragma once

#include <JuceHeader.h>
#include <cmath>

/** RT-safe scalar soft clip (Fruity-style cubic + tanh). Oversampling lives in plugin processor. */
class SoftClipper
{
public:
    static float processSample(float x, float drive) noexcept
    {
        const float y = juce::jlimit(-1.5f, 1.5f, x * drive);
        return std::tanh(y - (0.15f * y * y * y));
    }
};