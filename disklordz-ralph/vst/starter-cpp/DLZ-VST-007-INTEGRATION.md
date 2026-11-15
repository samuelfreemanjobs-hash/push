# DLZ-VST-007 — wire drag-export into PluginEditor

Copy headers from this folder into `C:\Dev\DetroitPhonkTrapVST3\Source\`.

**Fast path:** use `starter-cpp/Source/` as your plugin `Source/` (CMake in `starter-cpp/CMakeLists.txt` expects this layout) and keep pad headers in `starter-cpp/`.

## 1. Editor inherits drag host

```cpp
class DetroitPhonkTrapAudioProcessorEditor : public juce::AudioProcessorEditor,
                                             public juce::DragAndDropContainer
{
public:
    explicit DetroitPhonkTrapAudioProcessorEditor(DetroitPhonkTrapAudioProcessor& p)
        : AudioProcessorEditor(&p), processorRef(p)
    {
        padGrid = std::make_unique<PadGridComponent>(*this);
        addAndMakeVisible(*padGrid);
        padGrid->setPadCount(5);
        padGrid->setGetPadAudio([this](int i) { return processorRef.getPadBuffer(i); });
        padGrid->setGetPadRate([this](int i) { return processorRef.getPadSampleRate(i); });
        padGrid->setGetPattern([this](int i) { return processorRef.getPadMidiPattern(i); });
        padGrid->setGetHostTempo([this]() { return processorRef.getLastKnownBpm(); });
        padGrid->setLoadPadFromFile([this](int i, const juce::File& f) {
            processorRef.loadUserWavOntoPad(i, f); // message thread only
        });
        setSize(640, 120);
    }

    void resized() override { padGrid->setBounds(getLocalBounds()); }

private:
    DetroitPhonkTrapAudioProcessor& processorRef;
    std::unique_ptr<PadGridComponent> padGrid;
};
```

## 2. Processor hooks (message thread)

Embed `PadSampleStore padStore` on the processor:

```cpp
juce::AudioBuffer<float>* getPadBuffer(int i) { return padStore.getPadBufferForDrag(i); }
double getPadSampleRate(int i) const { return padStore.getPadSampleRate(i); }
juce::MidiMessageSequence getPadMidiPattern(int i) const { return padStore.getPadMidiPattern(i); }
void loadUserWavOntoPad(int i, const juce::File& f) { padStore.loadUserWavOntoPad(i, f); }
double getLastKnownBpm() const { return padStore.getLastKnownBpm(); }
```

In `processBlock`, call `padStore.setLastKnownBpm` from play head when available. Voices should read `padStore.getPublishedPad(i)` on the audio thread (never `loadUserWavOntoPad` there).

## 3. FL Studio QA

After build: drag pad WAV onto Playlist; Shift+drag MIDI pattern. Check `docs/FL-STUDIO-QA.md`.

## 4. Kit folder export

```cpp
KitExporter::PadSample s{ "808", processorRef.getPadBuffer(0), processorRef.getPadSampleRate(0) };
KitExporter::exportPadsToFolder({ s, ... }, KitExporter::defaultExportDir());
```

## 5. FL drag checklist (Week 2, Days 11–12)

- Click pad → drop WAV on **Playlist** (not only Channel Rack)
- Shift+click → drop `.mid` on Playlist
- Drag external WAV onto pad → reload without reopening editor
- Re-scan VST3 after installer: FL **Manage plugins → Find plugins**

## 6. Out of scope here

- Multi-out (`DLZ-VST-009`), `.blob` encryption, AutoGPT storefront publish (see `openclaw/autogpt/`).