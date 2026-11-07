#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "SequencerGrid.h"
#include "VoicePresets.h"

//==============================================================================
class KitForgeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    KitForgeLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& bg,
                               bool highlighted, bool down) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int bx, int by, int bw, int bh, juce::ComboBox&) override;
};

//==============================================================================
// A labelled rotary knob.
class Knob : public juce::Component
{
public:
    Knob (const juce::String& name, bool bipolar = false);
    void resized() override;

    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    void attach (juce::AudioProcessorValueTreeState&, const juce::String& paramID);
    void setColour (juce::Colour c);
};

// A labelled combo box.
class Choice : public juce::Component
{
public:
    explicit Choice (const juce::String& name);
    void resized() override;

    juce::ComboBox box;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;

    void attach (juce::AudioProcessorValueTreeState&, const juce::String& paramID);
};

//==============================================================================
// The left-hand track header: number, name/type, trigger LED. Clicking it
// selects the track for editing and auditions the sound.
class TrackHeader : public juce::Component
{
public:
    TrackHeader (KitForgeProcessor&, int track);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void update();

    std::function<void (int)> onSelect;
    bool selected = false;

private:
    KitForgeProcessor& proc;
    int track;
    int lastTriggerCount = 0;
    float flash = 0.0f;
};

//==============================================================================
// Shows the selected pad's rendered hit (through the vintage bus) with its
// transient and hold regions. Click to audition; drag it out to drop the
// sound as a WAV into a DAW, sampler or folder.
class WaveformView : public juce::Component, public juce::SettableTooltipClient
{
public:
    explicit WaveformView (KitForgeProcessor&);
    void setVoice (const kf::VoiceDef& voice, const kf::BusDef& bus, const juce::String& name, int track, juce::Colour colour);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;

private:
    KitForgeProcessor& proc;
    kf::VoiceDef voice {};
    kf::BusDef bus {};
    juce::String name;
    int track = 0;
    juce::Colour colour;
    std::vector<float> peaks;          // per-pixel peak (normalised)
    double lengthSeconds = 0.0;
    bool dragStarted = false;
};

//==============================================================================
// Synth engine editor for the selected track.
class VoicePanel : public juce::Component
{
public:
    explicit VoicePanel (KitForgeProcessor&);
    void setTrack (int track);
    int getTrack() const { return track; }

    void paint (juce::Graphics&) override;
    void resized() override;

    std::function<void()> onVoiceForged;
    std::function<void()> onNameChanged;
    void refreshName();
    void updateWaveform (bool force = false);

private:
    KitForgeProcessor& proc;
    int track = 0;

    void showPresetMenu();
    void applyPreset (const kf::Presets::Preset& preset);
    void savePreset();
    void exportPad();

    Choice type { "Drum Type" };
    juce::TextButton forgeButton { "FORGE" }, previewButton { "PLAY" },
                     presetButton { "PRESETS" }, exportPadButton { "EXPORT PAD" };
    WaveformView waveform;
    kf::VoiceDef shownVoice {};
    kf::BusDef shownBus {};
    std::unique_ptr<juce::FileChooser> chooser;

    Choice wave { "Wave" };
    Knob pitch { "Pitch" }, pEnv { "Pitch Env" }, pDecay { "P.Decay" }, tDecay { "Decay" }, hold { "Hold" },
         glide { "Glide" }, tLevel { "Level" }, fm { "FM" }, ratio { "FM Ratio" }, fmDecay { "FM Decay" };

    Choice noiseType { "Noise" }, filterMode { "Filter" };
    Knob nLevel { "Level" }, nDecay { "Decay" }, cutoff { "Cutoff" }, reso { "Reso" }, burst { "Bursts" };

    Knob click { "Click" }, attack { "Attack" }, punch { "Punch", true }, punchTime { "Punch Time" },
         drive { "Drive" }, level { "Volume" },
         pan { "Pan", true }, choke { "Choke" }, lofiSend { "Lo-Fi Send" };
    juce::Label nameLabel;

    juce::Rectangle<int> toneArea, noiseArea, transArea, outArea, headerArea;
    juce::Random rng;
};

//==============================================================================
// The 12-bit "Vintage Sampler Bus" controls.
class BusPanel : public juce::Component
{
public:
    explicit BusPanel (KitForgeProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
    void resetMachineBox() { machine.box.setSelectedId (0, juce::dontSendNotification); }

private:
    KitForgeProcessor& proc;
    Choice machine { "Machine" };
    Knob drive { "Drive" }, bits { "Bits" }, rate { "Rate" }, pitch { "Pitch Hack" }, lpf { "Low-pass" },
         lowKeep { "Low Keep" }, punch { "Punch", true }, tail { "Tail", true }, vinyl { "Vinyl" };
    juce::Rectangle<int> infoArea;
};

// Holds the whole UI at its design size; the editor scales it.
class ContentView : public juce::Component
{
public:
    std::function<void (juce::Graphics&)> painter;
    void paint (juce::Graphics& g) override { if (painter) painter (g); }
};

//==============================================================================
class KitForgeEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit KitForgeEditor (KitForgeProcessor&);
    ~KitForgeEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress&) override;
    void visibilityChanged() override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void selectTrack (int t);
    void togglePlay();
    void showGenerateMenu();
    void stopButtonsStealingKeys (juce::Component&);
    void saveKit();
    void loadKitFromFile();
    void refreshKitName();
    void exportPack();
    void paintContent (juce::Graphics&);
    void layoutContent();

    static constexpr int baseWidth = 1320, baseHeight = 1040;
    ContentView content;

    KitForgeProcessor& proc;
    KitForgeLookAndFeel lnf;

    // Transport / global
    juce::TextButton playButton { "PLAY" };
    Knob tempo { "Tempo" }, swing { "Swing" }, length { "Length" }, master { "Master" };

    // Kit factory
    juce::ComboBox kitBox;
    juce::Label kitName;
    juce::TextButton forgeKitButton { "FORGE KIT" }, mutateButton { "MUTATE" },
                     saveButton { "SAVE" }, loadButton { "LOAD" }, exportButton { "EXPORT" };

    // Patterns
    std::array<juce::TextButton, kf::numPatterns> patternButtons;
    juce::TextButton copyButton { "COPY" }, pasteButton { "PASTE" },
                     clearButton { "CLEAR" }, generateButton { "GENERATE" };
    std::array<uint8_t, kf::numTracks * kf::numSteps> clipboard {};
    std::array<uint8_t, kf::numTracks * kf::numSteps> clipboardRatchets {};
    std::array<int8_t, kf::numTracks * kf::numSteps> clipboardPitches {};
    juce::TextButton rollButton { "ROLL" }, pitchButton { "PITCH" };
    int timerTicks = 0;
    bool hasClipboard = false;

    // Tracks
    std::array<std::unique_ptr<TrackHeader>, kf::numTracks> headers;
    std::array<juce::TextButton, kf::numTracks> muteButtons, soloButtons;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;
    SequencerGrid grid;

    VoicePanel voicePanel;
    BusPanel busPanel;

    std::unique_ptr<juce::FileChooser> chooser;
    juce::TooltipWindow tooltipWindow { this, 600 };
    juce::Random rng;

    juce::Rectangle<int> headerArea, factoryArea, patternArea, gridArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KitForgeEditor)
};
