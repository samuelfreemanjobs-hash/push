import { NextRequest, NextResponse } from "next/server";
import { requireIdentity } from "../../../cursor/identity";
import { hasTelegramToken, tgApi } from "@/lib/telegram";
import { webhookSecret } from "@/lib/webhook-secret";

export async function POST(req: NextRequest) {
  const identity = await requireIdentity(req);
  if (identity instanceof Response) {
    return identity;
  }

  if (!hasTelegramToken()) {
    return NextResponse.json({
      ok: false,
      message: "TELEGRAM_BOT_TOKEN missing on host — publish this app and confirm secrets.",
    });
  }

  const origin = req.nextUrl.origin;
  const webhookUrl = `${origin}/api/webhook`;

  try {
    const me = await tgApi<{ username?: string }>("getMe");
    await tgApi("setMyDescription", {
      description:
        "DiskLordz command center — morning brief, dev status, drum kit ops. Powered by OpenClaw Sage.",
    });
    await tgApi("setMyShortDescription", {
      short_description: "OpenClaw sage. Prefixes: SAGE: DEV: MKT: DRUM:",
    });
    await tgApi("setMyCommands", {
      commands: [
        { command: "start", description: "Welcome" },
        { command: "help", description: "Prefix list" },
      ],
    });
    const secret = webhookSecret();
    if (!secret) {
      throw new Error("Could not derive webhook secret");
    }
    await tgApi("setWebhook", {
      url: webhookUrl,
      allowed_updates: ["message"],
      secret_token: secret,
    });

    return NextResponse.json({
      ok: true,
      bot: me.username,
      message: `Webhook set to ${webhookUrl}. Open @${me.username ?? "bot"} in Telegram and send hi.`,
    });
  } catch (e) {
    const message = e instanceof Error ? e.message : "bootstrap failed";
    return NextResponse.json({ ok: false, message });
  }
}