# Mem0 — cross-session agent memory (opt-in)

**Policy:** Does not block Lite VST3 ship (`STACK-FROZEN.md`). Operator credentials only — never commit `MEM0_API_KEY`.

[Mem0](https://mem0.ai) is the managed memory layer for **Claude Code** long runs and the **`memory-curator`** subagent. It complements (does not replace):

| Layer | Where | Best for |
|-------|--------|----------|
| **Mem0** | Platform + optional Claude plugin | Autoresearch wins, constraints, fixes across Claude Code nights |
| **Ruflo memory** | `npx ruflo@latest memory` | Same machine when Ruflo swarm is active |
| **Supermemory** | Cursor MCP | Cloud / Cursor session recall |
| **Repo truth** | `CLAUDE.md`, `.claude/rules`, judges | Immutable guardrails |

---

## Step A — Claude Code plugin (recommended)

Persistent recall without manual logging. See [Mem0 × Claude Code](https://docs.mem0.ai/integrations/claude-code).

```powershell
$env:MEM0_API_KEY = "m0-..."   # from https://app.mem0.ai/dashboard/api-keys
claude plugin marketplace add mem0ai/mem0
claude plugin install mem0@mem0-plugins --scope user --config api_key="$env:MEM0_API_KEY"
Remove-Item Env:MEM0_API_KEY
```

Restart Claude Code. Useful slash commands:

| Command | Use |
|---------|-----|
| `/mem0:search` | Prior DSP decisions, `--category results` |
| `/mem0:status` | API key + capture health |
| `/mem0:remember` | Pin a constraint after a hard-won fix |

**Sidekick:** `mem0:sidekick` runs in an isolated worktree for investigation — review before merging to ship path.

---

## Step B — Python bridge (explicit wins)

After a **kept** autoresearch commit:

```powershell
cd C:\Dev\push\disklordz-ralph\business\mem0
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -r requirements.txt

$env:MEM0_API_KEY = "m0-..."
# Optional scopes (defaults: diskordz-operator / memory-curator / disklordz-push):
# $env:DLZ_MEM0_USER_ID = "your-handle"
# $env:DLZ_MEM0_GIT_SHA = (git rev-parse --short HEAD)

python record_autoresearch_win.py --track cpp
python record_autoresearch_win.py --track kit
python search_dsp_memory.py "saturation filter pitch envelope"
```

Or from repo root: `scripts/mem0-record-win.ps1 -Track cpp`

`--dry-run` prints the payload without calling the API.

---

## `memory-curator` agent

Subagent: **`.claude/agents/memory-curator.md`**

| When | Action |
|------|--------|
| Start of autoresearch cycle | `/mem0:search` or `search_dsp_memory.py` for `decisions_and_constraints` + `results` |
| After qa-validator **keep** | `record_autoresearch_win.py` or `/mem0:remember` with `TOTAL_LOSS` + one-line hypothesis |
| Never | Store live API keys, Stripe secrets, or copyrighted stems |

Wire into swarm: **`/disk-autoresearch-cpp`** calls the curator after a keep (see `.claude/commands/disk-autoresearch-cpp.md`).

---

## Categories (plugin-aligned)

| Category | DiskLordz examples |
|----------|-------------------|
| `project_knowledge` | `STACK-FROZEN`, ship path `starter-cpp` |
| `decisions_and_constraints` | RT-safe rules, originality floor, pluginval strictness |
| `workflows` | Command → Agent → Skill, export kit handoff |
| `problems_and_fixes` | pluginval failures, judge regressions |
| `results` | `TOTAL_LOSS`, winning envelope constants |

Seed once: `scripts/mem0-seed-memory.ps1`

---

## Security

- API key in env or Claude plugin config only
- Do not upload copyrighted manuals — use RAGFlow MCP (`docs/dev/ragflow-knowledge-base.md`)
- Mem0 does not authorize storefront or license changes

---

## Related

- `docs/dev/claude-code-best-practice.md`
- `docs/dev/ruflo-opt-in.md` (Ruflo vector memory)
- `.claude/agents/memory-curator.md`