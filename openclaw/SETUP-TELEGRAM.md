# OpenClaw + Telegram for DiskLordz

**One bot → `sage` only.** Hermes never gets Telegram; it writes `~/.openclaw/hermes-out/`.

## Prerequisites

| Item | Notes |
|------|--------|
| **Node.js** | **24.16+** or **26.1+** ([OpenClaw requirement](https://docs.openclaw.ai/start/getting-started)) |
| **Clone** | This repo (`push`) — set `DLZ_REPO_ROOT` |
| **Gemini** | [Google AI Studio](https://aistudio.google.com/apikey) free API key |
| **Telegram** | [@BotFather](https://t.me/BotFather) → `/newbot` → bot token |
| **Host** | Windows PC, Oracle Free ARM, or home server (gateway must stay online) |

## Automated setup

### Windows (PowerShell)

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
cd C:\Dev\push
$env:DLZ_REPO_ROOT = (Get-Location).Path
.\openclaw\scripts\setup-disklordz-telegram.ps1
```

### Linux / VPS / WSL

```bash
export DLZ_REPO_ROOT=~/push
bash openclaw/scripts/setup-disklordz-telegram.sh
```

With token already exported:

```bash
export TELEGRAM_BOT_TOKEN='123456789:ABCDEF…'
export DLZ_REPO_ROOT=~/push
bash openclaw/scripts/setup-disklordz-telegram.sh
```

## Manual path (if you prefer)

### 1. Install OpenClaw

```bash
curl -fsSL https://openclaw.ai/install.sh | bash
# Windows: iwr -useb https://openclaw.ai/install.ps1 | iex
openclaw onboard --classic   # Google → Gemini 2.0 Flash
```

### 2. DiskLordz workspaces + skills

```bash
bash openclaw/disklordz-picks/scripts/install-pick-workspaces.sh
bash disklordz-agent-stack/deploy/bootstrap-free.sh
node openclaw/scripts/merge-disklordz-config.mjs
```

Copy fields from `~/.openclaw/disklordz.merged.json5` into `~/.openclaw/openclaw.json` (agents, bindings, skills, automation). Or use Control UI.

### 3. Telegram token

```bash
openclaw channels add --channel telegram --token "<BOTFATHER_TOKEN>"
```

Or env: `TELEGRAM_BOT_TOKEN` (default account only).

Config shape ([docs](https://docs.openclaw.ai/channels/telegram/setup)):

```json5
channels: {
  telegram: {
    enabled: true,
    dmPolicy: "pairing",
    groups: { "*": { requireMention: true } },
  },
},
bindings: [
  { agentId: "sage", match: { channel: "telegram", accountId: "default" } },
],
```

### 4. Gateway + verify

```bash
openclaw gateway install
openclaw gateway start
openclaw gateway status          # port 18789
openclaw channels status --probe
```

### 5. Pair your phone (required)

1. Send any DM to your bot in Telegram.
2. `openclaw pairing list telegram`
3. `openclaw pairing approve telegram <CODE>` (expires in ~1 hour)

### 6. Smoke test

```text
SAGE: morning brief
DEV: what's on the overnight kanban?
DRUM: summarize tripwire steps
BIZ: Friday KPI
```

Prefixes route to pods per `openclaw/disklordz-picks/workspaces/sage/AGENTS.md`.

## Optional: drum factory + elite roster

```bash
bash openclaw/drum-kit-factory/scripts/install-drum-workspaces.sh
```

Drum Telegram prefix `DRUM:` → `drum-lead` (still same bot; sage delegates).

## Cron without Hermes

```bash
crontab -e
# paste disklordz-agent-stack/crontab.free.example
```

Morning brief cron can stay in `openclaw.json` `automation.jobs` (see merged example).

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| Bot silent in DM | Approve pairing |
| `unauthorized token` | Regenerate token in BotFather; `channels add` again |
| No group replies | `requireMention: true` — @mention the bot |
| Wrong agent | Check `bindings` → `sage` + `accountId: default` |
| Skills missing | `skills.load.extraDirs` → `…/openclaw/skills/shared` |
| Node too old | Upgrade to Node 24+ |

```bash
openclaw triage
openclaw doctor
openclaw logs --follow
```

## Security

- Never commit bot token or Gemini key.
- `dmPolicy: "pairing"` — only approved users can DM.
- Do not enable a second Telegram bot (see `disklordz-picks/PICKS.yaml`).