#pragma once

#include "DrumVoice.h"
#include <array>

namespace kf
{
struct BusParams
{
    float drive   = 0.0f;
    float bits    = 16.0f;
    float rate    = 48000.0f;
    float pitch   = 0.0f;     // semitones of "pitch up before sampling, down on playback"
    float lpf     = 20000.0f;
    float lowKeep = 0.0f;     // crossover: below this frequency the signal bypasses the crusher (0 = off)
    float punch   = 0.0f;     // -1..1 transient attack
    float tail    = 0.0f;     // -1..1 transient sustain
    float vinyl   = 0.0f;

    template <typename Array>
    static BusParams fromValues (const Array& v)
    {
        BusParams p;
        p.drive = v[(size_t) BDrive]; p.bits = v[(size_t) BBits]; p.rate = v[(size_t) BRate]; p.pitch = v[(size_t) BPitch];
        p.lpf = v[(size_t) BLpf]; p.lowKeep = v[(size_t) BLowKeep]; p.punch = v[(size_t) BPunch]; p.tail = v[(size_t) BTail]; p.vinyl = v[(size_t) BVinyl];
        return p;
    }
};

// Stereo 12-bit-sampler emulation applied to the drum "send":
//   [crossover: lows bypass] -> drive -> bit reduction -> sample & hold decimation -> 2-pole low-pass
//
// The classic "pitch up before sampling, pitch down on playback" trick is
// equivalent, for a sampler with no interpolation, to sampling the original at
// rate / 2^(semitones/12) with a zero-order hold. That pushes the effective
// Nyquist down so highs fold back as ringing alias tones, and the hold steps
// become longer -- the gritty SP-1200 sound.
class VintageCrusher
{
public:
    void prepare (double sampleRate);
    void setParams (const BusParams& p);
    void reset();
    void process (float& left, float& right) noexcept;

    // Effective sampling rate after the pitch trick (for display / tests).
    double getEffectiveRate() const noexcept { return effectiveRate; }

private:
    float crushChannel (float x) noexcept;

    double sr = 48000.0;
    BusParams params;
    double effectiveRate = 48000.0;
    double holdInc = 1.0;
    double holdPhase = 1.0;
    float held[2] {};
    float driveGain = 1.0f, driveMakeup = 1.0f, quantSteps = 32768.0f;
    bool useCrossover = false, useLpf = false, useHold = false, useBits = false, useDrive = false;

    SVF xoverLp1[2], xoverLp2[2], xoverHp1[2], xoverHp2[2];
    SVF smooth[2];
};

// Stereo-linked transient shaper: compares fast and slow envelope followers to
// find the attack (punch) and the decay (tail) portions of each hit.
class TransientShaper
{
public:
    void prepare (double sampleRate);
    void setAmounts (float punch, float tail) { punchAmt = punch; tailAmt = tail; }
    void reset();
    void process (float& left, float& right) noexcept;

private:
    static float coef (double ms, double sr);
    float attFastA = 0, attSlowA = 0, relLong = 0, susFastR = 0, susSlowR = 0, atk1 = 0;
    float envAttFast = 0, envAttSlow = 0, envSusFast = 0, envSusSlow = 0;
    float punchAmt = 0, tailAmt = 0;
};

// Vinyl surface: filtered hiss plus random crackles and occasional pops.
class VinylNoise
{
public:
    void prepare (double sampleRate);
    void setAmount (float a) { amount = a; }
    void process (float& left, float& right) noexcept;

private:
    float white() noexcept;
    double sr = 48000.0;
    float amount = 0.0f;
    float hissL = 0, hissR = 0, hissCoef = 0;
    float crackle = 0, crackleCoef = 0, cracklePrev = 0;
    float pop = 0, popCoef = 0, popPhase = 0, popInc = 0;
    uint32_t rng = 0x9e3779b9u;
};
} // namespace kf
