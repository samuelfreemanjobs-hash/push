# Strix — security & anti-piracy red team (opt-in)

**Policy:** Does not block Lite VST3 ship (`STACK-FROZEN.md`). **Operator machine only** — Docker + your LLM keys. Never point Strix at production with live customer data without a scoped staging mirror.

[Strix](https://www.strix.ai/) ([usestrix/strix](https://github.com/usestrix/strix)) runs autonomous, sandboxed penetration tests and aims to report issues with reproducible proof-of-concept steps.

---

## Five-pillar company map

| Pillar | Tool | Responsibility |
|--------|------|----------------|
| DSP & samples | Ruflo + Claude Code | `VoiceDSP.h`, `drum_engine.py`, pluginval, `export_kit.py` |
| Knowledge | RAGFlow (MCP) | Schematics, EULAs, manuals |
| Video & social | OpenMontage | Shorts / trailers |
| Finance | OpenBB | `sales_ledger.csv`, Workspace widgets |
| **Security** | **Strix** | License API, checkout webhooks, download URLs, preset/WAV parsers |

Complements **`pluginval`** (host stability) and **`security_sentinel`** (secrets in git) — different layer.

---

## Prerequisites (Windows operator)

- **Docker Desktop** running  
- **Python 3.10+** and [pipx](https://pipx.pypa.io/)  
- LLM key per Strix docs (e.g. `STR_LLM` / provider env — see [Strix repo](https://github.com/usestrix/strix))

```powershell
pipx install strix-agent
```

Verify CLI flags against current Strix README — command names evolve between releases.

---

## What to scan in this repo

| Target | Path / URL | Focus |
|--------|------------|--------|
| **VST source (pre–Core DRM)** | `disklordz-ralph/vst/starter-cpp/Source/` | Preset/state XML, user WAV import, bounds checks |
| **Autoresearch harness** | `disklordz-ralph/vst/autoresearch-cpp/Source/` | Headless render only — not shipped |
| **Store metrics API** | `disklordz-ralph/business/openbb/store_backend.py` | CORS, path traversal, no PII in responses |
| **Static storefront** | `storefront/` | XSS in preview JS, open redirects in checkout placeholders |
| **Staging site** | `https://<your-staging-host>/` | Checkout, signed download links, webhook auth |

**Do not** commit Strix reports containing live tokens, customer emails, or bucket credentials.

---

## Suggested scan cadence

| When | Scan |
|------|------|
| Before **paid tier** launch | Full Strix on staging URL + `starter-cpp/Source` |
| Before **kit ZIP** on R2/S3 | Strix on presigned URL policy + Gumroad/Lemon webhook replay |
| After **license backend** ships | Dedicated playbook §1 in `docs/security/strix-vst-audit-playbook.md` |
| Weekly (optional) | Quick repo scan on `business/openbb` + `storefront` |

---

## Example commands (adapt flags to your Strix version)

From repo root `C:\Dev\push`:

```powershell
# Quick — local source (no network)
strix -n -t .\disklordz-ralph\vst\starter-cpp\Source --scan-mode quick

# Quick — storefront + openbb widget (local files)
strix -n -t .\storefront --scan-mode quick
strix -n -t .\disklordz-ralph\business\openbb --scan-mode quick

# Standard — staging storefront + license API (network; staging only)
strix --target https://staging.yourdisklordz.com --scan-mode standard
```

Use the custom playbook so the agent prioritizes audio-business surfaces:

```powershell
$env:STRIX_EXTRA_CONTEXT = "file:///$PWD/docs/security/strix-vst-audit-playbook.md"
```

(Exact env var name — confirm in Strix docs.)

---

## Helper script

```powershell
.\scripts\strix-pre-release.ps1 -Quick      # starter-cpp + storefront + openbb
.\scripts\strix-pre-release.ps1 -StagingUrl https://staging.example.com
```

---

## Human gates (non-negotiable)

1. **No auto-merge** Strix-suggested C++ or API patches without normal review + `pluginval` + Catch2.  
2. **No production** scan credentials in repo; use staging keys and test SKUs.  
3. **Paid copy & pricing** unchanged by security agents (`AGENTS.md`).  
4. Treat Strix output as **advisory** until you reproduce PoC in your environment.

---

## Related

| Doc | Purpose |
|-----|---------|
| `docs/security/strix-vst-audit-playbook.md` | License API, ZIP links, parser checklist |
| `docs/dev/vst-commercial-gates.md` | pluginval + REA |
| `openclaw/skills/shared/strix_vst_audit/SKILL.md` | OpenClaw / operator skill |
| `disklordz-ralph/vst/LICENSING.md` | Moonbase / Lite gate plan |