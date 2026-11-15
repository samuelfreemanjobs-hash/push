# Ruflo autopilot launch (Windows PowerShell)

Run from **`C:\Dev\push`** (repo root). Requires: MSVC, CMake, JUCE configured with `-DDLZ_AUTORESEARCH=ON`, `targets/kick.wav`.

---

## Step A — Initialize & seed memory

```powershell
cd C:\Dev\push
npx ruflo@latest init
# Copy security template:
Copy-Item .claude\settings.json.example .claude\settings.json

npx ruflo@latest memory store --key "dsp_constraints" --value "SampleRate=44100/48000, MaxBlock=512, RealTimeSafe=True, MaxRenderMs=15.0, OriginalityFloor=0.08"
npx ruflo@latest memory store --key "sonic_targets" --value "Punchy sub-808 kick (48Hz fundamental, 55ms pitch drop), crisp trap snare, 6-oscillator metallic hat"
```

Verify CLI flags against [Ruflo docs](https://github.com/ruvnet/ruflo) — syntax may differ by version.

---

## Step B — One-time build

```powershell
cd disklordz-ralph\vst\starter-cpp
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DDLZ_AUTORESEARCH=ON
cmake --build build --config Release
```

```powershell
copy C:\path\to\kick.wav disklordz-ralph\vst\autoresearch-cpp\targets\kick.wav
python disklordz-ralph\vst\autoresearch-cpp\evaluate_vst.py
```

---

## Step C — Swarm loop

```powershell
cd C:\Dev\push
npx ruflo@latest daemon start
npx ruflo@latest swarm init --topology hierarchical --max-agents 4 --strategy specialized
```

Launch Claude Code (example):

```powershell
claude "Read CLAUDE.md and query Ruflo memory for dsp_constraints. Spawn DSP Engineer and QA Gatekeeper to run continuous optimization on disklordz-ralph/vst/autoresearch-cpp/Source/VoiceDSP.h using python disklordz-ralph/vst/autoresearch-cpp/evaluate_vst.py. Log results.tsv; store winning formulas in Ruflo memory."
```

Or use helper script:

```powershell
.\scripts\ruflo-autopilot-launch.ps1
```

---

## Monitor (second terminal)

```powershell
npx ruflo@latest swarm status
npx ruflo@latest memory search --query "saturation filter loss"
```

**Audition:** `disklordz-ralph\vst\autoresearch-cpp\outputs\rendered_kick.wav`  
**Kit:** `outputs\kits\kick_vel*.wav` after `export_kit_velocities.ps1`  
**Demo beat:** `factory\autoresearch\outputs\best_kit.wav` (Python track)

---

## Optional — OpenMontage handoff (marketing)

After pack export:

```powershell
python disklordz-ralph\factory\autoresearch\export_kit.py --handoff
# or: python disklordz-ralph\factory\autoresearch\publish_to_openmontage.py
```

Clone [OpenMontage](https://github.com/calesthio/OpenMontage) to `C:\Dev\marketing-studio`, then open that folder in Claude Code/Cursor.  
Prompts: `docs/marketing/openmontage-prompt-templates.md` · setup: `docs/dev/openmontage.md`