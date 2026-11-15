# strix_vst_audit workflow

## Triggers

- Pre-release gate for paid VST or commercial kit ZIP
- New license API, download URL signing, or user WAV import path
- Quarterly staging re-scan

## Steps

1. Read `audit-checklist.md` and confirm **staging** targets only.
2. Install Strix per `docs/dev/strix-opt-in.md`.
3. Run with `skills/vst_audio_security.md` as `--instruction-file`.
4. Execute `scripts/strix-pre-release.ps1 -Quick`, then `-Standard` on staging when live paths change.
5. Triage findings manually — no auto-merge of Strix patches.
6. After C++ fixes, run `pluginval` and re-scan before ship.

## Escalation

Pair with `security_sentinel` on critical auth or webhook findings. Stop ship until operator signs off.