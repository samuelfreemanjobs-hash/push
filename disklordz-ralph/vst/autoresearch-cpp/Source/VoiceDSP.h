#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <cmath>

/** AGENT SANDBOX — the only file the C++ autoresearch agent may edit. */
class VoiceDSP
{
public:
    void prepare(double newSampleRate, int /*maxBlockSize*/)
    {
        sampleRate = newSampleRate;
        reset();
    }

    void reset()
    {
        phase = 0.0f;
        timeSec = 0.0f;
        envLevel = 0.0f;
        isActive = false;
    }

    void triggerNote(int midiNote, float velocity)
    {
        currentNote = midiNote;
        currentVelocity = std::clamp(velocity, 0.0f, 1.0f);
        phase = 0.0f;
        timeSec = 0.0f;
        envLevel = 1.0f;
        isActive = true;
    }

    void processBlock(juce::AudioBuffer<float>& buffer)
    {
        juce::ScopedNoDenormals noDenormals;

        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();
        if (numChannels == 0 || !isActive)
            return;

        auto* left = buffer.getWritePointer(0);
        const float dt = 1.0f / static_cast<float>(sampleRate);
        const float twoPi = juce::MathConstants<float>::twoPi;

        for (int i = 0; i < numSamples; ++i)
        {
            if (!isActive)
                break;

            float sample = renderKickSample(dt, twoPi);
            if (!std::isfinite(sample))
                sample = 0.0f;
            left[i] += sample * currentVelocity;

            timeSec += dt;
            if (envLevel < 1e-5f && timeSec > 0.05f)
                isActive = false;
        }

        for (int ch = 1; ch < numChannels; ++ch)
            buffer.copyFrom(ch, 0, buffer, 0, 0, numSamples);
    }

private:
    float renderKickSample(float dt, float twoPi)
    {
        const float fStart = 165.0f;
        const float fEnd = 48.0f;
        const float pitchDecay = 0.055f;
        const float currentFreq = fEnd + (fStart - fEnd) * std::exp(-timeSec / pitchDecay);

        phase += twoPi * currentFreq * dt;
        if (phase >= twoPi)
            phase -= twoPi;
        const float osc = std::sin(phase);

        const float ampDecay = 0.38f;
        const float attackTime = 0.0015f;
        const float attackGain = std::min(1.0f, timeSec / attackTime);
        envLevel = attackGain * std::exp(-timeSec / ampDecay);

        const float drive = 1.6f;
        return std::tanh(osc * envLevel * drive) * 0.90f;
    }

    double sampleRate = 44100.0;
    float phase = 0.0f;
    float timeSec = 0.0f;
    float envLevel = 0.0f;
    float currentVelocity = 1.0f;
    int currentNote = 36;
    bool isActive = false;
};