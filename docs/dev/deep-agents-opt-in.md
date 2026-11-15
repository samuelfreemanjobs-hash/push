# Deep Agents (langchain-ai/deepagents) — opt-in

## When to use

**Deep Agents** when one goal spans research, C++ edits, long build logs, pack export, and `RELEASE_SUMMARY.md` — beyond `MaxMessageTermination(15)` swarms.

Keep **Herdr** for visible panes; **Agno** for storefront API; **AutoGen** for factory/dsp closed loops.

## Install

```powershell
pip install deepagents langchain-mcp-adapters langgraph-checkpoint-sqlite
# or
.\scripts\run-drum-deep-agent.ps1   # creates venv on first run
```

## Configuration

| Variable | Purpose |
|----------|---------|
| `DLZ_VST_PROJECT_ROOT` | `drumkit-plugin` |
| `DLZ_AUDIO_FACTORY_ROOT` | `audio-factory` |
| `DLZ_DEEP_AGENT_MODEL` | default `openai:gpt-4o` |
| `DLZ_DEEP_AGENT_THREAD` | checkpoint thread id |
| `DLZ_DEEP_AGENT_INTERRUPT` | `1` = pause before tools |
| `DLZ_DEEP_AGENT_APPROVED` | `1` = allow Processor writes |

## Sub-agents

| Name | Tools |
|------|--------|
| `juce-dsp-engineer` | Context7, read/write DSP (gated), `build_and_validate_vst3` |
| `sample-and-preset-producer` | Context7, `run_multi_sku_pack_factory` |

## Checkpointing

SQLite: `business/deep-agents/tmp/deep_agent_checkpoints.db`

Resume with same `--thread-id`.

## dcode CLI

Optional terminal agent from LangChain — use for ad-hoc edits; still run pluginval on Windows before ship.

## STACK-FROZEN

No autonomous publish to `starter-cpp` or live Shopify without human gates aligned with `drumkit-plugin/AGENTS.md`.