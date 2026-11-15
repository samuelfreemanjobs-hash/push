# vst_gui_builder workflow

1. Match `storefront/plugin-faceplate/` layout when changing pad macros or labels.
2. Edit `PluginEditor.cpp` / `ui/` — never block audio thread from UI timers.
3. Run faceplate static server per `openclaw/elite-frameworks/agent-desktop/README.md`.
4. Hand off to `vst-qa` for Waza + optional desktop smoke.