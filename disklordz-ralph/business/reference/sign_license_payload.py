"""
AudioStorePortal license signing — must match Source/LicenseVerifier.cpp (Monocypher Ed25519 / PyNaCl).

pip install pynacl
ED25519_PRIVATE_KEY=hex64... python sign_license_payload.py
"""

from __future__ import annotations

import json
import os
from typing import Any

from nacl.signing import SigningKey


def sign_license_payload(
    private_key_hex: str,
    serial_key: str,
    machine_id: str,
    expires_at: int = 0,
) -> dict[str, Any]:
    signing_key = SigningKey(bytes.fromhex(private_key_hex))
    payload_str = json.dumps(
        {"serial_key": serial_key, "machine_id": machine_id, "expires_at": expires_at},
        separators=(",", ":"),
    )
    signed = signing_key.sign(payload_str.encode("utf-8"))
    return {
        "payload": payload_str,
        "signature_hex": signed.signature.hex(),
    }


if __name__ == "__main__":
    key = os.environ.get("ED25519_PRIVATE_KEY", "").strip()
    if not key:
        raise SystemExit("Set ED25519_PRIVATE_KEY (64-byte secret seed hex)")
    sample = sign_license_payload(
        key,
        serial_key="DLZ-TEST-0001-0002-0003",
        machine_id="replace-with-plugin-machine-id",
        expires_at=0,
    )
    print(json.dumps(sample, indent=2))