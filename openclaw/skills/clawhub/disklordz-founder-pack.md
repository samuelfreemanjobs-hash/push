# DiskLordz founder pack (ClawHub collection)

Seventeen OpenClaw skills for solo-founder ops: vault kanban, tripwire drums, gates, KPI, delegation.

## Install order (suggested)

1. `disklordz-delegate-subagent`
2. `disklordz-disk-ops-snapshot`
3. `disklordz-vault-kanban`
4. Remaining skills per agent allowlist in `openclaw.skills.json5.example`

## Source

Monorepo path: `openclaw/skills/shared/*`

Verify before publish:

```bash
cd openclaw/skills && npm install && npm run verify
```

Replace `YOUR_CLAWHUB_HANDLE` in `clawhub/manifest.yaml` before bulk publish.