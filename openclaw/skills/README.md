# DiskLordz OpenClaw skills library

OpenClaw-standard **`SKILL.md`** skills (YAML frontmatter + body). Patterns from OpenClaw skills, multi-agent delegation, and file-based handoffs (`disklordz-agent-stack`).

## Install

```bash
./install-skills.sh
openclaw skills check
```

Installs:

- **Shared** → `~/.openclaw/skills/` (all agents, allowlist-filtered)
- **Per-agent** → `~/.openclaw/workspace-<id>/skills/`

## Invoke in chat

```text
$disk_ops_snapshot summarize P0
$kit_spec_grok validate ./kit-spec.yaml
```

## Framework map

| Pattern | Where |
|---------|--------|
| Tool-gated ReAct | OpenClaw agent turn + skills_read |
| Coordinator / workers | `sage` + subagents (hierarchical delegation) |
| Blackboard handoff | `~/.openclaw/hermes-out/` + agent-stack |
| Skill allowlists | `agents/REGISTRY.yaml` → `openclaw.json` |
| Human-in-the-loop | skills mark `approval_required` steps |

Registry: `../agents/REGISTRY.yaml`