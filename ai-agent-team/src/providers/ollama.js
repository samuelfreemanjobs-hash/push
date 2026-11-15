/** Optional free local coding model via Ollama */
export async function ollamaGenerate({ baseUrl, model, system, user }) {
  const url = `${baseUrl.replace(/\/$/, "")}/api/chat`;
  const res = await fetch(url, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      model,
      stream: false,
      messages: [
        { role: "system", content: system },
        { role: "user", content: user },
      ],
    }),
  });
  if (!res.ok) {
    const text = await res.text();
    throw new Error(`Ollama ${res.status}: ${text.slice(0, 200)}`);
  }
  const data = await res.json();
  return data.message?.content ?? "";
}

export function shouldUseOllama(agentId) {
  const force = process.env.OLLAMA_FORCE_AGENT_IDS;
  if (!process.env.OLLAMA_BASE_URL || !process.env.OLLAMA_MODEL) return false;
  if (!force) return agentId === "patch";
  return force.split(",").map((s) => s.trim()).includes(agentId);
}