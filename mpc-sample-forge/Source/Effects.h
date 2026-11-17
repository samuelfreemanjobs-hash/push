#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

namespace forge
{
    /** How a rendered one-shot is shaped after it leaves the instrument,
        following the MPC signal flow:
        source -> varispeed -> trim (end point) -> amp envelope -> filters & FX. */
    struct Processing
    {
        double maxLength = 0;     // seconds: the sample's end point (0 = keep the natural tail)
        double attackMs = 0;      // fade-in ramp at the start
        double ampDecayMs = 0;    // One-Shot decay: exponential fall to silence over this time (0 = off)
        double fxTailMs = 0;      // extra room after the end point for reverb / delay tails
        int pitchShift = 0;       // semitones of tape-style varispeed before trimming (formants move too)
        double tempo = 90;        // BPM for tempo-synced delays
        juce::var fx;             // array of effect objects, applied in order

        bool isEmpty() const { return maxLength <= 0 && attackMs <= 0 && ampDecayMs <= 0 && pitchShift == 0 && ! fx.isArray(); }
    };

    /** Checks every effect has a known "type". */
    bool validateFx (const juce::var& fx, juce::String& error);
    juce::StringArray effectTypes();

    /** Plays the buffer at a different speed, like a tape deck: pitch and length change together. */
    void varispeed (juce::AudioBuffer<float>& buffer, double semitones);

    /** Fade-in ramp plus an exponential One-Shot decay that reaches silence at decayMs. */
    void applyAmpEnvelope (juce::AudioBuffer<float>& buffer, double sampleRate, double attackMs, double decayMs);

    /** Runs the effect chain in place. `seed` makes noise / modulation repeatable. */
    void applyFxChain (juce::AudioBuffer<float>& buffer, double sampleRate, const juce::var& fx, double tempo, juce::int64 seed);
}
