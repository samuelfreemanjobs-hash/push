# DiskLordz elite agent team (OpenClaw)

Free, self-hosted multi-agent crew for phone + desktop when Cursor/Grok are unavailable.

## Quick bootstrap

```bash
# 1) Install OpenClaw (host machine or free Oracle ARM VM)
curl -fsSL https://openclaw.ai/install.sh | bash
openclaw onboard

# 2) Copy this team's config snippet into ~/.openclaw/openclaw.json
#    See openclaw.json5.example — set models.providers.google.apiKey (Gemini free)

# 3) Materialize workspaces from this repo
./scripts/install-workspaces.sh

# 4) Register agents (non-interactive)
openclaw agents add sage --workspace ~/.openclaw/workspace-sage --non-interactive
openclaw agents add code --workspace ~/.openclaw/workspace-code --non-interactive
# … repeat for each id in roster.yaml, or run install-workspaces.sh which prints commands

openclaw gateway restart
openclaw dashboard   # Add to phone home screen
```

## Free coding model stack

| Priority | Provider | Model | Cost |
|----------|----------|-------|------|
| 1 | Google AI Studio | `gemini-2.0-flash` or `gemini-2.5-flash` | Free tier |
| 2 | Ollama on LAN | `qwen2.5-coder:7b` or `deepseek-r1:8b` | $0 local |
| 3 | Groq | `llama-3.3-70b-versatile` | Free tier |

Point **CODE**, **PATCH**, and **REVIEW** at Flash/Ollama; use **SAGE** as coordinator (delegates to specialists).

## Phone paths

1. **Telegram** → single bot → prefix routing (`CODE: fix vite config`)
2. **PWA** → Command Center **Mobile crew** → `ai-agent-team` API on Railway/Render free tier
3. **OpenClaw Web Control UI** over Tailscale

## Roster

Canonical list: `roster.yaml`. Elite lanes align with vault `slack-agents.yaml` plus coding specialists.

## ai-agent-team bridge (no OpenClaw required)

Lightweight Express API in repo root `ai-agent-team/`:

```bash
cd ai-agent-team && npm install
export GEMINI_API_KEY=...
npm run dev
```

`POST /api/agent` body: `{ "message": "CODE: explain this diff", "userId": "mobile" }`

Set Command Center `VITE_AGENT_API_URL=http://127.0.0.1:3000` for in-PWA chat.

## Drum kit factory (Grok Architect + OpenClaw)

Grok Bot **Drum Kit Architect** owns creative `kit-spec.yaml`. OpenClaw **drum-kit-factory** (5 agents) runs QC, manifest, zip, and Telegram on your VPS. See [`../drum-kit-factory/README.md`](../drum-kit-factory/README.md).

## Your OC + Hermes split

Operator picks and stack config: [`../disklordz-picks/README.md`](../disklordz-picks/README.md) · Hermes layer: [`../../hermes/disklordz-picks/README.md`](../../hermes/disklordz-picks/README.md).