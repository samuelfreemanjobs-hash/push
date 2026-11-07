#pragma once

#include <cstdint>
#include "Params.h"

namespace kf
{
// A snapshot of one track's synthesis parameters (real units).
struct VoiceParams
{
    int   wave        = 0;     // 0 sine, 1 triangle, 2 saw, 3 square
    float pitch       = 55.0f; // Hz
    float pEnvAmt     = 24.0f; // semitones
    float pEnvDecayMs = 40.0f;
    float toneDecayMs = 400.0f;
    float toneLevel   = 1.0f;
    float fmAmt       = 0.0f;
    float fmRatio     = 1.5f;
    float fmDecayMs   = 4000.0f; // modulation-index envelope

    int   noiseType    = 0;    // 0 white, 1 metallic
    float noiseLevel   = 0.0f;
    float noiseDecayMs = 150.0f;
    int   filterMode   = 0;    // 0 LP, 1 BP, 2 HP
    float cutoff       = 8000.0f;
    float reso         = 0.7f;
    int   bursts       = 1;

    float click    = 0.2f;
    float attackMs = 0.0f;
    float drive    = 0.0f;
    float level    = 0.8f;
    float pan      = 0.0f;
    int   choke    = 0;
    float lofiSend = 1.0f;   // how much of this voice goes through the vintage bus

    // Build from an array of real parameter values indexed by kf::VP.
    template <typename Array>
    static VoiceParams fromValues (const Array& v)
    {
        VoiceParams p;
        p.wave         = (int) v[(size_t) Wave];
        p.pitch        = v[(size_t) Pitch];
        p.pEnvAmt      = v[(size_t) PEnvAmt];
        p.pEnvDecayMs  = v[(size_t) PEnvDecay];
        p.toneDecayMs  = v[(size_t) ToneDecay];
        p.toneLevel    = v[(size_t) ToneLevel];
        p.fmAmt        = v[(size_t) FmAmt];
        p.fmRatio      = v[(size_t) FmRatio];
        p.fmDecayMs    = v[(size_t) FmDecay];
        p.noiseType    = (int) v[(size_t) NoiseType];
        p.noiseLevel   = v[(size_t) NoiseLevel];
        p.noiseDecayMs = v[(size_t) NoiseDecay];
        p.filterMode   = (int) v[(size_t) FilterMode];
        p.cutoff       = v[(size_t) Cutoff];
        p.reso         = v[(size_t) Reso];
        p.bursts       = (int) (v[(size_t) Burst] + 0.5f);
        p.click        = v[(size_t) Click];
        p.attackMs     = v[(size_t) Attack];
        p.drive        = v[(size_t) Drive];
        p.level        = v[(size_t) Level];
        p.pan          = v[(size_t) Pan];
        p.choke        = (int) (v[(size_t) Choke] + 0.5f);
        p.lofiSend     = v[(size_t) LoFiSend];
        return p;
    }
};

// Topology-preserving-transform state variable filter (Cytomic / Simper).
class SVF
{
public:
    void setCoefficients (float cutoffHz, float q, double sampleRate);
    void reset() { ic1 = ic2 = 0.0f; }
    float process (float x, int mode);

private:
    float g = 0, k = 1, a1 = 0, a2 = 0, a3 = 0;
    float ic1 = 0, ic2 = 0;
};

// One synthesised drum voice: a pitched oscillator with pitch envelope and FM,
// a filtered noise / metallic generator with optional clap-style bursts, a
// transient click, drive and an output level.
class DrumVoice
{
public:
    void prepare (double sampleRate);

    // Update coefficients from the current parameters (call once per block).
    void setParams (const VoiceParams& p);

    void trigger (float velocity);
    void choke();  // quick fade-out (used by choke groups)

    bool isActive() const noexcept { return active; }

    float process() noexcept;

private:
    float nextWhite() noexcept;
    float nextMetal() noexcept;
    static float decayCoef (float ms, double sr) noexcept;
    float osc (float phase) const noexcept;

    double sr = 44100.0;
    VoiceParams params;

    bool  active = false;
    bool  choking = false;
    float velocity = 1.0f;

    // Envelopes (exponential decays)
    float toneEnv = 0, noiseEnv = 0, pitchEnv = 0, clickEnv = 0, fmEnv = 0, chokeGain = 1;
    float toneCoef = 0, noiseCoef = 0, pitchCoef = 0, clickCoef = 0, fmCoef = 0, chokeCoef = 0, burstCoef = 0;

    // Attack ramp
    int attackSamples = 0;
    int sampleCount = 0;

    // Clap-style noise re-triggering
    int burstsLeft = 0;
    int burstCounter = 0;
    int burstInterval = 0;

    // Oscillators
    float phase = 0, modPhase = 0;
    float metalPhase[6] {};
    float metalInc[6] {};
    float driveGain = 1.0f, driveNorm = 1.0f;

    SVF noiseFilter;
    SVF clickFilter;
    uint32_t rng = 0x12345678u;
};
} // namespace kf
