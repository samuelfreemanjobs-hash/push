#include "PluginEditor.h"

#if defined(DLZ_HAS_EMBEDDED_UI)
 #include "WebUIAssets.h"
#endif

namespace
{
#if defined(DLZ_HAS_EMBEDDED_UI)

std::vector<std::byte> toByteVector(const char* data, int size)
{
    const auto* ptr = reinterpret_cast<const std::byte*>(data);
    return { ptr, ptr + static_cast<size_t>(size) };
}

#endif
} // namespace

DrumKitAudioProcessorEditor::DrumKitAudioProcessorEditor(DrumKitAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webView(juce::WebBrowserComponent::Options{}
                  .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
                  .withWinWebView2Options(
                      juce::WebBrowserComponent::Options::WinWebView2{}.withUserDataFolder(
                          juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory)))
                  .withNativeIntegrationEnabled()
                  .withOptionsFrom(outputGainRelay)
                  .withResourceProvider([this](const auto& url) { return getResource(url); })),
      outputGainAttachment(*processorRef.apvts.getParameter("outputGain"), outputGainRelay, nullptr)
{
    addAndMakeVisible(webView);
#if defined(DLZ_HAS_EMBEDDED_UI)
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
#else
    webView.goToURL("about:blank");
#endif
    setSize(720, 460);
}

void DrumKitAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff141416));
}

void DrumKitAudioProcessorEditor::resized()
{
    webView.setBounds(getLocalBounds());
}

std::optional<juce::WebBrowserComponent::Resource> DrumKitAudioProcessorEditor::getResource(const juce::String& url)
{
#if ! defined(DLZ_HAS_EMBEDDED_UI)
    juce::ignoreUnused(url);
    return std::nullopt;
#else
    const juce::String path = url.endsWith("/") ? "index.html" : url.fromLastOccurrenceOf("/", false, false);

    if (path.equalsIgnoreCase("index.html") || path == "/")
    {
        return juce::WebBrowserComponent::Resource{
            toByteVector(WebUIAssets::index_html, WebUIAssets::index_htmlSize),
            "text/html",
        };
    }

    if (path.equalsIgnoreCase("app.js"))
    {
        return juce::WebBrowserComponent::Resource{
            toByteVector(WebUIAssets::app_js, WebUIAssets::app_jsSize),
            "text/javascript",
        };
    }

    return std::nullopt;
#endif
}