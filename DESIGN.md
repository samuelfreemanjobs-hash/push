# DiskLordz — design system (Impeccable)

## Mood

Dark **hardware rack** — matte charcoal, amber power indicators, soft red clip LED.

## Color

| Token | Hex | Use |
|-------|-----|-----|
| `--dl-bg` | `#0e0e12` | Plugin / page background |
| `--dl-surface` | `#1a1a22` | Panels, cards |
| `--dl-pad` | `#2d2d38` | Drum pads default |
| `--dl-pad-active` | `#f4b35b` | Pad hover / hit |
| `--dl-accent` | `#e8a83e` | CTAs, tier highlight |
| `--dl-clip-led` | `#ff5c4d` | Soft clip / warning |
| `--dl-text` | `#ececf0` | Primary text |
| `--dl-muted` | `#9a9cad` | Secondary labels |

**Banned:** `#6366f1` indigo hero gradients, full-viewport purple mesh, generic “AI” blobs.

## Typography

- **UI / marketing:** `system-ui`, `Segoe UI`, `Inter` (fallback only — prefer system stack on Windows)
- **Headings:** `letter-spacing: -0.02em`, weight 650–700
- **Plugin labels:** 11–12px uppercase tracking `0.06em` for macros

## Spacing

- Base unit **8px**; pad grid **16px** gutters
- Storefront section padding **48px** desktop / **24px** mobile

## Components

### Drum pad

- Min size **72×72**; radius **8px**; inset shadow `0 2px 0 #00000055`
- Label bottom-center; drag hint on hover tooltip

### Macro knob row

- Glide + 808 Clip only in Lite; full row in Core
- Slider track `#32323b`, fill `--dl-accent`

### Pricing table

- Lite column muted border; Core column `--dl-accent` top border 2px
- One primary CTA per tier (“Download Lite” / “Buy Core”)

### CTA buttons

- Primary: solid `--dl-accent`, text `#0e0e12`
- Secondary: outline `--dl-muted`, no ghost buttons on hero

## Plugin surfaces

| Surface | Path |
|---------|------|
| Native editor (ship) | `disklordz-ralph/vst/starter-cpp/Source/PluginEditor.*` |
| WebView mock (Impeccable) | `storefront/plugin-faceplate/index.html` |
| Lite landing | `storefront/index.html` (primary), `storefront/disklordz-lite/` |

## Motion

- Pad press: 80ms scale `0.97`
- No infinite pulse on CTAs; single entrance fade ≤ 300ms

## Breakpoints

- Storefront: **640px** stack pads mock; **960px** 3-col pricing