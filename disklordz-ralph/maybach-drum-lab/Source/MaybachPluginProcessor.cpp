#include <JuceHeader.h>
#include "MaybachIdentity.h"
#include <algorithm>
#include <array>
#include <memory>

class MaybachDrumLabProcessor final : public juce::AudioProcessor
{
public:
    MaybachDrumLabProcessor()
        : juce::AudioProcessor(juce::AudioProcessor::BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          parameters(*this, nullptr, "PARAMS", makeLayout())
    {
    }

    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
        p.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { "identity", 1 }, "Luxury DNA",
            juce::StringArray {
                "Maybach Soul",
                "Cinematic Empire",
                "Luxury Trap",
                "Yacht Club",
                "Teflon Drums",
                "Royal Orchestra",
            },
            0));
        p.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { "morph", 1 }, "Acoustic Morph",
            juce::StringArray { "Live", "Hybrid", "Trap" },
            1));
        p.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "level", 1 }, "Level", juce::NormalisableRange<float>(0.0f, 1.0f), 0.58f));
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
    const juce::String getName() const override { return "Maybach Drum Lab PRO"; }
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

    void trigger(int note, float velocity)
    {
        if (note < 36 || note > 38)
            return;
        const auto type = note == 36 ? dl::Voice::Kick : note == 37 ? dl::Voice::Snare : dl::Voice::Clap;

        auto slot = std::find_if(voices.begin(), voices.end(), [](auto& v) { return !v.drum.isActive(); });
        if (slot == voices.end())
            slot = std::min_element(voices.begin(), voices.end(), [](auto& a, auto& b) { return a.age < b.age; });

        const auto idIndex = juce::jlimit(0, 5, (int) parameters.getRawParameterValue("identity")->load());
        const auto morphIndex = juce::jlimit(0, 2, (int) parameters.getRawParameterValue("morph")->load());
        const auto identity = static_cast<dl::maybach::Identity>(idIndex);
        const auto morph = static_cast<dl::maybach::AcousticMorph>(morphIndex);
        const auto design = dl::maybach::designForMaybach(type, identity, morph, 30260115u + unsigned(++serial));
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
            {
                if (!v.drum.isActive())
                    continue;
                mono += v.drum.next() * v.velocity;
            }
            mono *= gain;
            for (int ch = 0; ch < channels; ++ch)
                b.setSample(ch, i, mono);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaybachDrumLabProcessor)
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MaybachDrumLabProcessor();
}