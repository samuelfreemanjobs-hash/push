#pragma once

#include <JuceHeader.h>

/**
 * Suite-tier multi-out: Kick / Sub808 / Hats on aux buses (DLZ-VST-009).
 * Wire in PluginProcessor::BusesProperties + per-voice bus assignment.
 */
struct TrapMultiOutLayout
{
    static constexpr int kickBus = 1;
    static constexpr int subBus = 2;
    static constexpr int hatsBus = 3;

    static juce::AudioProcessor::BusesProperties buses()
    {
        return BusesProperties()
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)
            .withOutput("Kick", juce::AudioChannelSet::stereo(), false)
            .withOutput("Sub808", juce::AudioChannelSet::stereo(), false)
            .withOutput("Hats", juce::AudioChannelSet::stereo(), false);
    }

    static bool isLayoutSupported(const BusesProperties& layouts)
    {
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;

        for (const auto& out : { kickBus, subBus, hatsBus })
        {
            const auto set = layouts.getChannelSet(false, out);
            if (!set.isDisabled() && set != juce::AudioChannelSet::stereo())
                return false;
        }
        return true;
    }

    /** Copy voice sample to main + optional bus (call from mixer, not alloc in processBlock). */
    static void addToBus(juce::AudioBuffer<float>& buffer, int busIndex, float l, float r,
                         int sampleIndex, bool busEnabled)
    {
        if (!busEnabled || busIndex < 0 || busIndex >= buffer.getNumChannels())
            return;
        buffer.addSample(0, sampleIndex, l);
        if (buffer.getNumChannels() > 1)
            buffer.addSample(1, sampleIndex, r);
    }
};