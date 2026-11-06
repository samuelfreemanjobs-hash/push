#pragma once
#include "BatchJob.h"
#include "PluginHost.h"
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
        std::unique_ptr<juce::DocumentWindow> editorWindow;
        std::unique_ptr<BatchJob> job;
        std::unique_ptr<juce::FileChooser> chooser;

        juce::Array<Style> styles;
        std::vector<SampleSpec> lastPlan;
        juce::File outputFolder;
        double progress = 0;

        juce::Label title, pluginLabel, styleLabel, styleDesc, keyLabel, countLabel, seedLabel,
                    prefixLabel, audioLabel, outLabel, outPath, engineLabel, rootLabel;
        juce::ComboBox pluginBox, styleBox, keyBox, scaleBox, rateBox, bitsBox, engineBox, rootBox, lengthBox;
        juce::ToggleButton banksToggle { "16-pad banks" };
        juce::TextButton browseButton { "Browse..." }, editButton { "Edit Sound" }, saveSoundButton { "Save Sound" },
                         loadSoundButton { "Load Sound" }, styleFileButton { "Style JSON..." }, diceButton { "Dice" },
                         outButton { "Choose..." }, planButton { "Preview List" }, generateButton { "Generate" },
                         auditionButton { "Audition Random" };
        juce::ToggleButton normaliseToggle { "Normalise -1 dB" };
        juce::Slider countSlider { juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft };
        juce::TextEditor seedEditor, prefixEditor, logView;
        juce::ProgressBar progressBar { progress };
        juce::MidiKeyboardState keyboardState;
        juce::MidiKeyboardComponent keyboard { keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
    };
}
