#include "PluginHost.h"

namespace forge
{
PluginHost::PluginHost()
{
    formatManager.addDefaultFormats();
}

juce::AudioPluginFormat* PluginHost::vst3() const
{
    for (auto* f : formatManager.getFormats())
        if (f->getName() == "VST3")
            return f;
    return nullptr;
}

juce::StringArray PluginHost::findInstalledVST3() const
{
    juce::StringArray result;
    if (auto* f = vst3())
    {
        result = f->searchPathsForPlugins (f->getDefaultLocationsToSearch(), true, false);
        result.sortNatural();
    }
    return result;
}

std::unique_ptr<juce::AudioPluginInstance> PluginHost::load (const juce::String& pathOrId, double sampleRate,
                                                             int blockSize, juce::String& error)
{
    auto* format = vst3();
    if (format == nullptr)
    {
        error = "this build has no VST3 hosting support";
        return {};
    }

    juce::File file (juce::File::isAbsolutePath (pathOrId) ? juce::File (pathOrId)
                                                            : juce::File::getCurrentWorkingDirectory().getChildFile (pathOrId));
    if (! file.exists())
    {
        error = "plugin not found: " + file.getFullPathName();
        return {};
    }

    juce::OwnedArray<juce::PluginDescription> types;
    format->findAllTypesForFile (types, file.getFullPathName());

    if (types.isEmpty())
    {
        error = "no VST3 plugin could be read from " + file.getFullPathName();
        return {};
    }

    // A bundle can contain several plugins: prefer an instrument.
    auto* chosen = types.getFirst();
    for (auto* t : types)
        if (t->isInstrument) { chosen = t; break; }

    juce::String err;
    auto instance = formatManager.createPluginInstance (*chosen, sampleRate, blockSize, err);
    if (instance == nullptr)
        error = "failed to load " + chosen->name + ": " + err;

    return instance;
}

SoundState captureState (juce::AudioProcessor& p, const juce::String& pluginPath)
{
    SoundState s;
    s.pluginPath = pluginPath;
    s.pluginName = p.getName();
    p.getStateInformation (s.state);
    return s;
}

bool SoundState::save (const juce::File& f) const
{
    juce::XmlElement xml ("MPCSampleForgeSound");
    xml.setAttribute ("version", 1);
    xml.setAttribute ("pluginPath", pluginPath);
    xml.setAttribute ("pluginName", pluginName);
    xml.createNewChildElement ("State")->addTextElement (state.toBase64Encoding());
    return xml.writeTo (f);
}

bool SoundState::load (const juce::File& f, juce::String& error)
{
    auto xml = juce::XmlDocument::parse (f);
    if (xml == nullptr || ! xml->hasTagName ("MPCSampleForgeSound"))
    {
        error = "not a sound file: " + f.getFullPathName();
        return false;
    }

    pluginPath = xml->getStringAttribute ("pluginPath");
    pluginName = xml->getStringAttribute ("pluginName");
    state.reset();

    if (auto* st = xml->getChildByName ("State"))
        if (! state.fromBase64Encoding (st->getAllSubText().trim()))
        {
            error = "corrupt state in " + f.getFullPathName();
            return false;
        }

    return true;
}
}
