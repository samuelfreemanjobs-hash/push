# Agno AgentOS — revenue & custom configurator (opt-in)

## Role in DiskLordz

| Capability | Implementation |
|------------|----------------|
| **Custom kit API** | `custom-kit-architect` + `CustomKitConfigResponse` |
| **Preview WAV** | `audio-factory` `dsp_harness.exe` (Windows) |
| **Knowledge** | `knowledge/compatibility.json` → LanceDb later |
| **Demand forecast** | `drum_qlib_forecaster.py` stub → Qlib when CSV exists |
| **Store writes** | `@tool(requires_confirmation=True)` only |

Does **not** replace:

- `factory_swarm.py` (batch mastering)
- `launch-studio.ps1` (Herdr panes)
- Ship `starter-cpp` builds

## Human gates

| Tool | Behavior |
|------|----------|
| `publish_shopify_flash_promo` | Approve in AgentOS UI |
| `publish_vst_release` | Approve; never auto-promote ship tree |
| `issue_refund` | Approve |

Aligns with `HUMAN_GATE:*` tokens in `drumkit-plugin/AGENTS.md`.

## Frontend

`business/agno-revenue-engine/static/configurator.html` POSTs to:

`http://127.0.0.1:7777/agents/custom-kit-architect/runs`

Embed in storefront only after auth/CORS review (`docs/security/strix-vst-audit-playbook.md`).

## Environment

| Variable | Default |
|----------|---------|
| `DLZ_REPO_ROOT` | git root |
| `DLZ_AUDIO_FACTORY_ROOT` | `factory/audio-factory` |
| `AGNO_PORT` | `7777` |
| `AGNO_MODEL` | `gpt-4o` |

## Nightly “store operator” (realistic split)

Agno **checks** manifest + queues workflows. Heavy work stays on Windows:

1. Herdr `-WatchRawAudio` or cron → `factory_swarm.py`
2. `/disk-marketing-kit` for drafts
3. AgentOS promo card → you **Approve**
4. Manual or gated Shopify GraphQL (future module)

## vs AutoGen

| AutoGen | Agno |
|---------|------|
| Dev-time closed loops | **24/7** scheduler + customer-facing API |
| `factory_swarm` | `custom-kit-architect` + merch workflow |

Compose both; do not duplicate Shopify without one confirmation layer.