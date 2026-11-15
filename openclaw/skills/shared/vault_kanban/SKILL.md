---
name: vault_kanban
description: Interpret vault kanban lanes and propose card updates without auto-committing.
user-invocable: true
---

Vault kanban in `agents/disklordz/kanban.yaml` is source of truth.

- Lanes: VST, DRUM, SAAS, FLOW, PLAN, etc.
- Propose column moves as YAML diff snippets for operator to apply in GitHub.
- WIP: respect solo founder WIP limit (Command Center default 3 in_progress).
- Do not delete cards; add `note` when delegating to another lane.