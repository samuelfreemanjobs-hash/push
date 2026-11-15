import { createHash } from "node:crypto";

/** Telegram secret_token (A–Z, a–z, 0–9, _, -); derived from host-only bot token when unset. */
export function webhookSecret(): string | null {
  const explicit = process.env.TELEGRAM_WEBHOOK_SECRET?.trim();
  if (explicit) {
    return explicit;
  }
  const token = process.env.TELEGRAM_BOT_TOKEN?.trim();
  if (!token) {
    return null;
  }
  return createHash("sha256")
    .update(`disklordz-sage-webhook:v1:${token}`)
    .digest("base64url")
    .replace(/[^A-Za-z0-9_-]/g, "_")
    .slice(0, 64);
}

export function verifyWebhookSecretHeader(req: Request): boolean {
  const expected = webhookSecret();
  if (!expected) {
    return false;
  }
  const got = req.headers.get("x-telegram-bot-api-secret-token")?.trim();
  return got === expected;
}