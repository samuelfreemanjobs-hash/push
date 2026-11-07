#pragma once
#include "Arranger.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace forge
{
    /** The chord blocks of a progression, left to right, sized by length. Click to select,
        drag to reorder, double-click to hear one. */
    class BlockLane final : public juce::Component
    {
    public:
        std::function<void (int)> onSelect, onPlay;
        std::function<void (int from, int to)> onMove;

        void setBlocks (const std::vector<ChordBlock>* blocks, int tonic, int selected);
        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;
        void mouseDoubleClick (const juce::MouseEvent&) override;

    private:
        int blockAt (float x) const;
        int dropIndexAt (float x) const;
        juce::Rectangle<float> blockBounds (int index) const;

        const std::vector<ChordBlock>* blocks = nullptr;
        int tonic = 0, selected = -1, dragging = -1, dropAt = -1;
    };

    /** The Progressions page: build a progression from chord blocks (or let a genre preset write one),
        with keys, bass, pad / strings, lead and drums parts, and export MIDI and audio loops. */
    class ProgressionPanel final : public juce::Component
    {
    public:
        struct Hooks
        {
            std::function<std::vector<PaletteChord>()> palette;                        // the main window's palette
            std::function<void (const juce::AudioBuffer<float>&, double)> play;      // play a rendered loop
            std::function<void()> stop;
            std::function<juce::File()> outputFolder;
            std::function<double()> sampleRate;                                       // for playback (the audio device's)
            std::function<double()> exportRate;                                       // for files (the Format setting)
            std::function<int()> bitDepth;
            std::function<void (const juce::String&)> log;
            std::function<juce::StringArray()> installedPlugins;                      // VST3 paths
            std::function<std::unique_ptr<juce::AudioPluginInstance> (const juce::String&, juce::String&)> loadPlugin;
            std::function<bool (juce::String& path, juce::MemoryBlock& state)> mainSound; // the main window's plugin + its sound
        };

        ProgressionPanel (Hooks hooks, juce::PropertiesFile* settings);
        ~ProgressionPanel() override;

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        void presetChanged (bool keepChords);
        void generate();
        void blocksChanged();            // re-write the parts, refresh the view
        void refreshKeyChords();
        void selectBlock (int index);
        void addBlock (const ChordBlock& b);
        void playLoop();
        void exportFiles (bool midi, bool loops);
        void soundChanged (int part);
        void setPartPlugin (int part, std::unique_ptr<juce::AudioPluginInstance> instance, const juce::String& path);
        void openPartEditor (int part);
        InstrumentFor instrumentFor();
        /** Renders on a background thread with a progress window, then calls `done` on the message thread. */
        void renderInBackground (std::function<bool (std::function<void (double)>)> work, std::function<void()> done);
        juce::String baseName() const;
        const SongPreset& preset() const;
        std::vector<ChordBlock> keyChords() const;

        Hooks hooks;
        juce::PropertiesFile* settings;
        Arrangement arr;
        int selected = -1;
        juce::File lastFolder;

        juce::Label title, presetDesc, selectedLabel, addLabel, typedLabel, tracksLabel, status;
        juce::ComboBox presetBox, keyBox, barsBox, lengthBox, rootBox, typeBox;
        juce::Slider tempoSlider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight },
                     swingSlider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
        juce::Label tempoLabel, swingLabel;
        juce::TextEditor seedEditor, typedEditor;
        juce::TextButton diceButton { "Dice" }, generateButton { "Generate Progression" },
                         leftButton { "<" }, rightButton { ">" }, dupButton { "Duplicate" }, delButton { "Delete" }, clearButton { "Clear" },
                         addButton { "Add chord" }, addPaletteButton { "Add palette" }, setTypedButton { "Use this" },
                         newPartsButton { "New parts" }, playButton { "Play loop" }, stopButton { "Stop" },
                         midiButton { "Export MIDI" }, loopButton { "Export Loop" }, bothButton { "Export MIDI + Loop" },
                         openButton { "Open folder" };
        juce::OwnedArray<juce::TextButton> keyChordButtons;
        juce::OwnedArray<juce::ToggleButton> partToggles;
        juce::OwnedArray<juce::ComboBox> partRecipes;
        BlockLane lane;

        // your instruments for the loop: one optional VST3 per melodic part
        juce::OwnedArray<juce::ComboBox> partSounds;
        juce::OwnedArray<juce::TextButton> partEditButtons;
        std::unique_ptr<juce::AudioPluginInstance> partPlugins[4];
        std::unique_ptr<juce::DocumentWindow> partEditors[4];
        juce::StringArray installed;
        std::unique_ptr<juce::FileChooser> chooser;
        juce::ComboBox finishBox, chopsBox;
        juce::Label finishLabel, chopsLabel;
        std::unique_ptr<juce::Thread> renderThread;
    };
}
