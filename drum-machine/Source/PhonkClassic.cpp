#include "PhonkClassic.h"

#include <algorithm>
#include <cmath>

namespace kf::PhonkClassic
{
uint64_t Rng::next()
{
    uint64_t z = (s += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

double Rng::uniform() { return (next() >> 11) * (1.0 / 9007199254740992.0); }

bool Rng::chance (double p) { return uniform() < p; }

int Rng::range (int n) { return n <= 0 ? 0 : (int) (uniform() * n); }

int trackForVoice (juce::AudioProcessorValueTreeState& apvts, Voice voice, int variation)
{
    static const VoiceType map[] = { VoiceType::Kick, VoiceType::Snare, VoiceType::ClosedHat, VoiceType::OpenHat, VoiceType::Cowbell };
    const auto want = map[(int) voice];
    std::vector<int> tracks;
    for (int t = 0; t < numTracks; ++t)
        if ((VoiceType) (int) getParamValue (apvts, voiceParamID (t, Type)) == want)
            tracks.push_back (t);
    if (tracks.empty())
        return -1;
    const int v = juce::jlimit (0, kVariations - 1, variation);
    return tracks[(size_t) (v % (int) tracks.size())];
}

std::vector<Event> generateEvents (const Settings& ps)
{
    std::vector<Event> out;
    Rng r ((uint64_t) ps.seed * 7919ULL + 13ULL);
    const int kv = r.range (kVariations), sv = r.range (kVariations), hv = r.range (kVariations),
              ov = r.range (kVariations), cv = r.range (kVariations);

    const int bars = juce::jlimit (1, 2, ps.bars);
    const auto add = [&] (int bar, int step, Voice voice, int var, double vel, double micro = 0.0)
    {
        const double h = 1.0 + (r.uniform() - 0.5) * (double) ps.humanize * 0.5;
        Event e;
        e.step = bar * 16 + step;
        e.voice = voice;
        e.variation = var;
        e.micro = (float) micro;
        e.velocity = (float) std::clamp (vel * h, 0.05, 1.0);
        out.push_back (e);
    };

    static const int kickBase[] = { 0, 3, 7, 10, 14 };
    static const int snareBase[] = { 4, 12 };
    static const int ohBase[] = { 2, 10 };

    for (int bar = 0; bar < bars; ++bar)
    {
        const double pm = (double) ps.variation * (bar == 0 ? 0.4 : 1.0);
        std::array<bool, 16> kick {}, snare {}, oh {}, hatOff {};
        std::array<float, 16> snareVel {};
        for (int s : kickBase) kick[(size_t) s] = true;
        for (int s : snareBase) { snare[(size_t) s] = true; snareVel[(size_t) s] = 0.90f; }
        for (int s : ohBase) oh[(size_t) s] = true;

        for (int s = 1; s < 16; ++s)
        {
            if (kick[(size_t) s] && r.chance (pm * 0.30)) kick[(size_t) s] = false;
            else if (! kick[(size_t) s] && s != 4 && s != 12 && r.chance (pm * 0.16)) kick[(size_t) s] = true;
        }
        if (r.chance (pm * 0.4))
        {
            const int g = r.chance (0.5) ? 15 : 13;
            snare[(size_t) g] = true;
            snareVel[(size_t) g] = 0.32f;
        }
        if (r.chance (pm * 0.5))
        {
            static const int ohMoves[] = { 6, 14, 15 };
            const int from = ohBase[r.range (2)], to = ohMoves[r.range (3)];
            oh[(size_t) from] = false;
            oh[(size_t) to] = true;
        }
        for (int s = 0; s < 16; ++s)
            if (r.chance (pm * 0.10)) hatOff[(size_t) s] = true;

        int rollStep = -1;
        bool triplet = true;
        if (r.chance ((double) ps.rolls * (bar % 2 ? 1.0 : 0.6)))
        {
            rollStep = r.chance (0.5) ? 6 : 14;
            triplet = r.chance (0.65);
        }
        if (rollStep >= 0)
        {
            oh[(size_t) rollStep] = oh[(size_t) rollStep + 1] = false;
            hatOff[(size_t) rollStep] = hatOff[(size_t) rollStep + 1] = true;
        }

        for (int s = 0; s < 16; ++s)
        {
            if (kick[(size_t) s]) add (bar, s, Voice::Kick, kv, 0.95);
            if (snare[(size_t) s]) add (bar, s, Voice::Snare, sv, snareVel[(size_t) s]);
            if (oh[(size_t) s]) add (bar, s, Voice::OpenHat, ov, 0.55);
            else if (! hatOff[(size_t) s]) add (bar, s, Voice::ClosedHat, hv, (s % 2 == 0) ? 0.55 : 0.35);
        }
        if (rollStep >= 0)
        {
            if (triplet)
            {
                add (bar, rollStep, Voice::ClosedHat, hv, 0.40);
                add (bar, rollStep, Voice::ClosedHat, hv, 0.50, 2.0 / 3.0);
                add (bar, rollStep, Voice::ClosedHat, hv, 0.65, 4.0 / 3.0);
            }
            else
            {
                add (bar, rollStep, Voice::ClosedHat, hv, 0.50);
                add (bar, rollStep, Voice::ClosedHat, hv, 0.40, 0.5);
                add (bar, rollStep + 1, Voice::ClosedHat, hv, 0.50);
                add (bar, rollStep + 1, Voice::ClosedHat, hv, 0.45, 0.5);
            }
        }
        static const int cowSteps[] = { 3, 6, 11, 14 };
        for (int s : cowSteps)
            if (r.chance ((double) ps.cowbell * 0.5 * (bar == 0 ? 0.6 : 1.0)))
                add (bar, s, Voice::Cowbell, cv, 0.60);
    }
    return out;
}

void applyToPattern (PatternBank& bank, int pattern, juce::AudioProcessorValueTreeState& apvts,
                     const std::vector<Event>& events, int maxSteps)
{
    bank.clear (pattern);
    for (const auto& e : events)
    {
        const int step = e.step % maxSteps;
        const int track = trackForVoice (apvts, e.voice, e.variation);
        if (track < 0)
            continue;

        const auto type = (VoiceType) (int) getParamValue (apvts, voiceParamID (track, Type));
        const uint8_t vel = (uint8_t) juce::jlimit (1, 127, (int) std::lround (e.velocity * 127.0f));
        bank.set (pattern, track, step, vel);

        if (type == VoiceType::ClosedHat && e.micro > 0.05f)
        {
            if (std::abs (e.micro - 2.0f / 3.0f) < 0.05f || std::abs (e.micro - 4.0f / 3.0f) < 0.05f)
                bank.setRatchet (pattern, track, step, 3);
            else if (e.micro >= 0.45f)
                bank.setRatchet (pattern, track, step, 2);
        }

        if (type == VoiceType::OpenHat)
        {
            for (int t = 0; t < numTracks; ++t)
                if ((VoiceType) (int) getParamValue (apvts, voiceParamID (t, Type)) == VoiceType::ClosedHat)
                    bank.set (pattern, t, step, 0);
        }
    }
}
} // namespace kf::PhonkClassic