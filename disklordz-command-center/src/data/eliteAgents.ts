export type EliteAgent = {
  id: string;
  prefix: string;
  displayName: string;
  group: "coding" | "ops" | "factory" | "security";
  tier: "elite";
  iAm: string;
  coding?: boolean;
  freeModel: string;
};

/** Free-tier coding + ops specialists — mirrors openclaw/disklordz-elite-team/roster.yaml */
export const ELITE_AGENTS: EliteAgent[] = [
  {
    id: "code",
    prefix: "CODE",
    displayName: "Mobile Coder",
    group: "coding",
    tier: "elite",
    coding: true,
    freeModel: "gemini-2.0-flash",
    iAm: "Phone-first patches, diffs, shell — default when Cursor is down",
  },
  {
    id: "patch",
    prefix: "PATCH",
    displayName: "Patch Surgeon",
    group: "coding",
    tier: "elite",
    coding: true,
    freeModel: "ollama/qwen2.5-coder:7b",
    iAm: "Smallest diff for one bug",
  },
  {
    id: "arch",
    prefix: "ARCH",
    displayName: "Systems Architect",
    group: "coding",
    tier: "elite",
    freeModel: "gemini-2.0-flash",
    iAm: "ADRs, mermaid, JUCE/SaaS topology",
  },
  {
    id: "review",
    prefix: "REVIEW",
    displayName: "Elite Reviewer",
    group: "coding",
    tier: "elite",
    coding: true,
    freeModel: "gemini-2.0-flash",
    iAm: "Pre-merge verdict + security pass",
  },
  {
    id: "sage",
    prefix: "SAGE",
    displayName: "Chief of Staff",
    group: "ops",
    tier: "elite",
    freeModel: "gemini-2.0-flash",
    iAm: "Delegates to specialists; OpenClaw coordinator",
  },
  {
    id: "plan",
    prefix: "PLAN",
    displayName: "Launch Planner",
    group: "ops",
    tier: "elite",
    freeModel: "gemini-2.0-flash",
    iAm: "Gates G1–G6, kanban priority",
  },
  {
    id: "radar",
    prefix: "RADAR",
    displayName: "Tech Intelligence",
    group: "ops",
    tier: "elite",
    freeModel: "gemini-2.0-flash",
    iAm: "AI Dev Signals → six-category screening reports",
  },
  {
    id: "flow",
    prefix: "FLOW",
    displayName: "Workflow Elite",
    group: "ops",
    tier: "elite",
    coding: true,
    freeModel: "gemini-2.0-flash",
    iAm: "GitHub Actions, ops-sync, handoffs",
  },
  {
    id: "forge",
    prefix: "FORGE",
    displayName: "VST Forge",
    group: "factory",
    tier: "elite",
    coding: true,
    freeModel: "gemini-2.0-flash",
    iAm: "JUCE, pluginval, Windows CI",
  },
  {
    id: "trip",
    prefix: "TRIP",
    displayName: "Tripwire Elite",
    group: "factory",
    tier: "elite",
    freeModel: "gemini-2.0-flash",
    iAm: "$9 starter pack QC and naming",
  },
  {
    id: "sentinel",
    prefix: "SENTINEL",
    displayName: "Security Sentinel",
    group: "security",
    tier: "elite",
    freeModel: "gemini-2.0-flash",
    iAm: "PAT scope, secrets, publish-app checklist",
  },
];

export const DEFAULT_MOBILE_AGENT_ID = "code";

const API_BASE = (import.meta.env.VITE_AGENT_API_URL as string | undefined)?.replace(
  /\/$/,
  "",
);

export function agentApiBase(): string | undefined {
  return API_BASE;
}

export async function sendToEliteAgent(
  message: string,
  agentId?: string,
): Promise<{ aiResponse: string; prefix: string }> {
  if (!API_BASE) {
    throw new Error("offline");
  }
  const res = await fetch(`${API_BASE}/api/agent`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      message,
      userId: "disklordz-pwa",
      agentId,
    }),
  });
  const data = (await res.json()) as {
    aiResponse?: string;
    error?: string;
    details?: string;
    agent?: { prefix: string };
  };
  if (!res.ok) {
    throw new Error(data.details || data.error || res.statusText);
  }
  return {
    aiResponse: data.aiResponse ?? "",
    prefix: data.agent?.prefix ?? "CODE",
  };
}