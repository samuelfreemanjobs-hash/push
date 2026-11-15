#!/usr/bin/env python3
"""
Local webhook: write winning drum_engine.py from RAGFlow (or any HTTP client).

  pip install flask
  python tools/ragflow_persist_win.py --port 8765

POST /persist  JSON: { "drum_engine_py": "...", "total_loss": 12.34 }
"""

from __future__ import annotations

import argparse
import json
from datetime import datetime, timezone
from pathlib import Path

from flask import Flask, request

ROOT = Path(__file__).resolve().parent.parent
ENGINE = ROOT / "drum_engine.py"
ARCHIVE = ROOT / "outputs" / "ragflow_wins"


def create_app() -> Flask:
    app = Flask(__name__)

    @app.post("/persist")
    def persist():
        data = request.get_json(force=True, silent=True) or {}
        code = data.get("drum_engine_py") or data.get("code")
        if not code or not isinstance(code, str):
            return {"ok": False, "error": "missing drum_engine_py"}, 400

        loss = data.get("total_loss")
        ARCHIVE.mkdir(parents=True, exist_ok=True)
        stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        (ARCHIVE / f"drum_engine_{stamp}.py").write_text(code, encoding="utf-8")
        ENGINE.write_text(code, encoding="utf-8")

        meta = {"total_loss": loss, "saved_at": stamp, "path": str(ENGINE)}
        (ARCHIVE / f"meta_{stamp}.json").write_text(json.dumps(meta, indent=2))
        return {"ok": True, **meta}

    @app.get("/health")
    def health():
        return {"ok": True, "root": str(ROOT)}

    return app


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=8765)
    ap.add_argument("--host", default="127.0.0.1")
    args = ap.parse_args()
    create_app().run(host=args.host, port=args.port, debug=False)


if __name__ == "__main__":
    main()