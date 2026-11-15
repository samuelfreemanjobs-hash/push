---
name: github_pr_review
description: Pre-merge PR review summary; never approve or merge automatically.
user-invocable: true
metadata:
  {"openclaw":{"requires":{"bins":["gh"],"anyBins":["gh"]}}}
---

Review workflow:

1. `gh pr view` / diff when available; else user-pasted diff.
2. Sections: **Verdict** (approve / request changes), **P0**, **Tests**, **Secrets scan**.
3. DiskLordz: tripwire, vault, VST CI impact.
4. Draft comment text only — operator posts to GitHub.
5. Hermes `pr-review/*.md` in hermes-out is supplementary context.