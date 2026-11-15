#pragma once

#include "TrapSamplerSound.h"
#include <JuceHeader.h>

/**
 * Monophonic 808-style glide: multiplicative pitch ratio smoothing (semitone musical glide).
 * No allocations in renderNextBlock.
 */
class Trap808Voice final : public juce::SynthesiserVoice
{
public:
    void setGlideSeconds(float seconds) noexcept { glideSeconds = juce::jmax(0.0f, seconds); }

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        pitchRatioSmoother.reset(sampleRate, static_cast<double>(glideSeconds));
        adsr.setSampleRate(sampleRate);
        juce::ADSR::Parameters p;
        p.attack = 0.001f;
        p.decay = 0.15f;
        p.sustain = 0.0f;
        p.release = 0.25f;
        adsr.setParameters(p);
    }

    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound* sound, int) override
    {
        const auto* sampleSound = dynamic_cast<const TrapSamplerSound*>(sound);
        if (sampleSound == nullptr || sampleSound->audio.getNumSamples() == 0)
            return;

        const double targetRatio =
            std::pow(2.0, (midiNoteNumber - sampleSound->rootNote) / 12.0)
            * (sampleSound->sourceRate / currentSampleRate);

        activeSound = sampleSound;
        level = velocity;
        pitchRatioSmoother.reset(currentSampleRate, static_cast<double>(glideSeconds));

        if (legatoEnabled && isVoiceActive())
            pitchRatioSmoother.setTargetValue(targetRatio);
        else
        {
            pitchRatioSmoother.setCurrentAndTargetValue(targetRatio);
            sourcePosition = 0.0;
            adsr.noteOn();
        }
    }

    void stopNote(float, bool allowTailOff) override
    {
        if (allowTailOff)
            adsr.noteOff();
        else
        {
            clearCurrentNote();
            adsr.reset();
        }
    }

    void pitchWheelMoved(int newPitchWheelValue) override
    {
        if (activeSound == nullptr)
            return;
        const float bendSemis = (static_cast<float>(newPitchWheelValue) / 8192.0f) * pitchBendRangeSemis;
        const double baseRatio =
            std::pow(2.0, bendSemis / 12.0) * pitchRatioSmoother.getTargetValue();
        pitchRatioSmoother.setTargetValue(baseRatio);
    }

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample,
                         int numSamples) override
    {
        if (activeSound == nullptr || !isVoiceActive())
            return;

        const auto& src = activeSound->audio;
        const int numCh = src.getNumChannels();
        const int len = src.getNumSamples();
        if (len < 2)
            return;

        const float* ch0 = src.getReadPointer(0);

        for (int s = 0; s < numSamples; ++s)
        {
            if (!adsr.isActive())
            {
                clearCurrentNote();
                break;
            }

            const double step = pitchRatioSmoother.getNextValue();
            const float amp = level * adsr.getNextSample();
            const float sample = getInterpolatedSample(ch0, numCh, src, len) * amp;

            for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
                outputBuffer.addSample(ch, startSample + s, sample);

            sourcePosition += step;
            if (sourcePosition >= static_cast<double>(len - 1))
                sourcePosition = static_cast<double>(len - 1);
        }
    }

private:
    float getInterpolatedSample(const float* ch0, int numCh, const juce::AudioBuffer<float>& src,
                                int len) const
    {
        const int i0 = static_cast<int>(sourcePosition);
        const int i1 = juce::jmin(i0 + 1, len - 1);
        const float frac = static_cast<float>(sourcePosition - static_cast<double>(i0));
        float a = ch0[i0];
        float b = ch0[i1];
        if (numCh > 1)
        {
            const float* ch1 = src.getReadPointer(1);
            a = 0.5f * (a + ch1[i0]);
            b = 0.5f * (b + ch1[i1]);
        }
        return a + frac * (b - a);
    }

    double currentSampleRate = 44100.0;
    double sourcePosition = 0.0;
    float level = 0.0f;
    float glideSeconds = 0.08f;
    float pitchBendRangeSemis = 12.0f;
    bool legatoEnabled = true;
    const TrapSamplerSound* activeSound = nullptr;

    juce::SmoothedValue<double, juce::ValueSmoothingTypes::Multiplicative> pitchRatioSmoother;
    juce::ADSR adsr;
};