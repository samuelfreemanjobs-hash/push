# DLZ-VST-007 — wire drag-export into PluginEditor

Copy headers from this folder into `C:\Dev\DetroitPhonkTrapVST3\Source\export\` (or `Source/ui/`).

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

- `getPadBuffer` / `getPadSampleRate` — last rendered one-shot or loop per pad (or factory sample).
- `getPadMidiPattern` — optional `MidiMessageSequence` for Triggerman rolls (Shift+click drag).
- `loadUserWavOntoPad` — decode WAV, swap atomic pointer for sampler voice (see `TRAP-PHONK-ARCHITECTURE.md`).

## 3. FL Studio QA

After build: drag pad WAV onto Playlist; Shift+drag MIDI pattern. Check `docs/FL-STUDIO-QA.md`.

## 4. Kit folder export

```cpp
KitExporter::PadSample s{ "808", processorRef.getPadBuffer(0), processorRef.getPadSampleRate(0) };
KitExporter::exportPadsToFolder({ s, ... }, KitExporter::defaultExportDir());
```

## 5. Out of scope here

- Multi-out (`DLZ-VST-009`), `.blob` encryption, AutoGPT storefront publish (see `openclaw/autogpt/`).