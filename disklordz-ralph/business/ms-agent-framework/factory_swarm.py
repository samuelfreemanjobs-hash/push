#!/usr/bin/env python3
"""
Unified Audio Production Engine: DSP loop + sample pack + VST preset factories.

AutoGen + Context7. Operator on Windows for dsp_harness MSVC builds.
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

import audio_factory_tools as factory


def default_factory_root() -> Path:
    env = os.environ.get("DLZ_AUDIO_FACTORY_ROOT", "").strip()
    if env:
        return Path(env).resolve()
    repo = os.environ.get("DLZ_REPO_ROOT", "").strip()
    if repo:
        return (Path(repo) / "disklordz-ralph/factory/audio-factory").resolve()
    return (Path(__file__).resolve().parents[2] / "factory" / "audio-factory").resolve()


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
            "pip install -r disklordz-ralph/business/ms-agent-framework/requirements.txt"
        ) from exc

    api_key = os.environ.get("OPENAI_API_KEY", "").strip()
    if not api_key:
        raise SystemExit("Set OPENAI_API_KEY")

    factory.FACTORY_ROOT = default_factory_root()
    print(f"Audio factory root: {factory.FACTORY_ROOT}", file=sys.stderr)

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

    preset_ids = os.environ.get(
        "DLZ_FACTORY_PRESET_PARAMS",
        "outputGain (0.0–2.0 on drumkit-plugin APVTS)",
    )

    async with McpWorkbench(context7_server) as context7_workbench:
        dsp_engineer = AssistantAgent(
            name="DSP_Loop_Engineer",
            model_client=model_client,
            workbench=context7_workbench,
            tools=[factory.update_and_compile_dsp, factory.benchmark_dsp_audio],
            system_message=(
                "DSP engineer. Context7 for juce::dsp before suggesting JUCE upgrades. "
                "Only edit dsp_harness/SaturationDSP.h via update_and_compile_dsp. "
                "Target warm 12-bit punch: crest_factor_db 6.5–9.5, spectral_centroid_hz < 2200 "
                "at drive 0.65 on raw_audio/test_loop.wav."
            ),
        )

        sample_factory = AssistantAgent(
            name="Sample_Pack_Factory",
            model_client=model_client,
            tools=[factory.process_and_master_sample_pack],
            system_message=(
                "Sample pack producer. Run process_and_master_sample_pack; "
                "confirm manifest.json and README in dist_sample_pack/."
            ),
        )

        preset_factory = AssistantAgent(
            name="VST_Preset_Factory",
            model_client=model_client,
            workbench=context7_workbench,
            tools=[
                factory.write_and_validate_vst_preset,
                factory.generate_velocity_layer_map,
                factory.read_factory_manifest,
            ],
            system_message=(
                "VST sound designer. Context7 for APVTS XML. Params: "
                + preset_ids
                + ". Root <Parameters> with <PARAM id=\"...\" value=\"...\"/>. "
                "Save 3 presets (Memphis_94_Crunch, Atlanta_808_Knock, Clean_Studio_Punch). "
                "Then generate_velocity_layer_map. End with ALL_FACTORIES_COMPLETE."
            ),
        )

        termination = TextMentionTermination("ALL_FACTORIES_COMPLETE") | MaxMessageTermination(max_turns)

        swarm = RoundRobinGroupChat(
            participants=[dsp_engineer, sample_factory, preset_factory],
            termination_condition=termination,
        )

        await Console(swarm.run_stream(task=task))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--max-turns",
        type=int,
        default=int(os.environ.get("DLZ_FACTORY_MAX_TURNS", "18")),
    )
    parser.add_argument(
        "task",
        nargs="?",
        default=(
            "1) Benchmark 12-bit saturator at drive=0.65 on raw_audio/test_loop.wav. "
            "2) Master/tag raw_audio into dist_sample_pack/. "
            "3) Write 3 APVTS XML presets and velocity_layers.json in dist_presets/."
        ),
    )
    args = parser.parse_args()
    asyncio.run(main_async(args.task, args.max_turns))


if __name__ == "__main__":
    main()