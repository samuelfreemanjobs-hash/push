# disk_ops_snapshot workflow

## Inputs

- `DLZ_REPO_ROOT` — git root containing `disklordz-command-center/`
- Optional: `~/.openclaw/hermes-out/engineering-report.json`

## Steps

1. Load `disklordz-command-center/public/ops-snapshot.json`.
2. Merge engineering report fields if file exists (do not overwrite snapshot keys).
3. Surface P0, blocked, tripwire step, VST fleet summary.
4. Suggest up to 3 reply prefixes only from allowed set.

## Outputs

Short brief for Telegram or morning sage session; no writes to repo.