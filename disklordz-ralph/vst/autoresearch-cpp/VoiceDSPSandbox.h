#pragma once

/**
 * AGENT SANDBOX — only file the C++ autoresearch agent may edit (plus git commits).
 * Headless render + future port to TrapCoreDSP after Catch2 review.
 */
#include <algorithm>
#include <cmath>
#include <cstddef>

struct VoiceDSPSandbox
{
    void prepare(double sampleRate, int /*maxBlock*/)
    {
        fs = sampleRate > 8000.0 ? sampleRate : 44100.0;
    }

    void reset() noexcept
    {
        phase = 0.0;
        active = false;
        sampleIndex = 0;
    }

    void triggerKick(float velocity) noexcept
    {
        vel = std::clamp(velocity, 0.0f, 1.0f);
        phase = 0.0;
        sampleIndex = 0;
        active = true;
        pitchStartHz = 155.0f;
        rootHz = 49.0f;
        pitchTauSec = 0.028f;
        ampTauSec = 0.42f;
        drive = 1.35f + vel * 0.5f;
    }

    void renderKick(float* out, int numSamples) noexcept
    {
        if (out == nullptr || numSamples <= 0)
            return;

        const double twoPi = 6.283185307179586;
        for (int i = 0; i < numSamples; ++i)
        {
            if (!active)
            {
                out[i] = 0.0f;
                continue;
            }

            const double t = static_cast<double>(sampleIndex) / fs;
            const double pitchHz =
                static_cast<double>(rootHz)
                + (static_cast<double>(pitchStartHz) - static_cast<double>(rootHz))
                      * std::exp(-t / static_cast<double>(pitchTauSec));

            phase += twoPi * pitchHz / fs;
            if (phase > twoPi)
                phase -= twoPi * std::floor(phase / twoPi);

            const double env = std::exp(-t / static_cast<double>(ampTauSec)) * (0.35 + 0.65 * vel);
            double s = std::sin(phase) * env * static_cast<double>(drive);
            s = std::tanh(s);

            out[i] = static_cast<float>(s);
            ++sampleIndex;

            if (t > ampTauSec * 8.0)
                active = false;
        }
    }

    double fs = 44100.0;
    bool active = false;
    double phase = 0.0;
    std::size_t sampleIndex = 0;
    float vel = 1.0f;

    float pitchStartHz = 155.0f;
    float rootHz = 49.0f;
    float pitchTauSec = 0.028f;
    float ampTauSec = 0.42f;
    float drive = 1.35f;
};