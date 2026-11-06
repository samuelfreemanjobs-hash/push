#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace forge
{
    /** What Smart Pick measures about a rendered one-shot. */
    struct SampleFeatures
    {
        float peak = 0, crestDb = 0, lengthSec = 0;
        float centroidHz = 0;
        float subShare = 0, mudShare = 0, harshShare = 0;  // energy share of < 80 Hz, 200-500 Hz, 2.5-5 kHz
        float harmonicity = -1;                            // 0..1 periodicity at the root (-1 = not measured)
        std::vector<float> fingerprint;                    // spectral shape + envelope, level-independent
    };

    /** Perceived loudness of a one-shot: the loudest 400 ms window, K-weighted as in
        ITU-R BS.1770 (the measure behind LUFS). In LUFS-like units. */
    float momentaryLoudness (const juce::AudioBuffer<float>& audio, double sampleRate);

    SampleFeatures analyseSample (const juce::AudioBuffer<float>& audio, double sampleRate, int rootNote, bool measurePitch);

    /** RMS difference of two fingerprints in dB: small = sounds alike. */
    float fingerprintDistance (const SampleFeatures& a, const SampleFeatures& b);

    //==========================================================================
    struct PickCandidate
    {
        SampleFeatures features;
        juce::String group;     // candidates compete within a group (recipe + chord/key) to keep the batch's balance
        int rootNote = 60;
        int lowIntervalViolations = 0;   // from the notes played (see Theory.h)
        juce::String lowIntervalDetail;
        bool valid = true;      // false = render failed
    };

    struct PickVerdict
    {
        float score = 0;            // 0..100
        juce::StringArray reasons;  // why points were taken off
        bool kept = false;
        int similarTo = -1;         // candidate it was judged a near-duplicate of
        float nearest = 0;          // distance to the closest kept sample
    };

    struct PickSettings
    {
        float duplicateDb = 1.0f;     // closer than this to a kept sample = reported as a near-duplicate
        float varietyPerDb = 3.0f;    // score points a sample earns per dB of difference to the picks so far ...
        float varietyCapDb = 4.0f;    // ... up to this many dB (so variety can't outweigh a real problem)
    };

    /** Scores every candidate and chooses `target` of them. Within each group (keeping the
        batch's balance) it repeatedly takes the candidate with the best score plus a bonus
        for sounding different from everything already picked. */
    std::vector<PickVerdict> smartPick (const std::vector<PickCandidate>& candidates, int target, const PickSettings& settings = {});
}
