"use client";

import { useState } from "react";

export default function Home() {
  const [log, setLog] = useState<string>("");
  const [busy, setBusy] = useState(false);

  async function bootstrap() {
    setBusy(true);
    setLog("Connecting…");
    try {
      const res = await fetch("/api/bootstrap", { method: "POST" });
      const data = (await res.json()) as { ok?: boolean; message?: string; bot?: string };
      setLog(data.message ?? (data.ok ? `Bot: @${data.bot}` : "Failed"));
    } catch {
      setLog("Request failed");
    } finally {
      setBusy(false);
    }
  }

  return (
    <>
      <h1>DiskLordz Sage</h1>
      <p>
        Uses your saved <code>TELEGRAM_BOT_TOKEN</code> on Cursor hosting. Tap below once after
        publish to register the webhook and bot profile.
      </p>
      <button type="button" onClick={bootstrap} disabled={busy}>
        {busy ? "Working…" : "Connect Telegram bot"}
      </button>
      <pre style={{ marginTop: "1rem", whiteSpace: "pre-wrap" }}>{log}</pre>
      <p style={{ color: "#666", fontSize: 14 }}>
        In Telegram: message the bot <strong>hi</strong>, then try{" "}
        <strong>SAGE: morning brief</strong>. Add <code>GEMINI_API_KEY</code> in Cursor Secrets for
        live model replies; without it you get sample routing text.
      </p>
    </>
  );
}