# Microsoft Agent Framework / AutoGen (opt-in)

**Policy:** Does **not** replace Cursor Cloud Agents, **Ruflo**, or **OpenClaw** on the Lite ship path (`STACK-FROZEN.md`). Operator-only Python on Windows/macOS.

| Stack | When |
|-------|------|
| **Production (2025+)** | [Microsoft Agent Framework](https://learn.microsoft.com/en-us/agent-framework/overview/) — AutoGen + Semantic Kernel unified; native MCP |
| **Maintenance mode** | [AutoGen 0.4+](https://microsoft.github.io/autogen/stable/) — use if you already have scripts / AutoGen Studio |
| **DiskLordz default R&D** | Claude Code (`.claude/agents/`) + Context7 in Cursor + `pluginval` |
| **DiskLordz default ops** | OpenClaw Telegram + AutoGPT kit launch (`openclaw/autogpt/`) |

---

## Business multi-agent map (DiskLordz)

| Agent role | Responsibility | Repo / MCP |
|------------|----------------|------------|
| **Triage / Concierge** | Route VST vs kit vs order questions | Read-only: `storefront/`, `docs/` |
| **Kit & MIDI specialist** | GM / Roland / Alesis mapping, install paths | `docs/dev/juce8-cmake-webview-starter.md`, manuals in RAGFlow |
| **VST engineer** | JUCE 8, CMake, WebView2 | **Context7 MCP** + `disklordz-ralph/vst/` |
| **RT auditor** | No heap/locks in `processBlock` | `.claude/skills/dsp-rt-verification` (mirror in system prompt) |

**Human gates (required):** refunds, custom quotes, live Stripe/Gumroad writes, license issuance.

---

## Install (operator)

```powershell
cd C:\Dev\push
.\scripts\install-ms-agent-framework.ps1
Copy-Item disklordz-ralph\business\ms-agent-framework\.env.example disklordz-ralph\business\ms-agent-framework\.env
# Set OPENAI_API_KEY or Azure OpenAI vars
```

**Greenfield production:** `pip install agent-framework` per Microsoft docs (separate from AutoGen scripts in this repo).

---

## Autonomous VST R&D swarm (3-agent loop)

Round-robin: **DSP_Architect** (Context7 + file write) → **RT_Audio_Auditor** → **Build_QA_Runner** (cmake + pluginval).

```powershell
$env:DLZ_REPO_ROOT = "C:\Dev\push"
$env:OPENAI_API_KEY = "..."
.\scripts\run-vst-rd-swarm.ps1 "Implement velocity-sensitive drive with SmoothedValue"
```

| Safeguard | Behavior |
|-----------|----------|
| Write sandbox | Only `src/`, `ui/`, `CMakeLists.txt` under `DLZ_VST_PROJECT_ROOT` |
| Default target | `vst/templates/drumkit-plugin` (DrumKit Pro; not ship `starter-cpp`) |
| Ship tree | Blocked unless `DLZ_SWARM_ALLOW_SHIP=1` + human review |
| Turn cap | `DLZ_SWARM_MAX_TURNS` (default 15) |
| pluginval | `PLUGINVAL_STRICTNESS` default **8** (set **10** for torture mode) |

**Overnight:** run from Task Scheduler; **commit manually** — swarm does not `git push`.

---

## Run internal R&D team (Context7 + MCP)

```powershell
cd disklordz-ralph\business\ms-agent-framework
$env:DLZ_REPO_ROOT = "C:\Dev\push"
python run_vst_rd_team.py "Map TD-27 rimshot to note 38 and cite JUCE MidiBuffer sample-accurate split"
```

Uses `McpWorkbench` + `StdioServerParams` → `npx -y @upstash/context7-mcp@latest`.

---

## Run customer concierge (no MCP writes)

```powershell
python run_customer_concierge.py "Customer cannot see VST3 in FL Studio after zip install"
```

---

## Framework choice

| Choose | If |
|--------|-----|
| **Agent Framework** | New production workflows, approval gates, MCP + A2A |
| **AutoGen 0.4** | This repo’s starters, AutoGen Studio prototypes |
| **Ruflo + Claude Code** | You already ship with `CLAUDE.md` swarm |
| **OpenClaw** | Telegram ops + cron |

---

## What not to automate

- `evaluate_vst.py` / `evaluate.py` edits  
- MSVC builds without human `pluginval` review  
- Strix auto-fix merges  
- Physical acoustic drum inventory (DiskLordz sells **synthesis** kits + VST)

---

## Related

- `docs/dev/elite-dsp-agent-context7.md`
- `docs/dev/hybrid-agent-stack.md`
- `openclaw/autogpt/README.md`
- `docs/dev/ruflo-opt-in.md`