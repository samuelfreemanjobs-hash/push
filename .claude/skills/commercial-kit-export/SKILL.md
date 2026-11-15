---
name: commercial-kit-export
description: Export 24-bit WAV commercial pack from drum_engine.py and stage OpenMontage.
allowed-tools: Bash, Read
---

# Commercial kit export

```bash
cd disklordz-ralph/factory/autoresearch
python export_kit.py --sfz
# SKIP_OPENMONTAGE_PUBLISH=1 to skip marketing handoff
```

Outputs: `outputs/packs/*/manifest.json`  
OpenMontage: `publish_to_openmontage.py` → `assets/latest_release/`

Env: `PACK_NAME`, `OPENMONTAGE_ROOT`, `SKIP_OPENMONTAGE_PUBLISH`