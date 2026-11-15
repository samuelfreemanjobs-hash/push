import { NextResponse } from "next/server";
import { buildReply } from "@/lib/sage-reply";
import { rateLimitAllowed } from "@/lib/rate-limit";
import { hasTelegramToken, sendText } from "@/lib/telegram";
import { verifyWebhookSecretHeader } from "@/lib/webhook-secret";

type TelegramUpdate = {
  message?: { chat: { id: number }; text?: string };
};

export async function POST(req: Request) {
  if (!hasTelegramToken()) {
    return NextResponse.json({ ok: false }, { status: 503 });
  }

  if (!verifyWebhookSecretHeader(req)) {
    return NextResponse.json({ ok: false }, { status: 401 });
  }

  let update: TelegramUpdate;
  try {
    update = (await req.json()) as TelegramUpdate;
  } catch {
    return NextResponse.json({ ok: false }, { status: 400 });
  }

  const msg = update.message;
  if (!msg?.text) {
    return NextResponse.json({ ok: true });
  }

  const limitKey = `chat:${msg.chat.id}`;
  if (!rateLimitAllowed(limitKey)) {
    return NextResponse.json({ ok: true });
  }

  const reply = await buildReply(msg.text);
  await sendText(msg.chat.id, reply);
  return NextResponse.json({ ok: true });
}