#pragma once

#include <atomic>
#include <string>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

#include "TrtAudioRunner.h"

/** Background TensorRT generation — never call from processBlock(). */
class NeuralLoopGeneratorThread final : public juce::Thread
{
public:
    NeuralLoopGeneratorThread(juce::AudioBuffer<float>& targetBuf, std::atomic<bool>& readyFlag)
        : juce::Thread("TRT_Phonk_Gen"),
          outputBuffer(targetBuf),
          isBufferReady(readyFlag)
    {
    }

    void setEnginePath(const juce::String& path) { enginePath = path; }

    void triggerGeneration(const std::string& prompt, int bpm)
    {
        currentPrompt = prompt + ", " + std::to_string(bpm) + " BPM, dry mix";
        if (isThreadRunning())
            return;
        startThread();
    }

    void run() override
    {
        isBufferReady.store(false, std::memory_order_release);
        TrtAudioRunner engine;
        if (!engine.loadEngine(enginePath.toStdString()))
        {
            isBufferReady.store(false, std::memory_order_release);
            return;
        }

        const std::vector<float> rawPCM = engine.inferAudio(currentPrompt, 8.0f);
        if (rawPCM.size() < 4)
        {
            isBufferReady.store(false, std::memory_order_release);
            return;
        }

        const int frames = static_cast<int>(rawPCM.size() / 2);
        outputBuffer.setSize(2, frames, false, true, true);
        for (int i = 0; i < frames; ++i)
        {
            outputBuffer.setSample(0, i, rawPCM[static_cast<size_t>(i) * 2]);
            outputBuffer.setSample(1, i, rawPCM[static_cast<size_t>(i) * 2 + 1]);
        }
        isBufferReady.store(true, std::memory_order_release);
    }

private:
    TrtAudioRunner engine;
    juce::AudioBuffer<float>& outputBuffer;
    std::atomic<bool>& isBufferReady;
    std::string currentPrompt;
    juce::String enginePath;
};