# VST feature spec — EGM template

## Goal

One sentence measurable outcome (e.g. "Background TRT thread renders 8s stereo loop without blocking `processBlock`").

## Scope

- In: files, modules, DAW targets
- Ship path: starter-cpp only OR opt-in lab path — pick one

## Non-goals

What this PR explicitly does **not** do (cloud inference in RT path, new agent framework on Lite path, etc.).

## Architecture

Diagram or bullet flow (UI thread → worker → lock-free buffer → playback).

## RT safety

- No allocations/locks in `processBlock`
- TensorRT / GPU work only on `juce::Thread`
- Reference: `disklordz-ralph/vst/templates/drumkit-plugin/AGENTS.md`

## Acceptance

- [ ] pluginval strictness ≥ 5
- [ ] Waza eval `openclaw/elite-frameworks/waza/evals/run_all.sh` green
- [ ] EGM duel pass: `python openclaw/elite-frameworks/egm/run_spec_duel.py --spec <this file>`

## Test plan

Unit / offline render / manual DAW steps.

## Rollout

Feature flag, preset migration, operator doc link.