# JUCE Ed25519 offline license verification (Core tier)

Pairs with MetaGPT **AudioStorePortal** (`docs/plans/metagpt-license-portal-blueprint.md`).  
**Lite:** no cloud license — `DLZ_LICENSE_ED25519` off.

---

## Server contract (AudioStorePortal)

`POST /api/license/activate` returns JSON:

```json
{
  "serial_key": "DLZ-AB12-CD34-EF56-GH78",
  "machine_id": "host-fingerprint-sha256-hex",
  "issued_at": 1731600000,
  "expires_at": 1763136000,
  "signature": "<base64 64-byte Ed25519 signature>"
}
```

Backend signs **canonical UTF-8** string (order fixed):

```text
serial_key=<value>
machine_id=<value>
issued_at=<unix>
expires_at=<unix>
```

Embed **only** the 32-byte Ed25519 **public** key in the plugin binary (`DLZ_ED25519_PUBLIC_KEY_HEX`).

---

## Plugin code (this repo)

| File | Role |
|------|------|
| `starter-cpp/Modules/License/DlzOfflineLicense.h` | Parse + verify API |
| `starter-cpp/Modules/License/DlzOfflineLicense.cpp` | JUCE JSON parse; verify stub |

**Message thread only:** HTTP activate → parse → `verifyEd25519Offline` → cache result atomically for audio gate.

---

## Enable libsodium verify (Core builds)

1. Install libsodium (vcpkg: `libsodium` or system package).
2. CMake:

```cmake
option(DLZ_LICENSE_ED25519 "Enable Ed25519 offline license verify" OFF)
if (DLZ_LICENSE_ED25519)
    target_compile_definitions(${TARGET} PRIVATE DLZ_LICENSE_ED25519=1)
    find_package(unofficial-sodium CONFIG) # or PkgConfig
    target_link_libraries(${TARGET} PRIVATE unofficial-sodium::sodium)
endif()
```

3. Replace stub in `DlzOfflineLicense.cpp`:

```cpp
#include <sodium.h>

bool verifyEd25519Offline(const SignedLicensePayload& payload,
                          const std::array<std::uint8_t, 32>& publicKey32)
{
    if (sodium_init() < 0)
        return false;

    const auto message = canonicalSignPayload(payload);
    const juce::MemoryBlock sig = juce::Base64::convertFromBase64(payload.signatureBase64);
    if (sig.getSize() != crypto_sign_BYTES)
        return false;

    return crypto_sign_verify_detached(
        static_cast<const unsigned char*>(sig.getData()),
        reinterpret_cast<const unsigned char*>(message.toRawUTF8()),
        static_cast<unsigned long long>(message.getNumBytesAsUTF8()),
        publicKey32.data()) == 0;
}
```

4. **Audio thread:** read `std::atomic<bool> licensed` — never call sodium from `processBlock`.

---

## Public key at build time

```cpp
// Generated at release: include/generated/DlzPublicKey.inc or env from CMake
constexpr std::array<std::uint8_t, 32> kDlzEd25519PublicKey = { /* from hex */ };
```

Never ship `ED25519_PRIVATE_KEY` in the plugin.

---

## Strix + pytest alignment

Before production portal:

- `skills/vst_audio_security.md` §1 (replay, seat race, offline tamper)
- Portal pytest: concurrent activate, webhook replay, download IDOR

After wiring license gate in `PluginProcessor`:

- pluginval @ strictness **8**
- Strix on `starter-cpp/Source` with malformed preset tests (§4)