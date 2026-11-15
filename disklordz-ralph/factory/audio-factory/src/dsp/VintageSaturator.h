#pragma once

#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <cmath>

class VintageSaturator
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        juce::dsp::ProcessSpec filterSpec{ spec.sampleRate, spec.maximumBlockSize, spec.numChannels };

        lpf.prepare(filterSpec);
        lpf.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        lpf.setCutoffFrequency(14500.0f);
        lpf.setResonance(0.707f);
    }

    void reset() { lpf.reset(); }

    void setDrive(float newDrive) noexcept { drive = juce::jlimit(0.0f, 1.0f, newDrive); }

    void process(juce::AudioBuffer<float>& buffer)
    {
        const float preGain = 1.0f + drive * 6.5f;
        const float mix = 0.25f + drive * 0.75f;
        constexpr float quantLevels = 2048.0f;

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                const float dry = data[i];
                float wet = std::tanh(dry * preGain);
                wet = std::round(wet * quantLevels) / quantLevels;
                data[i] = dry * (1.0f - mix) + wet * mix;
            }
        }

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        lpf.process(context);
    }

private:
    double sampleRate = 44100.0;
    float drive = 0.5f;
    juce::dsp::StateVariableTPTFilter<float> lpf;
};