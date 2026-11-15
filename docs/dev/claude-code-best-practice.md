# Claude Code layout (shanraisshan best practice)

DiskLordz adapts patterns from [claude-code-best-practice](https://github.com/shanraisshan/claude-code-best-practice): **Command → Agent → Skill**, path-scoped **rules**, and hardened **settings**.

**Upstream reference only** — we do not vendor that repo. Re-read upstream when upgrading Claude Code.

---

## Repo layout

| Piece | Path | Purpose |
|-------|------|---------|
| Memory (short) | `CLAUDE.md` | Mission, guardrails, index |
| Path rules | `.claude/rules/*.md` | Lazy-loaded by `paths:` |
| Subagents | `.claude/agents/*.md` | Ruflo / specialist roles |
| Slash commands | `.claude/commands/*.md` | Operator workflows |
| Skills | `.claude/skills/<name>/SKILL.md` | Invokable procedures |
| Nested skills | `disklordz-ralph/**/.claude/skills/` | Monorepo package scope |
| Permissions | `.claude/settings.json.example` → local `.claude/settings.json` | Allow/deny Bash |

Also: root `skills/vst_audio_security.md` for **Strix** (not Claude Code skill format).

---

## Operator setup

```powershell
cd C:\Dev\push
Copy-Item .claude\settings.json.example .claude\settings.json
# Optional: npx ruflo@latest init — then merge permissions, do not commit secrets
claude
/disk-pluginval-gate
```

---

## Orchestration examples

| Command | Agents | Skills |
|---------|--------|--------|
| `/disk-export-kit` | `kit-producer` | `commercial-kit-export` |
| `/disk-pluginval-gate` | `qa-validator` | `pluginval-gate` |
| `/disk-autoresearch-cpp` | `memory-curator` → `dsp-architect` → `qa-validator` → `memory-curator` | `cpp-voice-dsp-loop`, `mem0-dsp-memory` |
| `/disk-mem0-recall` | `memory-curator` | `mem0-dsp-memory` |
| `/disk-rt-audit` | `dsp-rt-auditor` | `dsp-rt-verification` |

See `.claude/commands/` for prompts.

---

## Monorepo skills

Per [upstream mono-repo guide](https://github.com/shanraisshan/claude-code-best-practice/blob/main/reports/claude-skills-for-larger-mono-repos.md):

- Root `.claude/skills/` — shared (pluginval, kit export)
- `factory/autoresearch/.claude/skills/` — Python judge loop
- `vst/starter-cpp/.claude/skills/` — ship VST (when added)

Nested skills appear when Claude edits files under that tree.

---

## Practices we follow

1. **CLAUDE.md** stays short; details in `.claude/rules/`
2. **Plan mode** for ship-path (`starter-cpp`) and paid portal merges
3. **Human gates** — storefront, Stripe live, MetaGPT merge, Strix auto-fixes
4. **`/compact`** around 50% context on long autoresearch nights
5. Subagents use **Agent tool**, not nested bash `claude` processes

---

## Memory layers

- **Mem0** — Claude Code nights + `memory-curator` (`docs/dev/mem0-opt-in.md`)
- **Ruflo** — local swarm vector memory (`docs/dev/ruflo-opt-in.md`)
- **Supermemory** — Cursor MCP (cloud agents)

---

## Related

- `docs/dev/elite-dsp-agent-context7.md`
- `docs/dev/mem0-opt-in.md`
- `docs/dev/ruflo-opt-in.md`
- `AGENTS.md` (Cursor)
- `disklordz-ralph/STACK-FROZEN.md`