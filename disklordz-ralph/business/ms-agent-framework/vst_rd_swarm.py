#!/usr/bin/env python3
"""
Autonomous VST R&D swarm: DSP_Architect + RT_Audio_Auditor + Build_QA_Runner.

Context7 MCP + sandboxed file tools + cmake/pluginval on Windows.

Default project: drumkit-plugin template (not starter-cpp ship tree).
"""

from __future__ import annotations

import argparse
import asyncio
import os
import sys
from pathlib import Path

_SCRIPT_DIR = Path(__file__).resolve().parent
if str(_SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(_SCRIPT_DIR))

import vst_engineering_tools as eng


def default_project_root() -> Path:
    env = os.environ.get("DLZ_VST_PROJECT_ROOT", "").strip()
    if env:
        return Path(env).resolve()
    repo = os.environ.get("DLZ_REPO_ROOT", "").strip()
    if repo:
        return (Path(repo) / "disklordz-ralph/vst/templates/webview-drum-starter").resolve()
    return (
        Path(__file__).resolve().parents[2]
        / "vst"
        / "templates"
        / "webview-drum-starter"
    ).resolve()


def assert_project_allowed(project: Path) -> None:
    repo = os.environ.get("DLZ_REPO_ROOT", "").strip()
    project_s = str(project).replace("\\", "/")
    if "starter-cpp" in project_s and os.environ.get("DLZ_SWARM_ALLOW_SHIP", "") != "1":
        raise SystemExit(
            "Refusing starter-cpp ship tree. Set DLZ_SWARM_ALLOW_SHIP=1 only with human review, "
            "or point DLZ_VST_PROJECT_ROOT at webview-drum-starter."
        )
    if repo and not project_s.startswith(str(Path(repo).resolve()).replace("\\", "/")):
        print("Warning: project root is outside DLZ_REPO_ROOT", file=sys.stderr)


async def main_async(task: str, max_turns: int) -> None:
    try:
        from autogen_agentchat.agents import AssistantAgent
        from autogen_agentchat.conditions import MaxMessageTermination, TextMentionTermination
        from autogen_agentchat.teams import RoundRobinGroupChat
        from autogen_agentchat.ui import Console
        from autogen_ext.models.openai import OpenAIChatCompletionClient
        from autogen_ext.tools.mcp import McpWorkbench, StdioServerParams
    except ImportError as exc:
        raise SystemExit(
            "pip install -r requirements.txt (docs/dev/ms-agent-framework-opt-in.md)"
        ) from exc

    api_key = os.environ.get("OPENAI_API_KEY", "").strip()
    if not api_key:
        raise SystemExit("Set OPENAI_API_KEY")

    project = default_project_root()
    assert_project_allowed(project)
    eng.PROJECT_ROOT = project
    print(f"Project root: {project}", file=sys.stderr)

    model = os.environ.get("DLZ_AUTOGEN_MODEL", "gpt-4o")
    model_client = OpenAIChatCompletionClient(model=model, api_key=api_key)

    c7_env = {}
    if os.environ.get("CONTEXT7_API_KEY"):
        c7_env["CONTEXT7_API_KEY"] = os.environ["CONTEXT7_API_KEY"]

    context7_server = StdioServerParams(
        command="npx",
        args=["-y", "@upstash/context7-mcp@latest"],
        env=c7_env or None,
    )

    rt_checklist = (
        "No new/delete/malloc/vector growth/String in processBlock; no mutex; "
        "ScopedNoDenormals; SmoothedValue for continuous params. "
        "Approve with exact token: RT_AUDIT_APPROVED"
    )

    async with McpWorkbench(context7_server) as context7_workbench:
        dsp_architect = AssistantAgent(
            name="DSP_Architect",
            model_client=model_client,
            workbench=context7_workbench,
            tools=[eng.read_project_file, eng.write_project_file],
            system_message=(
                "Principal JUCE 8 / C++20 engineer. ALWAYS query Context7 before JUCE API edits. "
                f"Project root files only via read_project_file / write_project_file. "
                "Writes limited to src/, ui/, CMakeLists.txt. "
                "Zero RT violations in processBlock."
            ),
        )

        rt_auditor = AssistantAgent(
            name="RT_Audio_Auditor",
            model_client=model_client,
            tools=[eng.read_project_file],
            system_message="Real-time audio auditor. " + rt_checklist,
        )

        build_qa_runner = AssistantAgent(
            name="Build_QA_Runner",
            model_client=model_client,
            tools=[eng.run_cmake_build, eng.run_pluginval_tests],
            system_message=(
                "Build engineer (Windows). After RT_AUDIT_APPROVED, run run_cmake_build then "
                "run_pluginval_tests. Paste failures to DSP_Architect. "
                "When both pass, say SWARM_VERIFIED_COMPLETE exactly once."
            ),
        )

        termination = TextMentionTermination("SWARM_VERIFIED_COMPLETE") | MaxMessageTermination(max_turns)

        swarm = RoundRobinGroupChat(
            participants=[dsp_architect, rt_auditor, build_qa_runner],
            termination_condition=termination,
        )

        await Console(swarm.run_stream(task=task))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--max-turns",
        type=int,
        default=int(os.environ.get("DLZ_SWARM_MAX_TURNS", "15")),
    )
    parser.add_argument(
        "task",
        nargs="?",
        default=(
            "Add APVTS parameter 'drive' (0..1) with SmoothedValue soft saturation in "
            "src/PluginProcessor.cpp. Audit RT safety, build Release, pass pluginval."
        ),
    )
    args = parser.parse_args()
    asyncio.run(main_async(args.task, args.max_turns))


if __name__ == "__main__":
    main()