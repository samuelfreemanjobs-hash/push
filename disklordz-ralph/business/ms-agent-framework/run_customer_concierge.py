#!/usr/bin/env python3
"""DiskLordz customer concierge — AutoGen SelectorGroupChat (read-only guidance)."""

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
    except ImportError as exc:
        raise SystemExit(
            "Install: pip install -r requirements.txt (see docs/dev/ms-agent-framework-opt-in.md)"
        ) from exc

    api_key = os.environ.get("OPENAI_API_KEY", "").strip()
    if not api_key:
        raise SystemExit("Set OPENAI_API_KEY in .env")

    root = repo_root()
    shared = (
        f"DiskLordz sells synthesized drum kits and VST3 plugins. Repo: {root}. "
        "Do not promise physical drum shells. Human approves refunds and license keys."
    )

    model_client = OpenAIChatCompletionClient(model=os.environ.get("DLZ_AUTOGEN_MODEL", "gpt-4o"), api_key=api_key)

    concierge = AssistantAgent(
        name="DrumConcierge",
        model_client=model_client,
        system_message=shared
        + " Route VST/DAW issues to VSTSupport. Route kit ZIP/MIDI mapping to KitSpecialist.",
    )

    kit_specialist = AssistantAgent(
        name="KitSpecialist",
        model_client=model_client,
        system_message=shared
        + " Expert in GM MIDI, Roland V-Drums, Alesis maps, 24-bit kit install paths, "
        "factory/autoresearch export_kit.py layouts. No copyrighted sample advice.",
    )

    vst_support = AssistantAgent(
        name="VSTSupport",
        model_client=model_client,
        system_message=shared
        + " VST3 install: Inno Setup installers under vst/installer/, "
        "FL scan Common Files\\VST3, pluginval troubleshooting. "
        "Point engineers to docs/dev/juce-cmake-windows-troubleshooting.md.",
    )

    team = SelectorGroupChat(participants=[concierge, kit_specialist, vst_support], model_client=model_client)
    await Console(team.run_stream(task=task))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("task", nargs="?", default="Help a customer install DrumSynth VST3 in FL Studio 21")
    args = parser.parse_args()
    asyncio.run(main_async(args.task))


if __name__ == "__main__":
    main()