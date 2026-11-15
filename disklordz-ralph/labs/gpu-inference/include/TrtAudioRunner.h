#pragma once

#include <string>
#include <vector>

/** Stub — replace with TensorRT Model Connect generated implementation. */
class TrtAudioRunner
{
public:
    bool loadEngine(const std::string& enginePath)
    {
        loaded = !enginePath.empty();
        path = enginePath;
        return loaded;
    }

    bool isLoaded() const noexcept { return loaded; }

    std::vector<float> inferAudio(const std::string& prompt, float durationSec)
    {
        if (!loaded)
            return {};
        (void) prompt;
        (void) durationSec;
        return {};
    }

private:
    bool loaded = false;
    std::string path;
};