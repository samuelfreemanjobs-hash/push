#pragma once
#include "PresetSweep.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace forge
{
    /** The "Preset Sweep..." window: where the presets come from and how many to keep.
        The recipe, key, format, Smart Pick and VST copies come from the main window. */
    class PresetSweepPanel final : public juce::Component
    {
    public:
        struct Info
        {
            juce::String instrumentName, recipeName;
            bool builtIn = false;
            int numPrograms = 0;          // what the plugin lists to hosts
            int standardFolderPresets = 0; // .vstpreset files found in the standard folders
        };

        PresetSweepPanel (const Info& info, juce::PropertiesFile* settings);

        std::function<void (const SweepOptions&)> onStart;

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        SweepOptions current() const;
        void update();
        void save() const;

        Info info;
        juce::PropertiesFile* settings;
        juce::File folder;
        std::unique_ptr<juce::FileChooser> chooser;

        juce::Label title, intro, sourceLabel, folderLabel, keepLabel, perLabel, maxLabel, includeLabel,
                    variationsLabel, strengthLabel, summary;
        juce::ComboBox sourceBox, perBox, maxBox;
        juce::Slider keepSlider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight },
                     variationsSlider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight },
                     strengthSlider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
        juce::TextEditor includeEditor;
        juce::TextButton folderButton { "Choose..." }, startButton { "Start Sweep" };
        juce::ToggleButton skipNames { "Skip arps, drums, FX and init patches by name" },
                           skipPulsing { "Skip presets that pulse while held (arps, gates)" },
                           keepProbes { "Save every preset's test sound (_Preset_Probes)" };
    };
}
