#pragma once

#include <JuceHeader.h>

/**
 * 2x oversampled soft clip (Fruity-style cubic + tanh ceiling).
 * Construct once; call prepare() in prepareToPlay; process() in processBlock on kick/cowbell bus.
 */
class TrapSoftClipChain
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec, size_t maxBlock)
    {
        oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
            2, 2,
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
        oversampler->initProcessing(maxBlock);

        softClipper.functionToUse = [](float x) {
            x = juce::jlimit(-1.5f, 1.5f, x);
            return std::tanh(x - (0.15f * x * x * x));
        };
        softClipper.prepare(spec);
    }

    void setDrive(float newDrive) noexcept { drive.store(newDrive, std::memory_order_relaxed); }

    void process(juce::AudioBuffer<float>& buffer)
    {
        const float d = drive.load(std::memory_order_relaxed);
        juce::dsp::AudioBlock<float> block(buffer);
        auto up = oversampler->processSamplesUp(block);

        for (size_t ch = 0; ch < up.getNumChannels(); ++ch)
        {
            auto* data = up.getChannelPointer(ch);
            for (size_t i = 0; i < up.getNumSamples(); ++i)
                data[i] = softClipper.processSample(data[i] * d);
        }

        oversampler->processSamplesDown(block);
    }

private:
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    juce::dsp::WaveShaper<float> softClipper;
    std::atomic<float> drive { 2.5f };
};