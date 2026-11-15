# Relay code output (H → OC)

Hermes writes:

```markdown
---
relay: code
agent: codex|hermes-review
repo: owner/name
branch: overnight/...
pr: optional url
---

## Summary
(5 lines)

## Diff scope
(files)

## OC actions
- REVIEW: ...
- DEV: merge? no — operator
```

OpenClaw **sage** or **dev** reads this path when user asks or morning cron includes `hermes-out/relay/`.