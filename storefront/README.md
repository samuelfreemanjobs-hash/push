# Storefront & faceplate (Impeccable)

| Path | Purpose |
|------|---------|
| **`index.html`** | Primary Tailwind landing — Web Audio audition, tiers, checkout placeholders |
| **`interactive-lab/`** | GPU-style bass-reactive pad grid — keys 1–4, CTA to tiers |
| `disklordz-lite/` | Token-based CSS variant (same funnel, smaller surface) |
| `plugin-faceplate/` | WebView mock — `/impeccable distill` before optional `WebBrowserComponent` |

Preview:

```powershell
cd storefront
python -m http.server 8080 --bind 127.0.0.1
# open http://127.0.0.1:8080/index.html
```

Run `/impeccable craft` → `polish` → `audit` on `storefront/index.html`.

Copy approved HTML into tripwire / Lemon Squeezy embed as needed.