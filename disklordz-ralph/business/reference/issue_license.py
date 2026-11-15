#!/usr/bin/env python3
"""
Issue a signed license.json envelope for DiskLordz plugins (LicenseVerifier.h).

pip install pynacl

Examples:
  set ED25519_PRIVATE_KEY=<64-hex-seed>
  python issue_license.py DLZ-PRO-2026-0001 <machine_id_blake2b_hex>
  python issue_license.py DLZ-NFR-0001 <machine_id> 14
"""

from __future__ import annotations

import argparse
import json
import os
from datetime import datetime, timedelta, timezone
from pathlib import Path

from sign_license_payload import sign_license_payload


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("serial_key", help="Serial / SKU shown to customer")
    parser.add_argument("machine_id", help="Full Blake2b machine ID from plugin UI")
    parser.add_argument(
        "days_valid",
        nargs="?",
        type=int,
        default=0,
        help="0 = perpetual (expires_at 0); else days from now UTC",
    )
    parser.add_argument(
        "--private-key-hex",
        default=os.environ.get("ED25519_PRIVATE_KEY", "").strip(),
        help="32-byte seed hex (or ED25519_PRIVATE_KEY env)",
    )
    parser.add_argument("-o", "--output", type=Path, help="Write license.json to file")
    args = parser.parse_args()

    if not args.private_key_hex:
        raise SystemExit("Set ED25519_PRIVATE_KEY or pass --private-key-hex")

    expires_at = 0
    if args.days_valid > 0:
        expiry = datetime.now(timezone.utc) + timedelta(days=args.days_valid)
        expires_at = int(expiry.timestamp())

    machine_id = args.machine_id.strip()
    envelope = sign_license_payload(
        args.private_key_hex,
        serial_key=args.serial_key.strip(),
        machine_id=machine_id,
        expires_at=expires_at,
    )
    line = json.dumps(envelope, separators=(",", ":"))

    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(line + "\n", encoding="utf-8")
        print(f"Wrote {args.output}")
    else:
        print("\n=== Paste into plugin ACTIVATE LICENSE field ===\n")
        print(line)


if __name__ == "__main__":
    main()