# ClawHub publish prep — DiskLordz founder skills

Published skills are **MIT-0** per [ClawHub skill format](https://docs.openclaw.ai/clawhub/skill-format). No secrets in bundles.

## Prereqs

```bash
npm i -g clawhub   # or pnpm add -g clawhub
clawhub login      # GitHub account (age gate)
openclaw skills verify @owner/skill  # after first publish
```

## Verify locally (this repo)

```bash
cd openclaw/skills
npm install
node scripts/verify-skills.mjs
./scripts/publish-prep.sh
```

## Publish one skill (dry-run first)

```bash
cd openclaw/skills/shared/disk_ops_snapshot
clawhub skill publish . \
  --slug disklordz-disk-ops-snapshot \
  --name "DiskLordz Ops Snapshot" \
  --version 1.0.0 \
  --changelog "Initial solo-founder ops brief skill." \
  --tags latest,disklordz,solo-founder \
  --dry-run
```

Remove `--dry-run` to upload. Repeat per skill; slugs in `manifest.yaml`.

## Publish all (interactive)

```bash
./scripts/publish-prep.sh --publish
```

Requires `clawhub` logged in. Uses semver `1.0.0` from manifest.

## Install for users

```bash
openclaw skills install @YOUR_HANDLE/disklordz-disk-ops-snapshot
```

## Security

- No PATs, customer data, or private vault URLs in `references/`
- Declare `requires.env` / `envVars` in frontmatter when skills need tokens
- Run ClawHub scan; fix metadata mismatches before promoting `latest`

## Collection

Install full allowlist from `manifest.yaml` into workspace:

```bash
# after each skill is published under your handle
openclaw skills install @YOUR_HANDLE/disklordz-delegate-subagent
# … see manifest.yaml
```