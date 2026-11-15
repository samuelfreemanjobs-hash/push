---
name: strix_vst_audit
description: Operator playbook for Strix red-team scans on DiskLordz VST license APIs, kit download URLs, and JUCE parsers.
version: 1.0.0
user-invocable: true
homepage: https://github.com/samuelfreemanjobs-hash/push/tree/main/openclaw/skills/shared/strix_vst_audit
---

# Strix VST & storefront audit

Before acting, read `references/audit-checklist.md` in this skill directory.

## When to use

- Before **paid** VST or **commercial kit ZIP** launch  
- After adding **license activation**, **webhook fulfillment**, or **user WAV import**  
- Quarterly staging re-scan

## Operator workflow

1. Confirm **staging** URLs and test payment keys — never production customer data.  
2. Install Strix per `docs/dev/strix-opt-in.md` (Docker + pipx).  
3. Pass instruction file: `skills/vst_audio_security.md` (`--instruction-file`).
4. Optional extended checklist: `docs/security/strix-vst-audit-playbook.md`.  
5. Run `scripts/strix-pre-release.ps1 -Quick`, then staging with `-Standard` if live.
6. Triage findings — **do not auto-merge** Strix patches; run `pluginval` after C++ fixes.

## Targets (repo)

| Area | Path |
|------|------|
| Ship plugin | `disklordz-ralph/vst/starter-cpp/Source/` |
| Storefront | `storefront/` |
| Metrics API | `disklordz-ralph/business/openbb/` |

## Out of scope

- Cracking or distributing binaries  
- Live customer PII extraction  
- Changing prices/checkout without human approval

See also: `security_sentinel`, `docs/dev/vst-commercial-gates.md`.