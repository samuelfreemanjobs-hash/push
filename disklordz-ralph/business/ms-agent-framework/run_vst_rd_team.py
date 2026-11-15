#!/usr/bin/env python3
"""Internal VST R&D — AutoGen + Context7 MCP (JUCE 8 / CMake)."""

from __future__ import annotations

import argparse
import asyncio
import os
import sys


def repo_root() -> str:
    return os.environ.get("DLZ_REPO_ROOT", os.path.abspath(os.path.join(os.path.dirname(__file__), "../../..")))


async def main_async(task: str) -> None:
    try:
        from autogen_agentchat.agents import AssistantAgent
        from autogen_agentchat.teams import SelectorGroupChat
        from autogen_agentchat.ui import Console
        from autogen_ext.models.openai import OpenAIChatCompletionClient
        from autogen_ext.tools.mcp import McpWorkbench, StdioServerParams
    except ImportError as exc:
        raise SystemExit(
            "Install: pip install -r requirements.txt (see docs/dev/ms-agent-framework-opt-in.md)"
        ) from exc

    api_key = os.environ.get("OPENAI_API_KEY", "").strip()
    if not api_key:
        raise SystemExit("Set OPENAI_API_KEY in .env")

    root = repo_root()
    use_mcp = os.environ.get("DLZ_MCP_CONTEXT7", "1").lower() not in ("0", "false", "no")

    model_client = OpenAIChatCompletionClient(model=os.environ.get("DLZ_AUTOGEN_MODEL", "gpt-4o"), api_key=api_key)

    rt_rules = (
        "RT rules: no heap/locks/strings in processBlock; ScopedNoDenormals; SmoothedValue; "
        "pluginval strictness 8 pre-release. Repo rules: .cursor/rules/audio-dsp.mdc"
    )

    async def run_team(workbench=None):
        vst_engineer = AssistantAgent(
            name="VSTEngineer",
            model_client=model_client,
            workbench=workbench,
            system_message=(
                f"You are Principal JUCE 8 C++20 engineer for DiskLordz at {root}. "
                "Query Context7 before JUCE API changes. "
                "Ship path: disklordz-ralph/vst/starter-cpp. "
                "WebView template: vst/templates/webview-drum-starter. "
                + rt_rules
            ),
        )

        rt_auditor = AssistantAgent(
            name="RTAuditor",
            model_client=model_client,
            system_message=(
                "You review DSP diffs for real-time violations only. "
                "Reject new/mutex/String in processBlock. Recommend pluginval. " + rt_rules
            ),
        )

        team = SelectorGroupChat(participants=[vst_engineer, rt_auditor], model_client=model_client)
        await Console(team.run_stream(task=task))

    if not use_mcp:
        await run_team(None)
        return

    context7 = StdioServerParams(
        command="npx",
        args=["-y", "@upstash/context7-mcp@latest"],
    )

    async with McpWorkbench(context7) as mcp_workbench:
        await run_team(mcp_workbench)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "task",
        nargs="?",
        default="Draft sample-accurate MIDI handling for kick note 36 in a JUCE 8 drum synth",
    )
    args = parser.parse_args()
    asyncio.run(main_async(args.task))


if __name__ == "__main__":
    main()