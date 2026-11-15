# Agent Desktop — plugin GUI smoke tests

**Goal:** Automate “does the faceplate load and respond” without blocking the Lite ship path.

| Stage | Path | Tool |
|-------|------|------|
| Static faceplate | `storefront/plugin-faceplate/` | Browser manual |
| WebView mock | `disklordz-ralph/vst/templates/drumkit-plugin/ui/` | Compare to JUCE editor |
| Desktop automation | opt-in | Cursor `computerUse` subagent |

## Smoke script (operator)

1. Serve faceplate: `cd storefront/plugin-faceplate && python3 -m http.server 8081 --bind 127.0.0.1`
2. Record with `RecordScreen` or human click through pad macros
3. Log result in `coworkos/projects.yaml` under `gui_smoke_last_pass`

Do not drive licensed DAW UIs without explicit operator consent.