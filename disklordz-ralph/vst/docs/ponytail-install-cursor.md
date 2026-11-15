# Ponytail on Cursor (Windows)

## Rules (recommended)

This repo ships a JUCE-specific merge: **`.cursor/rules/ponytail-juce.mdc`** (use with `rea-juce-superpowers.mdc`).

Optional upstream copy from [DietrichGebert/ponytail](https://github.com/DietrichGebert/ponytail):

- `.cursor/rules/ponytail.mdc` — generic ladder (do not duplicate if `ponytail-juce.mdc` is enough)

## Optional MCP

In `~/.cursor/mcp.json`, add the package from [INSTALL.md](https://github.com/DietrichGebert/ponytail/blob/main/INSTALL.md) (`@dietrichgebert/ponytail`). Cursor hooks may only expose level switching; audits still work via Agent prompts in `ponytail-juce.mdc`.

## Intensity

| Mode | When |
|------|------|
| **full** | Daily TrapCoreDSP / starter-cpp |
| **lite** | Custom pad grid / LookAndFeel |
| **ultra** | Deleting wrapper classes before v1.0 |

## High-value prompts

See checklist at bottom of `.cursor/rules/ponytail-juce.mdc`.