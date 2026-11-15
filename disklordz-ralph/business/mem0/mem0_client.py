"""Thin Mem0 Platform client defaults for DiskLordz (operator machine only)."""

from __future__ import annotations

import os
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from mem0 import MemoryClient

DEFAULT_APP_ID = "disklordz-push"
DEFAULT_USER_ID = "disklordz-operator"
DEFAULT_AGENT_ID = "memory-curator"


def require_api_key() -> str:
    key = os.environ.get("MEM0_API_KEY", "").strip()
    if not key:
        raise SystemExit(
            "MEM0_API_KEY is not set. Get a key at https://app.mem0.ai/dashboard/api-keys "
            "(see docs/dev/mem0-opt-in.md)."
        )
    return key


def scope_ids() -> dict[str, str]:
    return {
        "user_id": os.environ.get("DLZ_MEM0_USER_ID", DEFAULT_USER_ID),
        "agent_id": os.environ.get("DLZ_MEM0_AGENT_ID", DEFAULT_AGENT_ID),
        "app_id": os.environ.get("DLZ_MEM0_APP_ID", DEFAULT_APP_ID),
    }


def get_client() -> MemoryClient:
    from mem0 import MemoryClient

    return MemoryClient(api_key=require_api_key())