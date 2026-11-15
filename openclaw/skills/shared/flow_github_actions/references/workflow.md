# flow_github_actions workflow

## Scope

Backup, VST CI, ops snapshot sync workflows in repo `.github/workflows/`.

## Steps

1. Read workflow YAML; summarize triggers and secrets needed (names only).
2. Propose dispatch or fix PR; never echo secret values.
3. After failure: link run URL and last error step name.