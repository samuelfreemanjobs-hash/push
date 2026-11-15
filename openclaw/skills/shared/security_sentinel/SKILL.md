---
name: security_sentinel
description: PAT scope, secrets in diffs, publish-app checklist.
user-invocable: true
---

Flag: API keys in commits, broad `repo` PATs, `.env` in PR, customer PII in logs.

Publish-app: private vs public, bound secrets read-only in UI.

Never print secret values; name env vars only.