# DiskLordz agent stack — $0 runbook

Use this when you want **your OC + H picks** without paid Hermes models, Fluence, or Codex.

## What costs $0

| Piece | Free option |
|-------|-------------|
| OpenClaw | MIT, self-host |
| Telegram | BotFather bot |
| LLM | Google AI Studio **Gemini 2.0 Flash** (free tier) |
| Heavy code (optional) | **Ollama** `qwen2.5-coder:7b` on same VPS |
| Hermes layer | **This package** (`disklordz-agent-stack`) writes `~/.openclaw/hermes-out/` |
| VPS | Oracle Always Free ARM, home PC, or old laptop |
| GitHub | Public API 60/h unauthenticated; faster with fine-grained PAT (free) |
| Grok Architect | Your existing Grok when available; else manual kit-spec |
| Mobile fallback | `ai-agent-team` + Gemini key |
| Command Center | Static PWA (publish or `npm run dev`) |

## What we skip in free mode

- Hermes product install (optional upgrade)
- Codex `/bg` (use OpenClaw **dev** overnight + Gemini/Ollama)
- Paid scrape APIs (research stubs + manual/Grok)
- Stripe KPI until you add read-only key

## 30-minute setup

```bash
# 1) VPS or home machine
curl -fsSL https://openclaw.ai/install.sh | bash
openclaw onboard   # Gemini API key from aistudio.google.com — free

# 2) Clone your repo
git clone https://github.com/samuelfreemanjobs-hash/push.git ~/push
cd ~/push/disklordz-agent-stack && npm install

# 3) Paths (adjust user)
export DLZ_REPO_ROOT=~/push
export DISKDLORDZ_OPS_TOKEN=   # optional GitHub PAT for vault
export GEMINI_API_KEY=         # for OpenClaw + ai-agent-team

# 4) Agent outputs (Hermes-free)
npm run all

# 5) OpenClaw workspaces
bash ~/push/openclaw/disklordz-picks/scripts/install-pick-workspaces.sh
bash ~/push/openclaw/drum-kit-factory/scripts/install-drum-workspaces.sh

# 6) Merge configs
#    openclaw/disklordz-picks/openclaw.stack.json5.example
#    models.providers.google.apiKey = Gemini

openclaw gateway install && openclaw gateway start

# 7) Elite skills (OpenClaw SKILL.md library)
bash ~/push/openclaw/skills/install-skills.sh
# Merge openclaw/skills/openclaw.skills.json5.example into openclaw.json
```

`DLZ_REPO_ROOT` defaults to the git repo that contains `disklordz-agent-stack/` (set explicitly if your clone path differs).

## Cron (free, no Hermes)

```bash
crontab -e
# paste from deploy/crontab.free.example
```

Morning: agent-stack → OpenClaw sage cron reads `hermes-out/`.

Technology radar: `npm run tech-radar` ingests [AI Dev Signals](https://www.aidevsignals.com/); `RADAR:` agent writes screened reports — see `TECH-RADAR.md` and opportunity stack `OPPORTUNITIES.md`.

## Telegram prompts

```text
SAGE: morning brief
DEV: overnight goal on disklordz-command-center — branch only
DRUM: ingest kit-spec (after Grok)
BIZ: Friday KPI
MKT: summarize hermes-out/research/
RADAR: review latest AI Dev Signals issue
MKT: complete Monday beatmaker brief from hermes-out
```

## Ralph loop (whole business)

Autonomous one-story iterations: `disklordz-ralph/ralph.ps1` (Windows) or `ralph.sh` (Linux). VST MSVC work is lane **DLZ-VST-*** only — not a separate business.

## Upgrade path (still your picks)

When ready: install Hermes, point crons at same `hermes-out/` paths, enable webhooks from `hermes/disklordz-picks/webhooks/github.yaml`.