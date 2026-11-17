// Wraps the built-in synth as a VST3 so the hosting code can be tested.
#include "BuiltInSynth.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new forge::BuiltInSynth();
}
