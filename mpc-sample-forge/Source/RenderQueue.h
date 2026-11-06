#pragma once
#include "BatchJob.h"

namespace forge
{
    /** One batch waiting in the render queue: everything the main window had set up when
        "Add to Queue" was pressed, including which instrument and its sound. */
    struct QueueItem
    {
        juce::String status = "waiting";   // waiting / running / done / failed / cancelled
        juce::String result;               // summary line when finished
        juce::File doneFolder;             // where it was written

        juce::String pluginPath;           // empty = built-in synth
        juce::MemoryBlock pluginState;     // the plugin's sound when it was queued
        int instances = 1;                 // parallel VST3 copies
        bool smartPick = false;
        BatchRequest request;              // outputFolder = the folder it will be written to (made unique when it runs)

        juce::String describe() const;
    };

    juce::var queueItemToVar (const QueueItem& item);
    bool queueItemFromVar (const juce::var& v, QueueItem& item, juce::String& error);

    /** The app's queue file (survives restarts). */
    juce::File defaultQueueFile();
    bool saveQueue (const juce::File& f, const std::vector<QueueItem>& items);
    std::vector<QueueItem> loadQueue (const juce::File& f, juce::String& error);
}
