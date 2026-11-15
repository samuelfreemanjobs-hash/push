# Herdr — mission control for parallel agents (opt-in)

[Herdr](https://herdr.dev/) is a Rust terminal multiplexer that keeps **Claude Code, Cursor CLI, Codex, Goose**, etc. in **parallel panes** with optional **herdr-mesh** MCP for pane status and delegation.

DiskLordz does **not** require Herdr on the ship path (`STACK-FROZEN`). Use it on your **Windows workstation** when you want three lanes visible at once without custom Python orchestration.

---

## Fit for DiskLordz

| Pane | Typical agent | Repo entry |
|------|----------------|------------|
| **DSP / JUCE** | Claude Code + Context7 | `vst/templates/drumkit-plugin/`, `good-money-drum-lab/` |
| **Factory** | AutoGen `factory_swarm.py` or `vst_rd_swarm.py` | `factory/audio-factory/`, `ms-agent-framework/` |
| **QA** | `scripts/vst-build-pluginval-gate.ps1` or cmake `validate_vst3` | `vst/starter-cpp/` (ship) or template build dir |

**OSC 7501 status** (`working` / `blocked` / `idle`) surfaces blocked MSVC builds while the sample factory still runs.

**herdr-mesh:** DSP pane can signal preset factory after `DSP_COMPILE_OK` — mirror handoffs in `config/herdr/AGENTS-herdr-coordination.md`.

---

## Windows install (operator)

```powershell
# Herdr
powershell -ExecutionPolicy Bypass -c "irm https://herdr.dev/install.ps1 | iex"

# Optional MCP bridge (Claude Code / compatible clients)
npx -y runchr-works/herdr-mesh install -y
```

---

## One-shot studio boot (repo script)

From repo root after `git pull`:

```powershell
$env:DLZ_REPO_ROOT = "C:\Dev\push"   # your clone
.\scripts\start-herdr-disklordz-studio.ps1
```

Environment overrides:

| Variable | Default |
|----------|---------|
| `DLZ_HERDR_SESSION` | `DiskLordzStudio` |
| `DLZ_VST_PROJECT_ROOT` | `...\vst\templates\drumkit-plugin` |
| `DLZ_AUDIO_FACTORY_ROOT` | `...\factory\audio-factory` |
| `DLZ_HERDR_DSP_AGENT` | `claude` |

---

## Layout file (copy to user config)

Template: `config/herdr/layouts/disklordz-vst-studio.json`

```powershell
New-Item -ItemType Directory -Force "$env:USERPROFILE\.config\herdr-app\layouts"
Copy-Item config\herdr\layouts\disklordz-vst-studio.json `
  "$env:USERPROFILE\.config\herdr-app\layouts\"
```

Import via Herdr UI or CLI per [herdr docs](https://github.com/herdrdev/herdr) (version-specific).

---

## Coordination rules

All agents in a Herdr session must follow:

**`config/herdr/AGENTS-herdr-coordination.md`**

Copy a link from root `AGENTS.md` (section 7) into each pane’s agent instructions.

---

## vs AutoGen / Ruflo

| Tool | Role |
|------|------|
| **Herdr** | Persistent **terminals** + human visibility; mobile approve |
| **factory_swarm / vst_rd_swarm** | **Python** closed loops inside one process |
| **Ruflo** | Claude Code **local** swarm memory |

They compose: Herdr runs three terminals; each terminal may run `python factory_swarm.py` or `claude` separately.

---

## Human gates (unchanged)

Herdr does not bypass:

- Storefront / Stripe publish
- Email send from outreach drafts
- `DLZ_SWARM_ALLOW_SHIP=1` for `starter-cpp`

Blocked panes should print `HUMAN_GATE: <reason>` for mesh consumers.