#pragma once
#include "ProgressionLibrary.h"
#include "BatchJob.h"
#include "PresetSweep.h"
#include "RenderQueue.h"
#include "PluginHost.h"
#include "Diagnostics.h"
#include <juce_audio_utils/juce_audio_utils.h>

namespace forge
{
    class MainComponent final : public juce::Component,
                                private juce::MidiKeyboardState::Listener
    {
    public:
        MainComponent();
        ~MainComponent() override;

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        // instrument
        void choosePlugin (int comboId);
        void browseForPlugin();
        void loadInstrument (const juce::String& path);
        void useBuiltIn();
        void setInstrument (std::unique_ptr<juce::AudioProcessor> p, const juce::String& path);
        void openEditor();
        void closeEditor();
        void saveSound();
        void loadSound();

        // batch
        void styleChanged();
        void loadStyleJson();
        bool buildRequest (BatchRequest& req, std::vector<SampleSpec>& specs);
        void previewPlan();
        void generateOrCancel();
        void jobFinished (bool ok, const juce::String& summary);
        void auditionSpec (int index);
        void setBusy (bool busy);
        void openDiagnostics();
        void openReview (const juce::File& batchFolder);
        void openStyleEditor();
        void openPresetSweep();
        void startSweep (const SweepOptions& options);
        void sweepFinished (bool ok, const juce::String& summary, const juce::File& folder);
        void makeExtraInstances (BatchRequest& req);

        // render queue
        void addToQueue();
        void openQueue();
        void startQueue();
        void stopQueue();
        void runNextQueued();
        void queueItemFinished (bool ok, const juce::String& summary, const juce::File& folder);
        void saveQueueFile();
        void updateQueueButton();
        void addOrReplaceStyle (const Style& st, bool select);
        void updateChordRow();
        const ProgressionSet* selectedProgressionSet (bool orFirst = false) const;
        void playChord (int index);
        SampleSpec chordSpec (int index) const;
        void exportChords (const std::vector<int>& indices);
        void moreLikeThis (const juce::File& batchFolder, const juce::File& sample);
        int currentTonic() const;
        std::vector<int> currentScale() const;
        void makeRegistersForKeepers (const juce::File& batchFolder, const juce::Array<juce::File>& keepers);
        void startJob (BatchRequest req, std::vector<SampleSpec> specs, bool openReviewAfter);
        void updateEstimate();
        double expectedSpeed() const;
        void chooseFolderToReview();
        void applyLiveEngine();
        juce::String selectedEngine() const;
        void log (const juce::String& s);

        void handleNoteOn (juce::MidiKeyboardState*, int ch, int note, float vel) override;
        void handleNoteOff (juce::MidiKeyboardState*, int ch, int note, float vel) override;

        juce::ApplicationProperties props;
        PluginHost host;
        juce::AudioDeviceManager deviceManager;
        juce::AudioProcessorPlayer player;
        std::unique_ptr<juce::AudioProcessor> instrument;
        juce::String instrumentPath;
        juce::StringArray installedPlugins;
        std::unique_ptr<juce::DocumentWindow> editorWindow, diagnosticsWindow, reviewWindow, styleEditorWindow;
        juce::File lastBatchFolder;
        std::vector<std::unique_ptr<juce::AudioPluginInstance>> extraInstances; // parallel VST3 copies during a job
        std::unique_ptr<BatchJob> job;
        std::unique_ptr<PresetSweep> sweep;
        std::unique_ptr<juce::DocumentWindow> sweepWindow;
        juce::MemoryBlock stateBeforeSweep;   // the sound to go back to after a sweep

        std::vector<QueueItem> queue;
        std::unique_ptr<juce::DocumentWindow> queueWindow;
        bool queueRunning = false, queueStopRequested = false;
        int queueIndex = -1, queueCopies = 1, queueRunNumber = 0, queueRunTotal = 0;
        juce::String queueStartPath;          // the instrument + sound to go back to after the queue
        juce::MemoryBlock queueStartState;
        std::unique_ptr<juce::FileChooser> chooser;

        juce::Array<Style> styles;
        std::vector<SampleSpec> lastPlan;
        juce::File outputFolder;
        double progress = 0;

        juce::Label title, pluginLabel, styleLabel, styleDesc, keyLabel, countLabel, seedLabel,
                    prefixLabel, audioLabel, outLabel, outPath, engineLabel, rootLabel;
        juce::ComboBox pluginBox, styleBox, keyBox, scaleBox, rateBox, bitsBox, engineBox, rootBox, lengthBox, levelBox, registersBox, bankBox, varietyBox, instancesBox,
                       progressionBox,
                       auditionOctaveBox,
                       takesBox;            // round robins: 1-4 takes of every sample   // Audition Random: play in this octave (or as planned / the chord row's)   // progression-library recipes (Trap Progressions): which set
        juce::ToggleButton banksToggle { "16-pad banks" };
        juce::TextButton browseButton { "Browse..." }, editButton { "Edit Sound" }, saveSoundButton { "Save Sound" },
                         loadSoundButton { "Load Sound" }, styleFileButton { "Style JSON..." }, diceButton { "Dice" },
                         outButton { "Choose..." }, planButton { "Preview List" }, generateButton { "Generate" },
                         auditionButton { "Audition Random" }, diagnosticsButton { "Diagnostics" },
                         reviewButton { "Review..." }, moreButton { "+16 More" }, sweepButton { "Preset Sweep..." }, addQueueButton { "Add to Queue" }, queueButton { "Queue" },
                         size16 { "16" }, size32 { "32" }, size64 { "64" };
        juce::Label estimateLabel, sizeLabel;

        // chord row: preview / export the selected style's diatonic chords
        juce::OwnedArray<juce::TextButton> chordButtons;
        juce::ComboBox chordOctaveBox, inversionBox;
        juce::TextButton exportChordButton { "Export chord" }, exportRowButton { "Export all 7" }, editStyleButton { "Edit Style..." };
        juce::Label chordRowLabel, exportInfo;   // exportInfo: what Export chord will render (notes + octave)
        void updateExportInfo();
        std::vector<DiatonicChord> currentChords;
        int selectedChord = 0;
        juce::ToggleButton smartPickToggle { "Smart Pick" }, shortNamesToggle { "Short names" };
        juce::Slider countSlider { juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft };
        juce::TextEditor seedEditor, prefixEditor, logView;
        juce::ProgressBar progressBar { progress };
        juce::MidiKeyboardState keyboardState;
        juce::MidiKeyboardComponent keyboard { keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
    };
}
