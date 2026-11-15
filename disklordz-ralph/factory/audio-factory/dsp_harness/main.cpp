#include "SaturationDSP.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
struct WavData
{
    std::vector<float> samples;
    uint32_t sampleRate = 44100;
    uint16_t channels = 1;
};

WavData readWav(const std::string& path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f)
        throw std::runtime_error("Cannot open input WAV");

    char riff[4];
    f.read(riff, 4);
    if (std::string(riff, 4) != "RIFF")
        throw std::runtime_error("Not a RIFF file");

    f.seekg(12);
    char wave[4];
    f.read(wave, 4);

    uint16_t channels = 1;
    uint32_t sampleRate = 44100;
    uint16_t bits = 16;
    std::vector<float> interleaved;

    while (f)
    {
        char chunkId[4];
        if (!f.read(chunkId, 4))
            break;
        uint32_t chunkSize = 0;
        f.read(reinterpret_cast<char*>(&chunkSize), 4);

        const std::string id(chunkId, 4);
        if (id == "fmt ")
        {
            uint16_t format = 0;
            f.read(reinterpret_cast<char*>(&format), 2);
            f.read(reinterpret_cast<char*>(&channels), 2);
            f.read(reinterpret_cast<char*>(&sampleRate), 4);
            f.ignore(6);
            f.read(reinterpret_cast<char*>(&bits), 2);
            if (chunkSize > 16)
                f.ignore(static_cast<std::streamsize>(chunkSize - 16));
        }
        else if (id == "data")
        {
            const size_t frames = chunkSize / (channels * (bits / 8));
            interleaved.reserve(frames * channels);
            for (size_t i = 0; i < frames * channels; ++i)
            {
                if (bits == 16)
                {
                    int16_t s = 0;
                    f.read(reinterpret_cast<char*>(&s), 2);
                    interleaved.push_back(static_cast<float>(s) / 32768.0f);
                }
                else if (bits == 24)
                {
                    unsigned char b[3];
                    f.read(reinterpret_cast<char*>(b), 3);
                    const int32_t v = (b[0]) | (b[1] << 8) | (b[2] << 16);
                    const int32_t sign = (v & 0x800000) ? (v | ~0xffffff) : v;
                    interleaved.push_back(static_cast<float>(sign) / 8388608.0f);
                }
                else
                {
                    throw std::runtime_error("Unsupported bit depth");
                }
            }
        }
        else
        {
            f.ignore(static_cast<std::streamsize>(chunkSize));
        }
    }

    WavData out;
    out.sampleRate = sampleRate;
    out.channels = channels;
    out.samples = std::move(interleaved);
    return out;
}

void writeWav16(const std::string& path, const std::vector<float>& samples, uint32_t sr, uint16_t ch)
{
    std::ofstream f(path, std::ios::binary);
    if (!f)
        throw std::runtime_error("Cannot open output WAV");

    const uint32_t dataBytes = static_cast<uint32_t>(samples.size() * 2);
    const uint32_t riffSize = 36 + dataBytes;

    f.write("RIFF", 4);
    f.write(reinterpret_cast<const char*>(&riffSize), 4);
    f.write("WAVE", 4);
    f.write("fmt ", 4);
    const uint32_t fmtSize = 16;
    const uint16_t format = 1;
    const uint32_t byteRate = sr * ch * 2;
    const uint16_t blockAlign = ch * 2;
    const uint16_t bits = 16;
    f.write(reinterpret_cast<const char*>(&fmtSize), 4);
    f.write(reinterpret_cast<const char*>(&format), 2);
    f.write(reinterpret_cast<const char*>(&ch), 2);
    f.write(reinterpret_cast<const char*>(&sr), 4);
    f.write(reinterpret_cast<const char*>(&byteRate), 4);
    f.write(reinterpret_cast<const char*>(&blockAlign), 2);
    f.write(reinterpret_cast<const char*>(&bits), 2);
    f.write("data", 4);
    f.write(reinterpret_cast<const char*>(&dataBytes), 4);

    for (float s : samples)
    {
        const float c = std::clamp(s, -1.0f, 1.0f);
        const auto v = static_cast<int16_t>(std::lround(c * 32767.0f));
        f.write(reinterpret_cast<const char*>(&v), 2);
    }
}
} // namespace

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        std::cerr << "Usage: dsp_harness <in.wav> <out.wav> <drive 0..1>\n";
        return 1;
    }

    try
    {
        const auto in = readWav(argv[1]);
        SaturationDSP dsp;
        dsp.setDrive(static_cast<float>(std::atof(argv[3])));

        std::vector<float> out;
        out.resize(in.samples.size());
        const auto ch = static_cast<size_t>(in.channels);
        const auto frames = in.samples.size() / ch;

        for (size_t frame = 0; frame < frames; ++frame)
        {
            for (size_t c = 0; c < ch; ++c)
            {
                const size_t i = frame * ch + c;
                out[i] = dsp.processSample(in.samples[i]);
            }
        }

        writeWav16(argv[2], out, in.sampleRate, in.channels);
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return 2;
    }
}