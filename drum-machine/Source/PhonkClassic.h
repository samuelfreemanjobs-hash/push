#pragma once

#include "Pattern.h"
#include "Params.h"
#include <cstdint>
#include <vector>

namespace kf::PhonkClassic
{
// Memphis pattern generator ported from Phonk Factory (splitmix64, classic groove at variation 0).
constexpr int kVariations = 4;
enum class Voice : int8_t { Kick = 0, Snare, ClosedHat, OpenHat, Cowbell, NumVoices };

struct Settings
{
    int seed = 1;
    int bars = 1;              // 1 or 2 (2-bar loops for export)
    float variation = 0.0f;  // 0 = original Memphis script groove, 1 = heavily mutated
    float rolls = 0.3f;
    float cowbell = 0.2f;
    float humanize = 0.15f;
};

struct Event
{
    int step = 0;              // 0..(bars*16 - 1)
    Voice voice = Voice::Kick;
    int variation = 0;           // 0..3
    float micro = 0.f;         // fractional step offset (rolls)
    float velocity = 1.f;
};

struct Rng
{
    explicit Rng (uint64_t seed = 1) : s (seed) {}
    uint64_t next();
    double uniform();
    bool chance (double p);
    int range (int n);
    uint64_t s;
};

std::vector<Event> generateEvents (const Settings& ps);
int trackForVoice (juce::AudioProcessorValueTreeState& apvts, Voice voice, int variation);

void applyToPattern (PatternBank& bank, int pattern, juce::AudioProcessorValueTreeState& apvts,
                     const std::vector<Event>& events, int maxSteps = numSteps);
} // namespace kf::PhonkClassic