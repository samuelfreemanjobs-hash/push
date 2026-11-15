const PREFIXES = ["SAGE:", "DEV:", "MKT:", "BIZ:", "DRUM:"] as const;

type Prefix = (typeof PREFIXES)[number];

function detectPrefix(text: string): Prefix | null {
  const upper = text.trim().toUpperCase();
  for (const p of PREFIXES) {
    if (upper.startsWith(p)) {
      return p;
    }
  }
  return null;
}

async function geminiReply(prompt: string): Promise<string | null> {
  const key = process.env.GEMINI_API_KEY?.trim();
  if (!key) {
    return null;
  }
  const model = "gemini-2.5-flash";
  const url = `https://generativelanguage.googleapis.com/v1beta/models/${model}:generateContent?key=${encodeURIComponent(key)}`;
  const res = await fetch(url, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      contents: [{ role: "user", parts: [{ text: prompt }] }],
      generationConfig: { maxOutputTokens: 512 },
    }),
  });
  if (!res.ok) {
    return null;
  }
  const data = (await res.json()) as {
    candidates?: { content?: { parts?: { text?: string }[] } }[];
  };
  return data.candidates?.[0]?.content?.parts?.[0]?.text?.trim() ?? null;
}

export async function buildReply(inbound: string): Promise<string> {
  const trimmed = inbound.trim();
  if (!trimmed) {
    return "Send SAGE:, DEV:, MKT:, BIZ:, or DRUM: with your request.";
  }
  if (/^\/start/i.test(trimmed) || /^hi$/i.test(trimmed)) {
    return "DiskLordz Sage online. Prefixes: SAGE: DEV: MKT: BIZ: DRUM:. Example: SAGE: morning brief";
  }

  const prefix = detectPrefix(trimmed);
  const agent = prefix?.replace(":", "") ?? "sage";
  const body = prefix ? trimmed.slice(prefix.length).trim() : trimmed;

  const system = `You are DiskLordz ${agent} (concise Telegram reply, <=12 lines).`;
  const model = await geminiReply(`${system}\n\nUser: ${body}`);
  if (model) {
    return model;
  }

  return `[${agent.toUpperCase()} sample] Received: ${body || "(empty)"}. Add GEMINI_API_KEY in Cursor Secrets for live answers.`;
}