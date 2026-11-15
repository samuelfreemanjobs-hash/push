# Claude-Mem (Cursor persistent memory)

[Claude-Mem](https://github.com/thedotmack/claude-mem) records Cursor agent sessions via hooks and a **local worker**, then injects relevant notes into new chats. It complements (does not replace) repo docs, **OpenClaw** ops memory, or **Supermemory** MCP in cloud agents.

**Operator install only** — hooks and worker run on your Windows/macOS machine, not in Cloud Agent VMs.

## Install (Windows — primary)

**Prerequisites:** [Bun](https://bun.sh) (installer will prompt if missing), Node 22+, Cursor restarted after install.

```powershell
cd C:\Dev\push
.\scripts\install-claude-mem.ps1
```

Or manually:

```powershell
npx claude-mem install
```

Pick **Cursor**, complete provider (CMEM Pro default, or Gemini / OpenRouter — see `claude-mem.settings.example.json`).

**Restart Cursor** once after install.

## Verify

```powershell
.\scripts\check-claude-mem-health.ps1
```

Or read port from `%USERPROFILE%\.claude-mem\.worker.port` and:

```powershell
curl -s "http://127.0.0.1:<port>/api/health"
```

Do a short Agent task, then open the worker URL printed at install — new observations should appear.

## Provider: Gemini (matches DiskLordz stack)

```powershell
mkdir $env:USERPROFILE\.claude-mem -Force
# Copy and edit:
# docs/dev/claude-mem.settings.example.json -> %USERPROFILE%\.claude-mem\settings.json
npx claude-mem install
```

Docs: [Cursor + Gemini](https://docs.claude-mem.ai/cursor/gemini-setup).

## Project vs user hooks

| Scope | Command |
|-------|---------|
| This repo only | Run install from `C:\Dev\push` (default **project**) |
| All Cursor projects | `npx claude-mem install --ide cursor` and choose **user** when prompted |

Hooks land under `.cursor/` (project) or user Cursor config. **Do not commit** live `hooks.json` if it contains secrets — use `npx claude-mem install` per machine.

## What to remember for DiskLordz

After install, seed context from:

- `disklordz-ralph/STACK-FROZEN.md`
- `disklordz-ralph/LAUNCH-ROADMAP-30D.md`
- `disklordz-ralph/vst/AGENTS.md`
- Active story: `disklordz-ralph/prd.json` (`DLZ-VST-*`)

Optional: `docs/dev/claude-mem-seed.md` — paste bullets into the memory viewer or first Agent prompt.

## Uninstall / repair

```powershell
npx claude-mem uninstall
npx claude-mem repair
npx claude-mem doctor
```

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| `bun-missing-after-install` | Install Bun, reopen terminal, re-run install script |
| Nothing stored | Healthy worker? Used `npx claude-mem install` not `npm i -g`? Restart Cursor |
| Hooks idle | Restart Cursor; logs at `%USERPROFILE%\.claude-mem\logs\worker-YYYY-MM-DD.log` |

Never restart a **healthy** worker — in-memory queue may drop notes.