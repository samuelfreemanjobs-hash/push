// Lock-free hand-off of heap objects from a message/worker thread to the audio thread.
// The audio thread never allocates or frees: replaced objects are queued back for deletion.
#pragma once
#include <atomic>
#include <array>
#include <juce_core/juce_core.h>

template <typename T>
class RcuSlot
{
public:
    ~RcuSlot()
    {
        delete current;
        delete pending.exchange (nullptr);
        collectGarbage();
    }

    // any non-audio thread
    void publish (T* p) { delete pending.exchange (p); }

    // message thread only
    void collectGarbage()
    {
        int s1, n1, s2, n2;
        fifo.prepareToRead (fifo.getNumReady(), s1, n1, s2, n2);
        for (int i = 0; i < n1; ++i) delete graveyard[(size_t) (s1 + i)];
        for (int i = 0; i < n2; ++i) delete graveyard[(size_t) (s2 + i)];
        fifo.finishedRead (n1 + n2);
    }

    // audio thread only: returns the live object (may be null before the first publish)
    T* acquire()
    {
        if (pending.load (std::memory_order_relaxed) != nullptr && fifo.getFreeSpace() > 0)
        {
            if (T* p = pending.exchange (nullptr))
            {
                if (current != nullptr)
                {
                    int s1, n1, s2, n2;
                    fifo.prepareToWrite (1, s1, n1, s2, n2);
                    graveyard[(size_t) s1] = current;
                    fifo.finishedWrite (1);
                }
                current = p;
                swapped = true;
            }
        }
        return current;
    }

    bool takeSwapped() { const bool s = swapped; swapped = false; return s; }

private:
    std::atomic<T*> pending { nullptr };
    T* current = nullptr;
    bool swapped = false;
    juce::AbstractFifo fifo { 16 };
    std::array<T*, 16> graveyard {};
};
