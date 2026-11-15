#!/usr/bin/env python3
"""Generate Ed25519 keypair for DiskLordz license signing (operator machine only)."""

from __future__ import annotations

try:
    from nacl.signing import SigningKey
except ImportError as exc:
    raise SystemExit("pip install pynacl") from exc


def main() -> None:
    sk = SigningKey.generate()
    pk = sk.verify_key
    print("--- DiskLordz Ed25519 (store PRIVATE offline; never commit) ---")
    print(f"ED25519_PRIVATE_KEY={sk.encode().hex()}")
    print(f"PUBLIC_KEY_HEX={pk.encode().hex()}")
    print()
    print("C++: paste PUBLIC 32 bytes into Modules/License/DlzEd25519PublicKey.h")
    print("Sign licenses: ED25519_PRIVATE_KEY=... python sign_license_payload.py")


if __name__ == "__main__":
    main()