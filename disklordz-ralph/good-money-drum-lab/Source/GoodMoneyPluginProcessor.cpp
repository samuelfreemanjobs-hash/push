#include <JuceHeader.h>
#include "GoodMoneyIdentity.h"
#include <algorithm>
#include <array>
#include <memory>

class GoodMoneyDrumLabProcessor final : public juce::AudioProcessor
{
public:
    GoodMoneyDrumLabProcessor()
        : juce::AudioProcessor(juce::AudioProcessor::BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          parameters(*this, nullptr, "PARAMS", makeLayout())
    {
    }

    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
        p.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ "identity", 1 },
            "Good Money DNA",
            juce::StringArray{
                "Good Money",
                "Midnight Player",
                "Orange Grove",
                "Luxury Soul",
                "Great Escape",
                "Bay Area Funk",
                "Night Drive",
                "Premium Street",
            },
            0));
        p.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ "mode", 1 },
            "Player Mode",
            juce::StringArray{ "Player Bounce", "Luxury Soul" },
            0));
        p.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ "level", 1 }, "Level", juce::NormalisableRange<float>(0.0f, 1.0f), 0.55f));
        return { p.begin(), p.end() };
    }

    void prepareToPlay(double sampleRate, int) override
    {
        fs = sampleRate;
        for (auto& v : voices)
            v.drum.prepare(fs);
    }

    void releaseResources() override {}

    bool isBusesLayoutSupported(const BusesLayout& b) const override
    {
        return b.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
            || b.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override
    {
        juce::ScopedNoDenormals noDenormals;
        buffer.clear();
        int cursor = 0;
        for (const auto e : midi)
        {
            const int next = juce::jlimit(0, buffer.getNumSamples(), e.samplePosition);
            render(buffer, cursor, next);
            const auto msg = e.getMessage();
            if (msg.isNoteOn())
                trigger(msg.getNoteNumber(), msg.getFloatVelocity());
            if (msg.isAllSoundOff())
                for (auto& v : voices)
                    v.drum.stop();
            cursor = next;
        }
        render(buffer, cursor, buffer.getNumSamples());
    }

    juce::AudioProcessorEditor* createEditor() override { return new juce::GenericAudioProcessorEditor(*this); }
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Good Money Drum Lab PRO"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& block) override
    {
        if (auto xml = parameters.copyState().createXml())
            copyXmlToBinary(*xml, block);
    }

    void setStateInformation(const void* data, int size) override
    {
        if (data == nullptr || size <= 0 || size > 1024 * 1024)
            return;
        if (auto xml = getXmlFromBinary(data, size))
            if (xml->hasTagName(parameters.state.getType()))
                parameters.replaceState(juce::ValueTree::fromXml(*xml));
    }

private:
    struct Playing
    {
        dl::ThreeLayerDrum drum;
        float velocity = 0.0f;
        uint64_t age = 0;
    };

    static constexpr int voicesCount = 32;
    std::array<Playing, voicesCount> voices;
    juce::AudioProcessorValueTreeState parameters;
    double fs = 48000.0;
    uint64_t serial = 0;

    static dl::Voice voiceFromMidi(int note)
    {
        if (note == 36)
            return dl::Voice::Kick;
        if (note == 35 || note == 37)
            return dl::Voice::Bass808;
        if (note == 38)
            return dl::Voice::Snare;
        return dl::Voice::Perc;
    }

    void trigger(int note, float velocity)
    {
        const auto type = voiceFromMidi(note);
        if (type == dl::Voice::Perc)
            return;

        auto slot = std::find_if(voices.begin(), voices.end(), [](auto& v) { return !v.drum.isActive(); });
        if (slot == voices.end())
            slot = std::min_element(voices.begin(), voices.end(), [](auto& a, auto& b) { return a.age < b.age; });

        const auto idIndex = juce::jlimit(0, 7, (int) parameters.getRawParameterValue("identity")->load());
        const auto modeIndex = juce::jlimit(0, 1, (int) parameters.getRawParameterValue("mode")->load());
        const auto identity = static_cast<dl::goodmoney::Identity>(idIndex);
        const auto mode = static_cast<dl::goodmoney::PlayerMode>(modeIndex);
        const auto design =
            dl::goodmoney::designForGoodMoney(type, identity, mode, 4'061'115u + static_cast<uint32_t>(++serial));

        slot->drum.start(design);
        slot->velocity = velocity;
        slot->age = ++serial;
    }

    void render(juce::AudioBuffer<float>& buffer, int start, int end)
    {
        const float level = parameters.getRawParameterValue("level")->load();
        for (int i = start; i < end; ++i)
        {
            float sample = 0.0f;
            for (auto& v : voices)
            {
                if (!v.drum.isActive())
                    continue;
                sample += v.drum.next() * v.velocity * level;
            }
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.addSample(ch, i, sample);
        }
    }
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GoodMoneyDrumLabProcessor();
}