# NEXUS launch runbook (on-demand Agency roster)

**Stack frozen** — these are `@`-tagged Cursor rules (`alwaysApply: false`), not new services. Roster: `msitarzewski/agency-agents` pattern, DiskLordz-tuned under `.cursor/rules/agency-*.mdc`.

## Roster

| Handle | Rule file |
|--------|-----------|
| @agency-juce-dsp | `agency-juce-dsp.mdc` |
| @agency-ui-designer | `agency-ui-designer.mdc` |
| @agency-reality-checker | `agency-reality-checker.mdc` |
| @agency-short-video | `agency-short-video.mdc` |
| @agency-reddit-ninja | `agency-reddit-ninja.mdc` |
| @agency-launch-strategist | `agency-launch-strategist.mdc` |

Always-on build rules stay: `rea-juce-superpowers.mdc`, `ponytail-juce.mdc` (full).

## Sprint prompt (paste in Cursor Agent / Ctrl+I)

```text
Run a coordinated 4-step sprint using our Agency roster and AGENTS.md rules:

1. @agency-juce-dsp: Using Superpowers TDD + Ponytail minimalism, verify external drag-and-drop WAV/MIDI in starter-cpp/Source/PluginEditor.cpp (PadGridComponent) so Baton Rouge 808s and Triggerman patterns drop into FL Studio's Playlist. Run ctest and pluginval on Windows when builds exist.

2. @agency-ui-designer: Review PluginEditor layout — pad drag affordance, Glide/808 Clip macros, dark hierarchy at 640×200.

3. @agency-reality-checker: Audit Release .vst3, Inno Lite installer path, FL-STUDIO-QA.md items, REA processBlock scope.

4. @agency-launch-strategist & @agency-short-video: Write docs/launch/changelog-v1.0.md, storefront-bullets.md, and shorts-drag-export-fl.md (45s cook-up script).
```

## Step 1 status (repo)

Drag-export **already implemented**:

- `starter-cpp/PadDragExport.h`, `MidiPatternDragExport.h`, `PadGridComponent.h`
- `starter-cpp/Source/PluginEditor.cpp` — hosts `PadGridComponent` on `DragAndDropContainer`
- `starter-cpp/Source/PluginProcessor.cpp` — `Trap808Voice` + pad previews

Next engineering work: FL smoke + `pluginval`, not duplicating editor drag code.

## Output folder

Save sprint artifacts under `docs/launch/` (git-tracked drafts; operator approves before publish).