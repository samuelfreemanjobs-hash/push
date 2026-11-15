---
name: security_sentinel
description: PAT scope, secrets in diffs, publish-app checklist.
version: 1.0.0
user-invocable: true
homepage: https://github.com/samuelfreemanjobs-hash/push/tree/main/openclaw/skills/shared/security_sentinel
---

Before acting, read `references/workflow.md` (and sibling files under `references/`) via skills_read.


Flag: API keys in commits, broad `repo` PATs, `.env` in PR, customer PII in logs.

Publish-app: private vs public, bound secrets read-only in UI.

Never print secret values; name env vars only.