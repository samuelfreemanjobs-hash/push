#include <JuceHeader.h>
#include "LugerIdentity.h"
#include <algorithm>
#include <array>
#include <memory>

// Milestone 1 JUCE shell — opt-in via -DDISKLORDZ_LUGER_BUILD_JUCE=ON
class LugerDrumLabProcessor final : public juce::AudioProcessor
{
public:
    LugerDrumLabProcessor()
        : juce::AudioProcessor(juce::AudioProcessor::BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          parameters(*this, nullptr, "PARAMS", makeLayout())
    {
    }

    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
        p.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { "identity", 1 }, "Luger DNA",
            juce::StringArray { "Brick Squad", "Orchestral Warfare", "Trap Anthem", "Dark Southern", "Arena Trap" }, 0));
        p.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "level", 1 }, "Level", juce::NormalisableRange<float>(0.0f, 1.0f), 0.62f));
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

    juce::AudioProcessorEditor* createEditor() override
    {
        return new juce::GenericAudioProcessorEditor(*this);
    }

    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "DiskLordz Luger Drum Lab"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }
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

    void trigger(int note, float velocity)
    {
        if (note < 36 || note > 42)
            return;
        const auto type = static_cast<dl::Voice>(note - 36);
        auto slot = std::find_if(voices.begin(), voices.end(), [](auto& v) { return !v.drum.isActive(); });
        if (slot == voices.end())
            slot = std::min_element(voices.begin(), voices.end(), [](auto& a, auto& b) { return a.age < b.age; });

        const auto idIndex = juce::jlimit(0, 4, (int) parameters.getRawParameterValue("identity")->load());
        const auto identity = static_cast<dl::luger::Identity>(idIndex);
        const auto design = dl::luger::designForLuger(type, identity, 261115u + unsigned(++serial));
        slot->drum.start(design);
        slot->velocity = juce::jlimit(0.0f, 1.0f, velocity);
        slot->age = serial;
    }

    void render(juce::AudioBuffer<float>& b, int from, int to)
    {
        const float gain = parameters.getRawParameterValue("level")->load();
        const int channels = b.getNumChannels();
        for (int i = from; i < to; ++i)
        {
            float mono = 0.0f;
            for (auto& v : voices)
                if (v.drum.isActive())
                    mono += v.drum.next() * v.velocity;
            const float result = std::tanh(mono * 0.7f) * gain;
            for (int c = 0; c < channels; ++c)
                b.setSample(c, i, result);
        }
    }
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LugerDrumLabProcessor();
}