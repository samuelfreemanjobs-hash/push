#!/usr/bin/env python3
"""
Deep Agent orchestrator: long-horizon VST + sample pack launches (opt-in).

Context7 MCP + sub-agents + filesystem backend + SQLite checkpoints + human interrupts.
"""

from __future__ import annotations

import argparse
import asyncio
import os
import sys
from pathlib import Path

_ROOT = Path(__file__).resolve().parent
_FRAMEWORK = _ROOT.parent / "ms-agent-framework"
if str(_FRAMEWORK) not in sys.path:
    sys.path.insert(0, str(_FRAMEWORK))
if str(_ROOT) not in sys.path:
    sys.path.insert(0, str(_ROOT))

import deep_agent_tools as tools


def _default_vst_root() -> Path:
    env = os.environ.get("DLZ_VST_PROJECT_ROOT", "").strip()
    if env:
        return Path(env).resolve()
    repo = Path(os.environ.get("DLZ_REPO_ROOT", _ROOT.parents[3])).resolve()
    return (repo / "disklordz-ralph/vst/templates/drumkit-plugin").resolve()


def _default_factory_root(repo: Path) -> Path:
    env = os.environ.get("DLZ_AUDIO_FACTORY_ROOT", "").strip()
    if env:
        return Path(env).resolve()
    return (repo / "disklordz-ralph/factory/audio-factory").resolve()


async def main_async(task: str, thread_id: str) -> None:
    try:
        from deepagents import create_deep_agent
        from deepagents.backends import FilesystemBackend
        from langchain_mcp_adapters.client import MultiServerMCPClient
    except ImportError as exc:
        raise SystemExit("pip install -r requirements.txt (deep-agents-opt-in.md)") from exc

    vst_root = _default_vst_root()
    repo = Path(os.environ.get("DLZ_REPO_ROOT", vst_root.parents[4])).resolve()
    factory_root = _default_factory_root(repo)
    tools.configure_roots(vst_root, factory_root, vst_root)

    checkpointer = None
    try:
        from langgraph.checkpoint.sqlite import SqliteSaver

        db = _ROOT / "tmp" / "deep_agent_checkpoints.db"
        db.parent.mkdir(parents=True, exist_ok=True)
        checkpointer = SqliteSaver.from_conn_string(str(db))
    except ImportError:
        print("langgraph sqlite checkpointer unavailable — running without durability", file=sys.stderr)

    mcp_env = dict(os.environ)
    if os.environ.get("CONTEXT7_API_KEY"):
        mcp_env["CONTEXT7_API_KEY"] = os.environ["CONTEXT7_API_KEY"]

    mcp_client = MultiServerMCPClient(
        {
            "context7": {
                "command": "npx",
                "args": ["-y", "@upstash/context7-mcp@latest"],
                "transport": "stdio",
                "env": mcp_env,
            }
        }
    )
    context7_tools = await mcp_client.get_tools()

    dsp_subagent = {
        "name": "juce-dsp-engineer",
        "description": "C++20 JUCE 8 DSP, Context7, cmake + pluginval.",
        "system_prompt": (
            "Principal JUCE engineer on Windows. Context7 before API edits. "
            "Use read_dsp_file / write_dsp_file (sandboxed src/, ui/, CMakeLists.txt). "
            "Run build_and_validate_vst3 after substantive C++ changes."
        ),
        "tools": [
            *context7_tools,
            tools.build_and_validate_vst3,
            tools.read_dsp_file,
            tools.write_dsp_file,
        ],
    }

    pack_subagent = {
        "name": "sample-and-preset-producer",
        "description": "Master WAVs, presets, SKU staging (no live Shopify).",
        "system_prompt": (
            "Sample pack producer. run_multi_sku_pack_factory after DSP QA passes. "
            "Never claim shipped until RELEASE_SUMMARY says HUMAN_GATE cleared."
        ),
        "tools": [*context7_tools, tools.run_multi_sku_pack_factory],
    }

    model = os.environ.get("DLZ_DEEP_AGENT_MODEL", "openai:gpt-4o")

    agent_kwargs = {
        "model": model,
        "backend": FilesystemBackend(root_dir=str(vst_root)),
        "subagents": [dsp_subagent, pack_subagent],
        "system_prompt": (
            "Autonomous R&D Director for DiskLordz VST + kits. "
            "Use write_todos for multi-step launches. Offload huge logs to .deep-agent-logs/. "
            "Delegate DSP to juce-dsp-engineer, packs to sample-and-preset-producer. "
            "Pause before processor writes unless operator set DLZ_DEEP_AGENT_APPROVED=1. "
            "Finish with RELEASE_SUMMARY.md in the VST project root."
        ),
    }
    if checkpointer is not None:
        agent_kwargs["checkpointer"] = checkpointer

    orchestrator = create_deep_agent(**agent_kwargs)

    config = {"configurable": {"thread_id": thread_id}}
    interrupt = os.environ.get("DLZ_DEEP_AGENT_INTERRUPT", "1") == "1"
    if interrupt:
        # Pause before sensitive tool calls — resume via LangGraph / Agent UI with approval
        config["interrupt_before"] = ["tools"]

    result = await orchestrator.ainvoke({"messages": [{"role": "user", "content": task}]}, config=config)

    messages = result.get("messages", [])
    if messages:
        print(messages[-1].content if hasattr(messages[-1], "content") else messages[-1])


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--thread-id", default=os.environ.get("DLZ_DEEP_AGENT_THREAD", "drumkit-default"))
    parser.add_argument(
        "task",
        nargs="?",
        default=(
            "Plan v1.2: delegate smoothed subPunch APVTS to juce-dsp-engineer (pluginval 10), "
            "then TRAP_V2 pack + presets via sample-and-preset-producer, write RELEASE_SUMMARY.md."
        ),
    )
    args = parser.parse_args()
    asyncio.run(main_async(args.task, args.thread_id))


if __name__ == "__main__":
    main()