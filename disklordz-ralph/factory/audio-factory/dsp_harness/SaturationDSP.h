#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

/** Headless 12-bit-style saturation for offline benchmark (swarm-editable). */
struct SaturationDSP
{
    float drive = 0.65f;

    void setDrive(float d) noexcept { drive = std::clamp(d, 0.0f, 1.0f); }

    float processSample(float x) noexcept
    {
        const float g = 1.0f + drive * 3.5f;
        const float y = std::tanh(x * g);

        const float steps = 4096.0f;
        const float crushed = std::floor(y * steps + 0.5f) / steps;
        return std::clamp(crushed, -1.0f, 1.0f);
    }
};