# MetaGPT blueprint — VST license & sample store portal

Pass this block to MetaGPT **as-is** (or via `scripts/metagpt-license-portal.ps1`).  
After generation, run Strix with `skills/vst_audio_security.md` on the output tree.

**Brand:** DiskLordz · product **Detroit Phonk Trap Maker** · synthesis-only drum kits.  
**Tiers:** See `disklordz-ralph/vst/LICENSING.md` (Lite free, Core ~$44, Suite ~$89).

---

## MetaGPT one-shot prompt

```text
You are building production-oriented specs and scaffold code for DiskLordz, a commercial audio software company.

## Product
- VST3/AU plugin: "Detroit Phonk Trap Maker" (C++20/JUCE — implemented separately, NOT in this repo output)
- Digital goods: 24-bit / 48 kHz mono WAV drum kits (ZIP), royalty-free, synthesis-only
- Payments: Stripe (primary), webhook stubs for Lemon Squeezy and Gumroad

## Deliverables (SOP order)
1. PRD with user stories, tier matrix (Lite/Core/Suite), and non-goals
2. System architecture (diagram in Mermaid): API, DB, object storage, plugin activation flow
3. OpenAPI 3.1 spec for all public and admin routes
4. PostgreSQL schema (customers, orders, license_keys, activations, download_grants, webhook_events)
5. Runnable scaffold:
   - Backend: FastAPI (Python 3.11), async SQLAlchemy, pydantic v2, structured logging, no secrets in repo
   - Frontend: React + Vite, minimal gunmetal (#121216) + amber accent UI matching a hardware booth aesthetic
6. Core features to implement in scaffold (with tests):
   - POST /webhooks/stripe — verify Stripe-Signature, idempotent order fulfillment, persist raw event_id
   - POST /api/v1/licenses/activate — serial + machine_fingerprint, max 3 seats, atomic seat lock (DB transaction)
   - POST /api/v1/licenses/refresh — short-lived signed token for plugin (JWT RS256, public key in plugin later)
   - GET /api/v1/downloads/{grant_id} — presigned R2/S3 URL, one-time or TTL 15 min, bound to order_id
   - Admin read-only: GET /api/v1/orders/summary (no PII in logs by default)
7. Security requirements (must appear in PRD threat model):
   - Webhook signature failure → 401, no side effects
   - IDOR on downloads/orders → 403
   - Rate limits on activate (per IP + per serial)
   - No directory listing on bucket; unpredictable object keys
   - Secrets from environment only (.env.example with names only)
8. Explicitly OUT OF SCOPE for this MetaGPT run:
   - JUCE plugin source, DSP, or pluginval
   - Modifying existing static site at storefront/index.html
   - Production deploy; provide Docker Compose for local dev only

## Quality bar
- Include pytest for webhook signature verification and activation seat-limit concurrency (mock DB ok)
- Include README with env vars, migration command, and "hand to Strix" checklist pointing to vst_audio_security.md themes
- Use deterministic fake data in dev; no real API keys in generated files

Output everything under ./workspace/disklordz-license-portal/
```

---

## Human acceptance checklist (before merge)

- [ ] PRD matches `LICENSING.md` tier names and Lite = no cloud license required  
- [ ] Webhook idempotency table (`webhook_events` unique on provider + event_id)  
- [ ] Activation uses transactions for seat count  
- [ ] Download grants are not guessable integers  
- [ ] `.env.example` only — no leaked keys in generated tree  

---

## Strix handoff (after MetaGPT)

From repo root, once `workspace/` exists on operator machine:

```powershell
strix -n `
  --target C:\Dev\metagpt-workspace\disklordz-license-portal `
  --instruction-file C:\Dev\push\skills\vst_audio_security.md `
  --scan-mode standard
```

Or copy portal into `disklordz-ralph/business/license-portal/` and run:

```powershell
.\scripts\strix-pre-release.ps1 -Quick
```

**Block production** until no verified **high/critical** findings on §1–§3 of `skills/vst_audio_security.md`.

---

## Integration path into this monorepo (optional)

1. MetaGPT generates into `C:\Dev\metagpt-workspace\`  
2. Operator cherry-picks FastAPI app → `disklordz-ralph/business/license-portal/` (future)  
3. Strix + pytest + manual staging Stripe test mode  
4. Cursor wires checkout URLs in `storefront/index.html` (human approves)  
5. Moonbase or custom JWT verify in JUCE **after** Core tier pluginval @ 8  