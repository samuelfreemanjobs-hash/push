# MetaGPT — product & web scaffolding (opt-in)

**Policy:** Does **not** replace Cursor/JUCE ship path (`STACK-FROZEN.md`). MetaGPT outputs live **outside** merged product code until a human reviews PRD, architecture, and security (Strix).

[MetaGPT](https://github.com/FoundationAgents/MetaGPT) simulates PM / Architect / Engineer / QA roles and can scaffold greenfield web apps and PRDs from one-line goals.

---

## Where it fits (vs Ruflo)

| Stage | Tool | Delivers |
|-------|------|----------|
| **1. Blueprint & web portal** | **MetaGPT** | PRD, API spec, DB schema, FastAPI/React scaffold (`workspace/`) |
| **2. C++ DSP & WAV kits** | Ruflo + Claude Code | `VoiceDSP.h`, `drum_engine.py`, pluginval, `export_kit.py` |
| **3. Knowledge** | RAGFlow (MCP) | Schematics, EULAs, manuals |
| **4. Promo** | OpenMontage | Shorts / trailers |
| **5. Security & finance** | Strix + OpenBB | Red-team portal; `sales_ledger.csv` |

**Rule:** MetaGPT **never** edits `disklordz-ralph/vst/starter-cpp/Source/` or autoresearch judges. Copy vetted web code in via normal PRs.

---

## Windows setup (operator)

```powershell
pip install --upgrade metagpt
metagpt --init-config
notepad $env:USERPROFILE\.metagpt\config2.yaml   # add LLM API key
```

Generate into a sibling folder (not inside `starter-cpp`):

```powershell
cd C:\Dev
mkdir metagpt-workspace -Force
cd C:\Dev\push
.\scripts\metagpt-license-portal.ps1
```

Default output: `C:\Dev\metagpt-workspace\` (gitignored pattern in `disklordz-ralph/business/metagpt/README.md`).

**Cost / time:** Full MetaGPT runs are **long and token-heavy** — run intentionally, not in CI.

---

## Ready-made blueprint (license + store portal)

Copy-paste prompt + acceptance criteria + Strix handoff:

**`docs/plans/metagpt-license-portal-blueprint.md`**

After generation:

1. Human read PRD + threat model  
2. `strix -n --instruction-file skills/vst_audio_security.md` on generated `workspace/`  
3. Merge only reviewed routes into `business/` or a dedicated repo  

---

## Existing repo assets (do not duplicate blindly)

| Need | Already in push |
|------|-----------------|
| Static marketing | `storefront/index.html` |
| Sales aggregates | `business/openbb/store_backend.py` (read-only metrics) |
| Tiers / DRM plan | `disklordz-ralph/vst/LICENSING.md` |
| Security skill | `skills/vst_audio_security.md` |

MetaGPT should **extend** checkout/license/download — not replace Impeccable storefront without approval.

---

## Optional MetaGPT prompts

| Doc | Use |
|-----|-----|
| `docs/plans/metagpt-license-portal-blueprint.md` | License server + webhooks + presigned kits |
| `docs/plans/metagpt-product-prd-blueprint.md` | VST feature PRD before Core tier |

---

## Related

- Hybrid stack: `docs/dev/hybrid-agent-stack.md`  
- Strix: `docs/dev/strix-opt-in.md`  
- `CLAUDE.md` — optional `product-architect` role