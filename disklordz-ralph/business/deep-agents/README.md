# Deep Agents — long-horizon VST + pack orchestrator (opt-in)

LangChain **Deep Agents**: `write_todos`, filesystem backend, sub-agents, Context7 MCP.

| vs | Use when |
|----|----------|
| AutoGen `vst_rd_swarm` | Tight 3-agent loop, fixed turns |
| **Deep Agents** | 30–50+ tool steps, log offloading, crash-resume |
| Herdr | Interactive panes you steer live |
| Agno | Customer API + scheduled merch |

## Install

```powershell
cd disklordz-ralph\business\deep-agents
python -m venv .venv
.\.venv\Scripts\pip install -r requirements.txt
$env:OPENAI_API_KEY = "..."
```

## Run

```powershell
$env:DLZ_REPO_ROOT = "C:\Dev\push"
python drum_deep_agent.py

# Or from template:
cd ..\..\vst\templates\drumkit-plugin
python drum_deep_agent.py
```

## Human-in-the-loop

| Control | Effect |
|---------|--------|
| `DLZ_DEEP_AGENT_INTERRUPT=1` (default) | `interrupt_before: ["tools"]` — approve in LangGraph UI |
| `DLZ_DEEP_AGENT_APPROVED=1` | Allow `write_dsp_file` on Processor |
| `tmp/deep_agent_checkpoints.db` | Resume after crash |

## Logs

Large MSVC/pluginval output → `.deep-agent-logs/` under VST project root.

Docs: `docs/dev/deep-agents-opt-in.md`