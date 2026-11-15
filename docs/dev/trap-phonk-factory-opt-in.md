# Trap / Phonk nightly factory (opt-in)

## Bot roster

`business/trap-phonk-factory/hermes_roster.yaml` — maps to three roles:

| Bot | In-repo implementation |
|-----|-------------------------|
| MemphisSoundArchitect | `run_factory.py` stub copies `audio-factory/raw_audio` until GPU infer licensed |
| KitMasteringEngineer | `kit_mastering.py` (pedalboard + librosa) |
| PromoReelDirector | `promo_meta.json` stub — FFmpeg operator-run |

## Nightly run

```powershell
$env:DLZ_REPO_ROOT = "C:\Dev\push"
pip install -r disklordz-ralph\business\trap-phonk-factory\requirements.txt
python disklordz-ralph\business\trap-phonk-factory\run_factory.py
```

Outputs: `business/trap-phonk-factory/outputs/YYYYMMDD/`

## Kit mastering only

```powershell
python kit_mastering.py --in-dir ..\..\factory\audio-factory\raw_audio `
  --out-dir .\mastered --style Memphis --bpm 145 --zip .\lead_magnet.zip
```

## Hermes / DeepSeek

When harness is installed, point `workspace` at `audio-factory` and replace `run_memphis_architect_stub` with roster tools.

Human gate: no Shopify upload from cron without approval.