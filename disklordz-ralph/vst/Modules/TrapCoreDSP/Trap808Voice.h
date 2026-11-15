#pragma once

#include "TrapSamplerSound.h"
#include <JuceHeader.h>

class Trap808Voice final : public juce::SynthesiserVoice
{
public:
    void setGlideSeconds(float seconds) noexcept;

    void prepare(double sampleRate);
    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample,
                         int numSamples) override;

private:
    float getInterpolatedSample(const float* ch0, int numCh, const juce::AudioBuffer<float>& src,
                                int len) const;

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