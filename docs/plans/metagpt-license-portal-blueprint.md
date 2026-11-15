# MetaGPT blueprint — AudioStorePortal (license + store)

Strix-ready · pass `--instruction-file skills/vst_audio_security.md` after scaffold.

**Tiers:** `disklordz-ralph/vst/LICENSING.md` · **Plugin offline verify:** `docs/dev/juce-ed25519-offline-license.md`

---

## 1. Copy-paste MetaGPT command (PowerShell)

After `pip install --upgrade metagpt` and `metagpt --init-config` + API key:

```powershell
cd C:\Dev\push
$prompt = Get-Content .\docs\plans\metagpt-audiostoreportal-prompt.txt -Raw
metagpt $prompt --project-name AudioStorePortal
```

Or:

```powershell
.\scripts\metagpt-audiostoreportal.ps1
```

---

## 2. Expected output (`workspace/AudioStorePortal/`)

| Area | Contents |
|------|----------|
| **docs/** | PRD, architecture (Mermaid), REST/OpenAPI, DB schema |
| **backend/** | `webhooks.py` (Stripe), `licensing.py` (Ed25519 seats), `downloads.py` (presigned ZIP) |
| **frontend/** | Dark Tailwind storefront, customer dashboard, Web Audio demo player |
| **tests/** | Webhook signature reject, seat concurrency, download IDOR |
| **ops/** | `Dockerfile`, `docker-compose.yml`, `.env.example` |

Tables: `users`, `orders`, `processed_webhooks`, `licenses`, `machine_activations`, `download_grants`.

**Serial format:** `DLZ-XXXX-XXXX-XXXX-XXXX` (not legacy test prefixes).

---

## 3. Strix handoff (staging container)

```powershell
cd C:\Dev\metagpt-workspace\workspace\AudioStorePortal
docker-compose up -d --build

cd C:\Dev\push
strix -n `
  --target C:\Dev\metagpt-workspace\workspace\AudioStorePortal `
  --instruction-file .\skills\vst_audio_security.md `
  --scan-mode standard
```

**Release gate:** no verified **high/critical** on webhook forgery, seat bypass, or unauthenticated ZIP access (`vst-commercial-gates.md` §5).

---

## 4. Merge into DiskLordz monorepo (optional)

Cherry-pick reviewed backend → future `disklordz-ralph/business/license-portal/`.  
Do **not** auto-merge MetaGPT output.  
Wire checkout URLs in `storefront/index.html` after human approval.

---

## 5. C++ plugin side

Server issues Ed25519-signed JSON → plugin verifies offline:

- Header/API: `disklordz-ralph/vst/starter-cpp/Modules/License/DlzOfflineLicense.h`
- Guide + libsodium CMake: `docs/dev/juce-ed25519-offline-license.md`

Lite tier: **no** activation (factory-only). Core+: enable `DLZ_LICENSE_CORE` in CMake when portal is staging-ready.