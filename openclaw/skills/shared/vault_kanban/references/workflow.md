# vault_kanban workflow

## Source

`agents/disklordz/kanban.yaml` on default branch (read via git or local clone).

## Rules

- Propose diffs only; operator commits.
- WIP in_progress ≤ 3 unless kanban says otherwise.
- Lane codes: VST, DRUM, SAAS, FLOW, PLAN.
- Add `note` on delegate; never delete cards.