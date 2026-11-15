# overnight_branch workflow

## Preconditions

- Goal file in workspace `OVERNIGHT.md` or kanban card linked
- No customer-facing deploys overnight

## Steps

1. Branch `cursor/overnight-<date>` off default
2. Implement only scoped items; commit atomically
3. Open draft PR with checklist from `OVERNIGHT.md`
4. Stop before merge; tag sage in morning brief