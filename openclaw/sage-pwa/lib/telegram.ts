const token = () => process.env.TELEGRAM_BOT_TOKEN?.trim() ?? "";

export function hasTelegramToken(): boolean {
  return token().length > 0;
}

export async function tgApi<T>(method: string, body?: Record<string, unknown>): Promise<T> {
  const t = token();
  if (!t) {
    throw new Error("TELEGRAM_BOT_TOKEN not set");
  }
  const res = await fetch(`https://api.telegram.org/bot${t}/${method}`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: body ? JSON.stringify(body) : undefined,
  });
  const json = (await res.json()) as { ok: boolean; result?: T; description?: string };
  if (!json.ok) {
    throw new Error(json.description ?? `Telegram ${method} failed`);
  }
  return json.result as T;
}

export async function sendText(chatId: number, text: string): Promise<void> {
  await tgApi("sendMessage", { chat_id: chatId, text });
}