import { NextResponse } from "next/server";

/** Wire to Resend / ConvertKit / Lemon Squeezy list — env: LEAD_MAGNET_WEBHOOK_URL */
export async function POST(request: Request) {
  let body: { email?: string };
  try {
    body = (await request.json()) as { email?: string };
  } catch {
    return NextResponse.json({ error: "Invalid JSON" }, { status: 400 });
  }

  const email = body.email?.trim();
  if (!email || !/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(email)) {
    return NextResponse.json({ error: "Valid email required" }, { status: 400 });
  }

  const webhook = process.env.LEAD_MAGNET_WEBHOOK_URL;
  if (webhook) {
    try {
      await fetch(webhook, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ email, source: "free-starter-kit-2026" }),
      });
    } catch (err) {
      console.error("Lead magnet webhook failed", err);
      return NextResponse.json({ error: "Upstream failed" }, { status: 502 });
    }
  }

  const downloadUrl =
    process.env.LEAD_MAGNET_DOWNLOAD_URL ?? "/free-starter-kit/manifest.json";

  return NextResponse.json({ ok: true, downloadUrl });
}