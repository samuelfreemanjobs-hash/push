#include "LugerIdentity.h"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Offline renderer only — not for audio thread.
static bool writeWavMono24(const std::string& path, const std::vector<float>& samples, double sampleRate)
{
    std::ofstream out(path, std::ios::binary);
    if (!out)
        return false;
    const uint32_t numSamples = static_cast<uint32_t>(samples.size());
    const uint16_t channels = 1;
    const uint16_t bits = 24;
    const uint32_t byteRate = static_cast<uint32_t>(sampleRate * channels * bits / 8);
    const uint16_t blockAlign = channels * bits / 8;
    const uint32_t dataSize = numSamples * 3;
    out.write("RIFF", 4);
    uint32_t chunkSize = 36 + dataSize;
    out.write(reinterpret_cast<const char*>(&chunkSize), 4);
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    uint32_t fmtSize = 16;
    uint16_t fmt = 1;
    out.write(reinterpret_cast<const char*>(&fmtSize), 4);
    out.write(reinterpret_cast<const char*>(&fmt), 2);
    out.write(reinterpret_cast<const char*>(&channels), 2);
    uint32_t sr = static_cast<uint32_t>(sampleRate);
    out.write(reinterpret_cast<const char*>(&sr), 4);
    out.write(reinterpret_cast<const char*>(&byteRate), 4);
    out.write(reinterpret_cast<const char*>(&blockAlign), 2);
    out.write(reinterpret_cast<const char*>(&bits), 2);
    out.write("data", 4);
    out.write(reinterpret_cast<const char*>(&dataSize), 4);
    for (float s : samples)
    {
        const int32_t v =
            static_cast<int32_t>(std::clamp(double(s), -1.0, 1.0) * 8388607.0);
        const auto b0 = static_cast<char>(v & 0xff);
        const auto b1 = static_cast<char>((v >> 8) & 0xff);
        const auto b2 = static_cast<char>((v >> 16) & 0xff);
        out.write(&b0, 1);
        out.write(&b1, 1);
        out.write(&b2, 1);
    }
    return true;
}

int main(int argc, char** argv)
{
    std::string outDir = argc > 1 ? argv[1] : "renders";
    const double sr = 48000.0;
    const dl::luger::Identity ids[] = {
        dl::luger::Identity::BrickSquad,
        dl::luger::Identity::OrchestralWarfare,
        dl::luger::Identity::TrapAnthem,
        dl::luger::Identity::DarkSouthern,
        dl::luger::Identity::ArenaTrap,
    };
    int written = 0;
    for (const auto id : ids)
    {
        const std::string name = dl::luger::identityName(id);
        for (const auto voice : { dl::Voice::Kick, dl::Voice::Snare, dl::Voice::Bass808 })
        {
            const auto d = dl::luger::designForLuger(voice, id, 20261115u);
            const auto samples = dl::renderDesign(d, sr);
            const char* voiceTag = voice == dl::Voice::Kick ? "kick" : voice == dl::Voice::Snare ? "snare" : "808";
            const std::string path = outDir + "/Luger_" + name + "_" + voiceTag + ".wav";
            if (writeWavMono24(path, samples, sr))
                ++written;
        }
    }
    std::cout << "Wrote " << written << " M1 showcase WAVs under " << outDir << "\n";
    return written > 0 ? 0 : 1;
}