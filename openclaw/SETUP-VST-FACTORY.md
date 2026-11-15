# VST Development Factory — VPS activation

Turn on the **seven factory pods** (architect → release) on an OpenClaw host that already has the `push` repo.

Prereqs: same as [SETUP-TELEGRAM.md](./SETUP-TELEGRAM.md) (Node 24+, `DLZ_REPO_ROOT`, `~/.openclaw`).

---

## One-shot (Linux / VPS / WSL)

```bash
cd /path/to/push
export DLZ_REPO_ROOT="$(pwd)"
git pull
bash openclaw/scripts/setup-vst-factory-vps.sh
```

This:

1. Creates `~/.openclaw/vst-architect` … `vst-release` workspaces with `AGENTS.md`
2. Writes `~/.openclaw/disklordz.vst-factory.merged.json5` with your real `skills/shared` path
3. Runs `run_factory_pipeline.sh` (Waza + EGM) as a smoke test

---

## Merge into `openclaw.json`

**You must merge manually** (or via Control UI) so you do not overwrite Telegram / sage settings.

```bash
cp ~/.openclaw/openclaw.json ~/.openclaw/openclaw.json.bak
```

From `disklordz.vst-factory.merged.json5`, paste:

| Section | Purpose |
|---------|---------|
| `skills.load.extraDirs` | Points at `$DLZ_REPO_ROOT/openclaw/skills/shared` |
| `skills.entries` | Enables `egm_spec_challenge`, `waza_agent_eval`, `vst_*` pods |
| `agents.entries["vst-architect"]` … `vst-release` | Seven pod configs |
| `agents.entries.dev.subagents.allowAgents` | `vst-architect`, `vst-dsp`, `vst-gui`, `vst-build`, `review` |
| `automation.jobs` | Optional 06:00 QA pipeline brief |

Then:

```bash
openclaw gateway restart
openclaw channels status --probe
```

---

## Alternative: emit agent JSON only

```bash
cd "$DLZ_REPO_ROOT"
node openclaw/scripts/apply-vst-factory-agents.mjs
```

Paste `agents.entries` from stdout; still set `skills.load.extraDirs` yourself.

---

## Registry reference

- Pod definitions: `openclaw/agents/vst-factory-pods.yaml`
- Elite registry: `openclaw/agents/REGISTRY.yaml` (`dev.subagents`, `factory_pods`)

**`dev` delegates to:** architect, dsp, gui, build (implementation); review/qa for gates; release stays **human**.

---

## Mobile / API (`ai-agent-team`)

Skill hints for API agents:

| API id | OpenClaw skills |
|--------|-----------------|
| `arch` | `vst_architect`, `egm_spec_challenge` |
| `dsp` | `vst_dsp_engineer`, `mcode_vst_dsp` |
| `gui` | `vst_gui_builder`, `agent_desktop` |
| `build` | `vst_build_engineer`, `vst_factory` |
| `preset` | `vst_preset_author`, `tripwire_pack` |
| `qa` | `vst_qa_gate`, `waza_agent_eval` |
| `release` | `vst_release_manager`, `launch_gates` |

Source: `ai-agent-team/src/agents/openclawSkillsMap.js`

---

## Operator cron (repo-level, no OpenClaw)

```bash
# Daily artifacts: demos + kit drop + handoff JSON
bash openclaw/elite-frameworks/automation/factory_finish_ship.sh
```

Add to crontab on VPS:

```cron
0 3 * * * DLZ_REPO_ROOT=/path/to/push bash /path/to/push/openclaw/elite-frameworks/automation/factory_finish_ship.sh >> /var/log/dlz-factory.log 2>&1
```

---

## Verify

```bash
bash openclaw/elite-frameworks/waza/evals/run_all.sh
openclaw agent list   # should show vst-* after merge
```

Telegram → `sage` → `@dev` or delegate: *“Run EGM on gpu-inference README”* → should route `$egm_spec_challenge`.