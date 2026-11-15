# Gemini CLI — kit folder audit (copy prompt body)

Run from `C:\Dev\push` after `gemini` is installed. Replace `RELEASE_DIR`.

```text
Scan Assets/Releases/Louisiana_Bounce_Vol1/ (or RELEASE_DIR).

1. List every .wav and .mid with path.
2. Flag files that do NOT match [Style]_[Type]_[Key]_[Index].wav (808/Boo-Bass must include key like _C1 or _F#1).
3. Flag duplicate basenames or likely duplicate audio (same length/size clusters).
4. Cross-check kit_manifest.json if present — sample_count vs files on disk.
5. Write docs/launch/tracklist.md: table of Track Name | Type | Key | Filename | Duration hint.
6. Do not rename files automatically — output a rename-plan.csv only.

Respect GEMINI.md: no C++ edits this turn.
```