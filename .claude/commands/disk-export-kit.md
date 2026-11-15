---
description: Export commercial 24-bit kit and stage OpenMontage latest_release.
allowed-tools: Bash, Read, Agent
---

# /disk-export-kit

Orchestration: **Command → qa-validator (quick) → kit-producer**

1. Confirm latest autoresearch commit is a **win** (or user override).
2. `Agent(subagent_type="kit-producer", ...)` to run `export_kit.py`.
3. Verify `../marketing-studio/assets/latest_release/release_brief.json` exists (or report skip).
4. Point user to `docs/marketing/openmontage-prompt-templates.md` Template A.

Skill: `commercial-kit-export`