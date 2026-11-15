# Strix audit playbook — DiskLordz VST & drum kits

**Primary Strix instruction file:** `skills/vst_audio_security.md` (pass `--instruction-file`).

Use this playbook as **extended context** when running [Strix](https://github.com/usestrix/strix) against DiskLordz staging or local trees.  
Product: **Detroit Phonk Trap Maker** (VST3), **DiskLordz** WAV kits (24-bit), tiers in `disklordz-ralph/vst/LICENSING.md`.

---

## 0. Scope & rules of engagement

- **In scope:** staging hosts, test Stripe/Lemon webhooks, test license keys, operator-owned S3/R2 buckets with dummy kits.  
- **Out of scope:** DDoS, customer production DBs, third-party payment dashboards, cracked-binary distribution forums.  
- **Success:** reproducible finding + severity + minimal fix suggestion; no false positives without PoC.  
- **Audio IP:** C++ under `vst/starter-cpp` may leave the machine only as **signed release binaries** — prefer local `-n` scans on source.

---

## 1. License activation & DRM API (Core+ / Moonbase path)

When a cloud activate/refresh endpoint exists:

| Test | Intent |
|------|--------|
| **IDOR** | Swap `user_id` / `machine_id` in activate body — must not return another user’s license. |
| **Replay** | Replay captured activate response on second machine — must fail or bind to fingerprint. |
| **Token forgery** | Mutate JWT/signed blob (alg none, weak HMAC) — must fail closed; plugin stays muted without crash. |
| **Rate limit** | Brute serial / email — lockout or exponential backoff. |
| **Offline file** | Tamper with offline license file — parser rejects; no RCE in plugin. |
| **Enumeration** | `GET /license/:id` must not leak emails or order metadata without auth. |

**Plugin-side (JUCE):**

- License check on **message thread** or lock-free gate — never block audio with network I/O.  
- Failed verify → silence or demo mode — **no** memory leak on repeated failure.

---

## 2. Storefront, checkout & webhooks

| Test | Intent |
|------|--------|
| **Webhook signature** | Stripe/Lemon/Gumroad POST without valid signature → `401`, no order fulfillment. |
| **Replay webhook** | Same `event_id` twice → idempotent, single fulfillment. |
| **Price tampering** | Client-side SKU/price override → server validates amount vs product catalog. |
| **Open redirect** | `?next=` / success URL params → no arbitrary external redirect. |
| **CSRF** | State-changing operator routes require session + CSRF token. |

**Local FastAPI (`business/openbb/store_backend.py`):**

- Read-only metrics — no admin routes, no filesystem read outside ledger CSV.  
- CORS limited to OpenBB / localhost — not `*`.

---

## 3. Protected WAV / kit ZIP download links

Attack goal: **unpaid access** to `Assets/Releases/*.zip` or R2 `kits/*.zip`.

| Test | Intent |
|------|--------|
| **Direct object URL** | Unauthenticated GET on bucket object → `403` unless public marketing samples only. |
| **Presigned TTL** | Expired presigned URL → fail; no unlimited reuse. |
| **Path traversal** | `../` in download token or filename → rejected. |
| **Token entropy** | Guessable download tokens (`?id=1`) → use high-entropy, single-use or order-bound tokens. |
| **IDOR on download** | Order UUID from another user → `403`. |
| **Hotlinking** | Referer-only bypass on CDN → policy documented. |

**Manifest:** `export_kit.py` outputs are **not** DRM — commercial packs assume checkout-gated ZIP. Strix validates **hosting**, not WAV content.

---

## 4. C++ JUCE — parsers & user content

Prioritize files that touch **untrusted bytes**:

| Surface | Location (ship tree) | Risks |
|---------|---------------------|--------|
| **User WAV import** | `starter-cpp` drag/import paths | Out-of-range samples, huge files, zip bombs |
| **Preset/state** | APVTS XML, custom JSON | XXE (if libxml), buffer overrun on long strings |
| **MIDI drag** | Playlist export (Core+) | Malformed SMF, meta event bombs |
| **`.blob` loader** | Future Core factory audio | Decrypt bounds, allocation on hostile blob |

**Fuzz hints:**

- Truncate WAV headers, wrong `data` chunk size, 32-bit float NaN samples.  
- 10 MB+ single-channel WAV — must reject or stream with cap.  
- Preset XML with 1M nested nodes — must fail parse without stack overflow.

Cross-check with **pluginval** after any parser change.

---

## 5. Reporting template (paste into your tracker)

```markdown
## Strix finding — [title]
- **Asset:** staging URL / file path
- **Severity:** critical | high | medium | low
- **Business impact:** piracy | outage | data leak | crash
- **PoC:** minimal steps
- **Suggested fix:** code/config (operator merges manually)
- **Retest:** command or URL after patch
```

---

## 6. Release gate (paid tier)

Before Core/Suite goes live:

- [ ] `scripts/vst-build-pluginval-gate.ps1` @ strictness 8  
- [ ] Strix staging scan — no **critical** open on §1–§3  
- [ ] Manual: signed download works once, fails when unpaid  
- [ ] Secrets scan — no keys in `storefront` or git (`security_sentinel`)

Lite (free) tier: pluginval + quick Strix on `storefront` sufficient; license API N/A until Moonbase ships.