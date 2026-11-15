# Storefront & faceplate (Impeccable)

| Folder | Purpose |
|--------|---------|
| `disklordz-lite/` | Lite VST landing + 3-tier pricing (Week 3 launch) |
| `plugin-faceplate/` | WebView HTML mock — polish with `/impeccable` before any JUCE `WebBrowserComponent` work |

Preview locally:

```powershell
cd storefront\disklordz-lite
python -m http.server 8080 --bind 127.0.0.1
```

Then `/impeccable live` or `audit` on that folder.

Copy approved HTML into tripwire / Lemon Squeezy embed as needed.