// Wraps the built-in FM keys as a VST3 so the hosting code can be tested.
#include "BuiltInKeys.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new forge::BuiltInKeys();
}
