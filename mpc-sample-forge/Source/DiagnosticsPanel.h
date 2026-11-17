#pragma once
#include "Diagnostics.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace forge
{
    /** The Diagnostics window: system report, self-test, log access. */
    class DiagnosticsPanel final : public juce::Component, private juce::Thread
    {
    public:
        struct Hooks
        {
            std::function<juce::String()> report;               // system report text
            std::function<juce::AudioProcessor*()> acquire;     // take the instrument away from the audio device
            std::function<void()> release;                      // give it back
        };

        explicit DiagnosticsPanel (Hooks h);
        ~DiagnosticsPanel() override;

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        void run() override;
        void startSelfTest();
        void append (const juce::String& s);
        void refresh();

        Hooks hooks;
        std::atomic<bool> cancel { false };
        bool holdingPlugin = false;
        juce::AudioProcessor* testPlugin = nullptr;
        DiagSummary summary;
        juce::String selfTestText;

        juce::TextEditor text;
        juce::TextButton runButton { "Run Self-Test" }, refreshButton { "Refresh" }, copyButton { "Copy Report" },
                         saveButton { "Save Report..." }, logButton { "Open Log Folder" };
        juce::Label status;
        std::unique_ptr<juce::FileChooser> chooser;
    };
}
