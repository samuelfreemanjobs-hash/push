# JUCE Ed25519 offline license (Monocypher)

**Lite:** `DLZ_LICENSE_ED25519` **OFF** — plugin always licensed (factory-only).  
**Core+:** `-DDLZ_LICENSE_ED25519=ON` — verify `license.json` on `prepareToPlay` (message thread).

---

## Vendored crypto (zero DLL deps)

| File | Role |
|------|------|
| `Modules/License/crypto/monocypher.c` | Ed25519 + Blake2b |
| `Source/LicenseVerifier.{h,cpp}` | Verify + machine ID |
| `Modules/License/DlzEd25519PublicKey.h` | Replace before release |

Refresh upstream: `scripts/vendor-monocypher.ps1`

---

## On-disk license file

`%AppData%/DiskLordz/license.json` (see `LicenseVerifier::defaultLicenseFile()`):

```json
{
  "payload": "{\"serial_key\":\"DLZ-...\",\"machine_id\":\"<blake2b-hex>\",\"expires_at\":0}",
  "signature_hex": "<128 hex chars, 64-byte Ed25519 sig>"
}
```

Server must sign **exact** `payload` UTF-8 bytes (compact JSON). Python reference:

`disklordz-ralph/business/reference/sign_license_payload.py`

Generate a dev keypair (operator machine):

```powershell
pip install pynacl
python disklordz-ralph/business/reference/generate_ed25519_keypair.py
```

---

## Audio thread

- `std::atomic<bool> isLicensed` updated only from `refreshLicenseFromDisk()`  
- `processBlock`: relaxed load; optional 20s demo dip @ 0.75 gain when unlicensed (Core builds only)  
- **Never** call Monocypher from `processBlock`

---

## CMake

```powershell
cmake -S . -B build -DDLZ_LICENSE_ED25519=ON
```

---

## Strix + MetaGPT

Portal `licensing.py` must use the same `sign_license_payload` contract.  
Scan with `skills/vst_audio_security.md` before production.

Machine ID in API activate request must match `LicenseVerifier::getLocalMachineId()`.