#pragma once
#include <juce_audio_utils/juce_audio_utils.h>

namespace forge
{
    /** Review & Keep: audition a batch quickly from the keyboard, mark keepers and
        rejects, then export just the keepers (in 16-pad banks) or bin the rejects.

        Keys: Up/Down move (and play), Space plays, K keep, X reject, U clear. */
    class ReviewPanel final : public juce::Component,
                              private juce::TableListBoxModel,
                              private juce::KeyListener,
                              private juce::ChangeListener,
                              private juce::Timer
    {
    public:
        ReviewPanel (const juce::File& batchFolder, juce::AudioDeviceManager& devices);
        ~ReviewPanel() override;

        void paint (juce::Graphics&) override;
        void resized() override;
        bool keyPressed (const juce::KeyPress&) override;

    private:
        enum class Mark { None, Keep, Reject };
        struct Entry
        {
            juce::File file;
            juce::String recipe, type, name, root;
            int score = -1;
            Mark mark = Mark::None;
        };

        // table
        int getNumRows() override;
        void paintRowBackground (juce::Graphics&, int row, int w, int h, bool selected) override;
        void paintCell (juce::Graphics&, int row, int col, int w, int h, bool selected) override;
        void selectedRowsChanged (int lastRowSelected) override;
        void cellDoubleClicked (int row, int col, const juce::MouseEvent&) override;
        void sortOrderChanged (int columnId, bool forwards) override;

        bool keyPressed (const juce::KeyPress&, juce::Component*) override;
        void changeListenerCallback (juce::ChangeBroadcaster*) override;

        void load();
        void saveMarks() const;
        void setMark (Mark m, bool advance);
        void play (int row);
        void stop();
        void exportKeepers();
        void trashRejects();
        void updateStatus();
        void setTranspose (int semitones, bool playNow = true);
        void startRangeCheck();
        void timerCallback() override;

        juce::File folder;
        juce::AudioDeviceManager& devices;
        std::vector<Entry> entries;

        juce::AudioFormatManager formats;
        juce::AudioSourcePlayer sourcePlayer;
        juce::AudioTransportSource transport;
        std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
        juce::AudioThumbnailCache thumbCache { 8 };
        juce::AudioThumbnail thumbnail { 512, formats, thumbCache };
        int playingRow = -1;
        int transpose = 0;                 // 16 Levels preview: play the sample re-pitched like MPC Tune
        std::vector<int> rangeSteps;       // range check sequence in progress
        size_t rangeIndex = 0;

        juce::TableListBox table { "samples", this };
        juce::TextButton playButton { "Play (Space)" }, keepButton { "Keep (K)" }, rejectButton { "Reject (X)" },
                         clearButton { "Clear (U)" }, exportButton { "Export Keepers..." }, trashButton { "Move Rejects to Trash" },
                         folderButton { "Open Folder" }, downButton { "<" }, upButton { ">" },
                         resetPitchButton { "0" }, rangeButton { "Range check (R)" };
        juce::ToggleButton autoPlay { "Play when selected" };
        juce::Label status, help, pitchLabel;
        juce::Rectangle<int> waveArea;
    };
}
