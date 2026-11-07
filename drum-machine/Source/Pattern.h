#pragma once

#include "Params.h"
#include <atomic>

namespace kf
{
// Lock-free storage for all patterns. Each step holds a velocity:
// 0 = off, 1..127 = on. Written by the GUI, read by the audio thread.
class PatternBank
{
public:
    PatternBank() { clearAll(); }

    uint8_t get (int pattern, int track, int step) const noexcept
    {
        return steps[pattern][track][step].load (std::memory_order_relaxed);
    }

    void set (int pattern, int track, int step, uint8_t velocity) noexcept
    {
        steps[pattern][track][step].store (velocity, std::memory_order_relaxed);
    }

    void clear (int pattern) noexcept
    {
        for (int t = 0; t < numTracks; ++t)
            for (int s = 0; s < numSteps; ++s)
                set (pattern, t, s, 0);
    }

    void clearAll() noexcept
    {
        for (int p = 0; p < numPatterns; ++p)
            clear (p);
    }

    void copy (int from, int to) noexcept
    {
        for (int t = 0; t < numTracks; ++t)
            for (int s = 0; s < numSteps; ++s)
                set (to, t, s, get (from, t, s));
    }

    // Serialise one track of one pattern as a hex string ("00647F...").
    juce::String trackToString (int pattern, int track) const
    {
        juce::String s;
        for (int i = 0; i < numSteps; ++i)
            s << juce::String::toHexString ((int) get (pattern, track, i)).paddedLeft ('0', 2);
        return s;
    }

    void trackFromString (int pattern, int track, const juce::String& s)
    {
        for (int i = 0; i < numSteps; ++i)
        {
            const auto hex = s.substring (i * 2, i * 2 + 2);
            set (pattern, track, i, (uint8_t) juce::jlimit (0, 127, hex.getHexValue32()));
        }
    }

private:
    std::atomic<uint8_t> steps[numPatterns][numTracks][numSteps];
};
} // namespace kf
