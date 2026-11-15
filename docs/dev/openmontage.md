# OpenMontage — local marketing studio (opt-in)

**Stack policy:** Does not block Lite VST3 ship (`STACK-FROZEN.md`). Run on your Windows operator machine after kits or plugin screen captures exist.

[OpenMontage](https://openmontage.video/) is a local-first, agentic video stack (FFmpeg, Remotion, Hyperframes, optional cloud models) you drive from **Claude Code**, **Codex**, or **OpenMontage Studio**. For DiskLordz it covers beat visualizers, drum-pack Shorts, and VST launch trailers without watermark SaaS.

**Canonical source:** [github.com/calesthio/OpenMontage](https://github.com/calesthio/OpenMontage) (per [openmontage.video](https://openmontage.video/)).

---

## Layout (sibling repo)

Keep OpenMontage **outside** this git tree so upgrades do not pollute the audio repo:

```text
C:\Dev\
├── push\                          # this repo (DiskLordz)
└── marketing-studio\              # git clone OpenMontage
    └── assets\
        └── disklordz\               # staged by handoff scripts (gitignored in push)
```

---

## Windows setup

**Prerequisites:** Python 3.10+, Node.js 18+, FFmpeg on `PATH`, `make` (optional).

```powershell
cd C:\Dev
git clone https://github.com/calesthio/OpenMontage.git marketing-studio
cd marketing-studio
make setup
```

If `make` is missing:

```powershell
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -r requirements.txt
# Then install Node deps per OpenMontage README (npm install / package.json).
```

Open **`C:\Dev\marketing-studio`** in Cursor or Claude Code with the OpenMontage plugin/skills enabled. Point creative work at `assets/disklordz/<pack>/`.

---

## Handoff from autoresearch / Ruflo

After a commercial pack export:

```powershell
cd C:\Dev\push
python disklordz-ralph\factory\autoresearch\export_kit.py
.\scripts\handoff-pack-to-openmontage.ps1
```

Or one step:

```powershell
python disklordz-ralph\factory\autoresearch\export_kit.py --handoff
```

Env overrides:

| Variable | Default | Purpose |
|----------|---------|---------|
| `OPENMONTAGE_ROOT` | `../marketing-studio` (relative to repo root) | Clone location |
| `PACK_NAME` | `DiskLordz_Autoresearch_Vol1` | Used by `export_kit.py` |

The handoff copies:

- Latest `outputs/packs/<PACK>_<timestamp>/` (or the pack you pass)
- `factory/autoresearch/outputs/best_kit.wav` when present (loop demo)
- `manifest.json`, hero WAVs (Hard/Crack/Open layers), `AGENT_PROMPT.md`, `brief.json`

**Ruflo (`kit-producer`):** after `export_kit.py`, run the handoff script; optional **media-studio** agent loads `AGENT_PROMPT.md` in `marketing-studio` — human approves storyboard before final render (see `CLAUDE.md`).

---

## Quality gates (operator)

Before publishing MP4s, OpenMontage-style checks you should enforce:

1. **ffprobe** — peak &lt; −0.1 dBFS after your master; no clipped soundtrack
2. **Transient sync** — first kick within 200 ms of first visual hit (beat demos)
3. **Captions** — burnt-in text matches `manifest` claims (24-bit / 48 kHz, synthesis-only)
4. **Brand** — gunmetal `#121216`, amber accent; no purple AI-gradient slop (`PRODUCT.md`, `DESIGN.md`)

---

## Prompt templates

Copy-paste prompts for drum packs and VST launches:  
**`docs/marketing/openmontage-prompt-templates.md`**

---

## Related paths

| Asset | Path in push repo |
|-------|-------------------|
| Pack export | `disklordz-ralph/factory/autoresearch/export_kit.py` |
| Judge loop | `factory/autoresearch/outputs/best_kit.wav` |
| Storefront copy | `storefront/index.html`, `PRODUCT.md` |
| Screen captures | Operator records `starter-cpp` UI → drop in handoff `screens/` |