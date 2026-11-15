# Elite agent frameworks (how DiskLordz uses them)

## OpenClaw (primary runtime)

- **Skills** = procedural memory (`SKILL.md`, `$skill` invocation)
- **Multi-agent** = isolated workspaces + `subagents.allowAgents`
- **Cron/automation** = proactive briefs without prompting
- **Tool allowlists** per agent (factory vs coordinator)

## Patterns we compose

| Framework pattern | DiskLordz implementation |
|-------------------|---------------------------|
| Hierarchical task decomposition | sage → dev/mkt/biz/drum-lead |
| Blackboard / shared state | `hermes-out/` + ops-snapshot |
| ReAct + tools | OpenClaw turn + skills_read |
| Human-on-the-loop | G3 founder ears, draft PR only |
| Planner–worker–verifier | Hermes research pipeline (optional) |
| File-based contracts | `kit-spec.yaml`, JSON reports |

## Not in repo (optional later)

- LangGraph / Temporal — use when you need durable multi-day workflows
- Custom MCP — add per integration in OpenClaw plugins

## Free tier

Gemini Flash + skills + `disklordz-agent-stack` = no Hermes required. See `disklordz-agent-stack/FREE.md`.

## Registry

`agents/REGISTRY.yaml` lists every elite/subagent and skill allowlist.