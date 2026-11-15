#include "PluginEditor.h"

#if defined(DLZ_HAS_EMBEDDED_UI)
 #include "WebUIAssets.h"
#endif

namespace {

#if defined(DLZ_HAS_EMBEDDED_UI)

std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url)
{
    const juce::String path = url.endsWith("/") ? "index.html"
                                                : url.fromLastOccurrenceOf("/", false, false);

    auto makeResource = [](const void* data, int size, const juce::String& file) {
        juce::WebBrowserComponent::Resource resource;
        resource.data = juce::MemoryBlock(data, static_cast<size_t>(size));
        if (file.endsWithIgnoreCase(".css"))
            resource.mimeType = "text/css";
        else if (file.endsWithIgnoreCase(".js"))
            resource.mimeType = "text/javascript";
        else
            resource.mimeType = "text/html";
        return resource;
    };

    for (int i = 0; i < WebUIAssets::namedResourceListSize; ++i)
    {
        if (!path.equalsIgnoreCase(WebUIAssets::getNamedResourceOriginalFilename(i)))
            continue;
        int size = 0;
        const void* data = WebUIAssets::getNamedResource(i, size);
        return makeResource(data, size, path);
    }

    const auto symbol = path.replaceCharacter('.', '_').replaceCharacter('-', '_').toRawUTF8();
    int size = 0;
    if (const void* data = WebUIAssets::getNamedResource(symbol, size))
        return makeResource(data, size, path);

    return std::nullopt;
}

#endif

} // namespace

WebDrumAudioProcessorEditor::WebDrumAudioProcessorEditor(WebDrumAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p)
{
    setSize(560, 400);

    juce::WebBrowserComponent::Options options;
#if JUCE_WINDOWS
    options = options.withBackend(juce::WebBrowserComponent::Options::Backend::webview2);
#endif

    options = options.withNativeIntegrationEnabled(true);
    options = options.withEventListener(
        "triggerKick",
        [this](const juce::var&) {
            processorRef.triggerKickFromUi();
        });

#if defined(DLZ_HAS_EMBEDDED_UI)
    options = options.withResourceProvider(
        [](const juce::String& url) { return getResource(url); },
        juce::URL("juce://local/").getOrigin());
#endif

    webView = std::make_unique<juce::WebBrowserComponent>(options);
    addAndMakeVisible(*webView);

#if defined(DLZ_HAS_EMBEDDED_UI)
    webView->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
#else
    webView->goToURL("about:blank");
#endif
}

WebDrumAudioProcessorEditor::~WebDrumAudioProcessorEditor()
{
    webView = nullptr;
}

void WebDrumAudioProcessorEditor::resized()
{
    if (webView != nullptr)
        webView->setBounds(getLocalBounds());
}