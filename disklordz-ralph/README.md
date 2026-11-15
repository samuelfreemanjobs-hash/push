# DiskLordz Ralph loop (whole business)

**Ralph** is one-story-per-iteration autonomy: read `prd.json` + `progress.txt`, ship the highest-priority item with `"passes": false`, run that story’s verification, commit, repeat until `<promise>COMPLETE</promise>`.

This folder is the **business** control plane. The Detroit Phonk VST3 MSVC loop is one **lane** (`VST`), not the whole product.

## What Ralph owns

| Lane | Examples |
|------|-----------|
| **FLOW** | Command Center PWA, CI, ops snapshot, GitHub workflows |
| **OC** | OpenClaw install, skills, Telegram sage, overnight dev |
| **H** | Hermes-out / `disklordz-agent-stack` (free tier first) |
| **DRUM** | Grok kit-spec → drum-kit-factory → tripwire pack |
| **VST** | Instruments / JUCE (often `C:\Dev\DetroitPhonkTrapVST3` on Windows) |
| **SAAS** | Tripwire checkout, Supabase, Stripe (operator gates) |
| **BIZ** | Launch gates G3, Friday KPI, vault kanban alignment |

Vault kanban (`agents/disklordz/kanban.yaml`) is truth for **priority**; `prd.json` is truth for **Ralph execution state**. Optional: `node scripts/sync-priority-hint.mjs` to annotate the next story from P0 cards.

## Windows (founder machine)

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
cd C:\Dev\push   # or your clone path
.\disklordz-ralph\ralph.ps1
```

Requires [Claude Code CLI](https://docs.anthropic.com/en/docs/claude-code) (`claude -p`) or swap the loop body for Cursor Agent.

**VST-only sub-workspace** (MSVC + pluginval): `windows\bootstrap-DetroitPhonkTrapVST3.ps1` then stories `DLZ-VST-*` in `prd.json`.

## Linux / VPS / Cloud Agent

```bash
cd /path/to/push
./disklordz-ralph/ralph.sh
```

Or paste `PROMPT.md` into a Cursor Cloud Agent on branch `cursor/disklordz-command-center-634f`.

## Verify one story

```bash
node disklordz-ralph/scripts/verify-story.mjs DLZ-OPS-001
```

## Operator vs agent

Stories with `"actor": "operator"` or `"gate": "G3"` must **not** auto-set `passes: true`. Ralph documents blockers in `progress.txt` and exits cleanly.

## Files

| File | Role |
|------|------|
| `PROMPT.md` | Agent system prompt (mirror to `.cursorrules` at repo root optional) |
| `prd.json` | All lanes, one story at a time |
| `progress.txt` | Append-only ship log |
| `ralph.ps1` / `ralph.sh` | Iteration drivers |
| `windows/` | Windows-only bootstrap (JUCE, pluginval, VST tree) |