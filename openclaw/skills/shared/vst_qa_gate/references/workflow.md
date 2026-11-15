# vst_qa_gate workflow

1. `bash openclaw/elite-frameworks/waza/evals/run_all.sh` — must exit 0.
2. EGM duel on all paths in `openclaw/elite-frameworks/automation/specs.manifest.yaml`.
3. pluginval artifact required for paid VST claims (`openduet/approval_gates.yaml`).
4. Escalate security to `$security_sentinel` / Strix for storefront auth changes.