# Autoresearch — C++ / JUCE workflow

**Implemented:** `disklordz-ralph/vst/autoresearch-cpp/` — `VoiceDSPSandbox.h`, `evaluate_vst.py`, `HeadlessMain.cpp`, build via `-DDLZ_AUTORESEARCH=ON`.

Use when Python winners need **real-time** validation on `TrapCoreDSP` or `ThreeLayerDrum`.

## Pattern (same three layers)

| Layer | C++ analogue |
|-------|----------------|
| Sandbox | One translation unit, e.g. `Trap808VoiceAutoresearch.cpp` or `#ifdef DLZ_AUTORESEARCH_SANDBOX` block in a fork branch |
| Judge | `luger-drum-lab/Tools/render_milestone1.cpp` extended, or `vst/Tests/AutoresearchRenderTest.cpp` — **not** edited by agents |
| Playbook | Same `factory/autoresearch/program.md`; add "sandbox path" line |

## Harness sketch

1. **Offline render executable** links sandbox voice only.
2. Renders N velocities × M notes to float buffers (no JUCE editor).
3. Python `evaluate.py` reads WAVs **or** share metric code via `evaluate_cpp` (future).

## RT guardrails (judge must fail)

- Wall time for N renders > budget
- Any sample non-finite
- Peak > 0 dBFS target

## Multi-voice kit export (phase 2b)

After kit autoresearch stabilizes `drum_engine.py`:

1. Map `synthesize_kick/snare/hat` constants into three offline render CLIs or one `KitVoiceRender.cpp`.
2. Run the **same** `evaluate.py` metrics on emitted WAVs (subprocess — judge unchanged).
3. Load winners into `Factory808` / pad map in `starter-cpp`.

## Merge policy

1. Python autoresearch lowers profile/STFT loss.
2. Operator ports constants to `Trap808Voice` / `SoftClipper`.
3. Catch2 + `pluginval` on Windows before merge to `starter-cpp`.

Do **not** let an agent edit `PluginProcessor::processBlock` in autoresearch loops — offline render only until REA sign-off.