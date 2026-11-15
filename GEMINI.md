# Gemini CLI — DiskLordz push

@AGENTS.md

## Project focus

- **Repo:** Baton Rouge trap / NOLA bounce / Memphis phonk — `disklordz-ralph/vst/`, `Assets/`, `docs/`
- **Build lane (Cursor only):** C++ JUCE — do not edit `Source/` or `Modules/TrapCoreDSP/` from Gemini CLI unless the operator explicitly asks for a patch; prefer audits and asset/metadata tasks here.
- **Rules:** `disklordz-ralph/STACK-FROZEN.md`, `disklordz-ralph/vst/AGENTS.md`, Ponytail **full** on any code suggestions (`ponytail-juce.mdc`)

## Gemini CLI role

| Use | Avoid |
|-----|--------|
| WAV/MIDI catalog audit, rename plans, SEO copy, multimodal ref analysis | Replacing Cursor for VST compile, Catch2, REA |
| Read `kit_manifest.json`, `Assets/Releases/` | Heavy `npm install` or new agent frameworks |

## Key paths

- Masters: `disklordz-ralph/vst/Assets/RawMasters/`
- Factory imports (local WAV): `Assets/Imports/`
- Forge v4 source: `disklordz-ralph/factory/drum-forge-v4/`
- Releases: `Assets/Releases/`
- DSP (read-only audit): `disklordz-ralph/vst/Modules/TrapCoreDSP/`
- Plugin UI (already ships drag-export): `disklordz-ralph/vst/starter-cpp/Source/PluginEditor.cpp`

## Prompt templates

`docs/dev/gemini-cli.md` and `docs/launch/prompts/gemini-*.md`