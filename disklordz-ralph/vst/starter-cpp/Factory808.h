#pragma once

#include <JuceHeader.h>
#include <cmath>

/** Procedural 55 Hz decay hit for Lite pad 0 — replace with LouisianaKit assets before ship. */
namespace Factory808
{
inline juce::AudioBuffer<float> render(double sampleRate)
{
    const int len = static_cast<int>(sampleRate * 0.55);
    juce::AudioBuffer<float> buffer(1, len);
    for (int i = 0; i < len; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(sampleRate);
        const float env = std::exp(-3.5f * t);
        buffer.setSample(0, i, std::sin(juce::MathConstants<float>::twoPi * 55.0f * t) * env);
    }
    return buffer;
}
} // namespace Factory808