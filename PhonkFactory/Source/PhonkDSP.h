// PhonkDSP - JUCE-free core of Phonk Factory.
// Synthesises a Memphis-phonk drum kit (808-ish kick/snare/hats/cowbell run through a
// cassette / SP-1200 style chain), generates seeded Memphis drum patterns, and renders
// loops + WAV exports. Shared by the plugin, the standalone app and the kitgen CLI.
#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace phonk
{
enum Voice { Kick = 0, Snare, ClosedHat, OpenHat, Cowbell, NumVoices };
constexpr int kVariations = 4;
constexpr int kNumPads = NumVoices * kVariations;   // pad = voice * 4 + variation
constexpr int kFirstMidiNote = 36;                  // pads live on notes 36..55

const char* voiceName (int voice);                  // "Kick", "Snare", "ClosedHat", ...
inline int padIndex (int voice, int variation) { return voice * kVariations + variation; }

//==============================================================================
struct Rng   // splitmix64 - deterministic, platform independent
{
    explicit Rng (uint64_t seed = 1);
    uint64_t next();
    double uniform();          // [0,1)
    double gauss();            // N(0,1)
    int range (int n);         // [0,n)
    bool chance (double p);
    uint64_t s;
};

//==============================================================================
struct Biquad
{
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    float process (float x) noexcept
    {
        const double y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return (float) y;
    }
    void reset() noexcept { z1 = z2 = 0; }
    static Biquad make (bool highpass, double fc, double sr, double q);
    static Biquad makeFirstOrder (bool highpass, double fc, double sr);
};

// Butterworth filter, any order (cascade of biquads) - replaces scipy.signal.butter/lfilter.
class ButterFilter
{
public:
    ButterFilter (int order, double cutoffHz, double sr, bool highpass);
    float process (float x) noexcept;
    void process (std::vector<float>& v) noexcept;
private:
    std::vector<Biquad> sections;
};

//==============================================================================
// Per-sample cassette chain: saturation -> bit crush -> hiss -> lowpass.
struct CassetteParams
{
    double drive = 2.5, bitDepth = 12.0, hiss = 0.004, lpfHz = 9500.0;
};
void cassetteFx (std::vector<float>& audio, const CassetteParams& p, double sr, Rng& rng, bool normalize = true);

//==============================================================================
struct KitSettings
{
    int    seed = 1;
    double kickTune = 0, kickDecay = 1;      // semitones, decay multiplier
    double snareTune = 0, snareDecay = 1;
    double hatDecay = 1;
    double cowTune = 0;
    double grit = 1.0;                       // multiplies per-voice tape drive
    double bitDepth = 12.0;
    double hiss = 0.004;
    double tone = 1.0;                       // multiplies per-voice lowpass cutoff
};

struct Kit
{
    double sampleRate = 44100.0;
    std::array<std::vector<float>, kNumPads> samples;
};

std::vector<float> generateKick    (double sr, Rng& rng, double startFreq, double endFreq, double punch,
                                    double duration, const CassetteParams& fx);
std::vector<float> generateSnare   (double sr, Rng& rng, double toneFreq, double decayMul, const CassetteParams& fx);
std::vector<float> generateHat     (double sr, Rng& rng, bool open, double decayMul, double pitchRatio, const CassetteParams& fx);
std::vector<float> generateCowbell (double sr, Rng& rng, double rootFreq, const CassetteParams& fx);

Kit renderKit (const KitSettings& ks, double sr);

//==============================================================================
// Live/offline tape bus: wow & flutter -> saturation -> bit crush -> hiss -> lowpass.
struct BusSettings
{
    double drive = 2.4, bitDepth = 12.0, hiss = 0.005, lpfHz = 8800.0, wow = 0.5;
    bool operator== (const BusSettings& o) const
    {
        auto same = [] (double a, double b) { return ! (a < b) && ! (a > b); };
        return same (drive, o.drive) && same (bitDepth, o.bitDepth) && same (hiss, o.hiss) && same (lpfHz, o.lpfHz) && same (wow, o.wow);
    }
};

class BusFx
{
public:
    void prepare (double sr, uint64_t seed = 7);
    void setSettings (const BusSettings& s);
    void setWowRate (double hz) { wowHz = hz; }
    void reset();
    float process (float x) noexcept;
private:
    double sr = 44100.0, wowHz = 1.8, lfoPhase = 0.0;
    BusSettings st;
    Biquad lpf;
    std::vector<float> delay;
    int writePos = 0;
    double steps = 2048.0, tanhDrive = 1.0;
    Rng rng { 7 };
};

//==============================================================================
struct PatternEvent
{
    uint8_t step = 0, voice = 0, variation = 0;
    float micro = 0.f;       // offset inside/after the step, in steps (rolls, triplets)
    float velocity = 1.f;
};

struct PatternSettings
{
    int seed = 1;
    int bars = 2;            // 1, 2 or 4
    double variation = 0.5;  // 0 = pure classic groove, 1 = heavily mutated
    double rolls = 0.3;      // hat roll probability
    double cowbell = 0.2;
    double humanize = 0.15;
};

struct Pattern
{
    int bars = 2;
    std::vector<PatternEvent> events;
    int totalSteps() const { return bars * 16; }
};

Pattern generatePattern (const PatternSettings& ps);
// Position of an event in steps from loop start (swing delays odd 16ths).
double eventTime (const PatternEvent& e, double swing);

//==============================================================================
struct MixSettings { std::array<float, NumVoices> level { 1.f, 1.f, 1.f, 1.f, 1.f }; };

// Renders a seamless loop (tails wrap round the loop point, wow LFO is quantised to whole cycles).
std::vector<float> renderLoop (const Kit& kit, const Pattern& pat, double bpm, double swing,
                               const MixSettings& mix, const BusSettings& bus, bool normalize = true);

bool writeWav (const std::string& path, const std::vector<float>& mono, double sr, int bits = 16);

//==============================================================================
struct ExportOptions
{
    double sampleRate = 44100.0;
    int bits = 16;
    KitSettings kit;
    BusSettings bus;
    MixSettings mix;
    PatternSettings pattern;   // used for the "current" loop
    double swing = 0.03;
    double currentBpm = 116.0;
    int extraLoops = 5;        // seeded variations on top of the 3 classic loops
};

// Writes 20 one-shots + classic loops (112/118/124) + variations + the current loop. Returns file count.
int exportKit (const std::string& folder, const ExportOptions& opt,
               const std::function<void (const std::string&)>& log = {});
}
