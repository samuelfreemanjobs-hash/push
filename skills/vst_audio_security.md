# Custom Security Skill: Commercial VST3 Plugin & Digital Sample Storefront

**Product context:** DiskLordz — **Detroit Phonk Trap Maker** (VST3/AU), synthesis-only **24-bit WAV** drum kits.  
**Repo paths:** `disklordz-ralph/vst/starter-cpp/Source/` (ship tree), `business/openbb/` (metrics API), `storefront/` (static marketing).

## Domain Context

You are auditing a commercial audio software and digital sample-pack business. The stack consists of:

1. **C++20 / JUCE VST3/AU plugin (`disklordz-ralph/vst/starter-cpp/Source/`)** — Real-time audio, APVTS preset/state (XML), future user WAV import and cloud license checks (Core+).
2. **License activation & DRM API** — Serial keys, machine/hardware IDs, subscription tokens (Moonbase / custom backend when live).
3. **Digital storefront & asset delivery** — Stripe / Gumroad / Lemon Squeezy webhooks; presigned URLs for kit ZIP archives (R2/S3).

## Priority Attack Vectors & Verification Rules

### 1. License Activation & DRM Bypass

- **Replay & token forgery:** Test whether activation JWTs or license responses can be replayed across different `machine_id` values or modified without invalidating cryptographic signatures.
- **Seat-limit race conditions:** Concurrent activation requests with the same serial key must not exceed max activations (e.g. 2–3 machines); verify atomic DB locking.
- **Offline grace-period manipulation:** Client-side license cache must not use unencrypted timestamps or predictable checksums that allow infinite offline trial resets.
- **Plugin behavior:** Failed or missing license → fail closed (mute/demo); **no network I/O on the audio thread**.

### 2. Unauthenticated Sample Pack & ZIP Enumeration

- **Direct bucket / CDN access:** Probe S3, Cloudflare R2, DigitalOcean Spaces for public listing, predictable keys (`DiskLordz_Autoresearch_Vol1.zip`, `outputs/packs/*.zip`), missing presign expiry or Referer policy.
- **IDOR on order downloads:** `GET /api/downloads/{order_id}` or `/api/receipts/{id}` with sequential or foreign IDs must return **403**, not ZIP bytes.

### 3. Payment Webhook Spoofing

- **Signature verification:** POST webhooks without valid `Stripe-Signature` / Lemon / Gumroad HMAC must **not** mint licenses or fulfill kits.
- **Idempotency replay:** Replaying a valid historical payload must not issue duplicate keys or double-credit accounts.

### 4. C++ JUCE Parser & Memory Safety

- **Malformed preset & WAV parsing:** Audit `setStateInformation`, XML/JSON deserializers, and WAV/SFZ loaders for unbounded allocations, out-of-bounds indexing, integer overflow, or division-by-zero on corrupted user files.
- **Hardcoded secrets:** Scan sources and release binaries for embedded API master keys, webhook secrets, or private signing keys (only **public** verification material may ship in the plugin).

## Proof-of-Concept & Remediation Standards

- Report only vulnerabilities confirmed with a **reproducible** exploit or traced crash inside the Strix sandbox.
- For each confirmed issue, suggest a **minimal** patch (C++20, Python/FastAPI, or Node.js) that preserves real-time audio performance (no locks/alloc in `processBlock`).
- Do not exfiltrate real customer PII or production payment keys during testing.

## Out of Scope

- Distributing cracked binaries, warez forums, or denial-of-service against production payment providers.