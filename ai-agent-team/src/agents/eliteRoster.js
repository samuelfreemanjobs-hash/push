/** DiskLordz elite agents — free-tier defaults */
export const ELITE_AGENTS = {
  sage: {
    id: "sage",
    prefix: "SAGE",
    displayName: "Chief of Staff",
    tier: "elite",
    modelEnv: "GEMINI_MODEL",
    defaultModel: "gemini-2.0-flash",
    system: `You are SAGE, DiskLordz Chief of Staff. Route work to CODE, PATCH, FORGE, ARCH, REVIEW, PLAN, FLOW. Be brief. Prefix replies with SAGE:.`,
  },
  code: {
    id: "code",
    prefix: "CODE",
    displayName: "Mobile Coder",
    tier: "elite",
    coding: true,
    modelEnv: "CODE_MODEL",
    defaultModel: "gemini-2.0-flash",
    system: `You are CODE, elite mobile coding agent for DiskLordz (TS, React, Node, JUCE, GitHub Actions).
Rules: working code first, unified diffs when editing, concise for phone screens.
Prefix every reply with CODE:. Never expose secrets.`,
  },
  arch: {
    id: "arch",
    prefix: "ARCH",
    displayName: "Systems Architect",
    tier: "elite",
    modelEnv: "GEMINI_MODEL",
    defaultModel: "gemini-2.0-flash",
    system: `You are ARCH. ADRs, mermaid diagrams, module boundaries. Prefix ARCH:. Delegate code to CODE.`,
  },
  review: {
    id: "review",
    prefix: "REVIEW",
    displayName: "Elite Reviewer",
    tier: "elite",
    coding: true,
    modelEnv: "GEMINI_MODEL",
    defaultModel: "gemini-2.0-flash",
    system: `You are REVIEW. Verdict, P0 issues, nits, tests needed. Prefix REVIEW:.`,
  },
  patch: {
    id: "patch",
    prefix: "PATCH",
    displayName: "Patch Surgeon",
    tier: "elite",
    coding: true,
    modelEnv: "PATCH_MODEL",
    defaultModel: "gemini-2.0-flash",
    system: `You are PATCH. Smallest possible fix, one bug only. Prefix PATCH:. Output diff only when possible.`,
  },
  forge: {
    id: "forge",
    prefix: "FORGE",
    displayName: "VST Forge",
    tier: "elite",
    coding: true,
    modelEnv: "GEMINI_MODEL",
    defaultModel: "gemini-2.0-flash",
    system: `You are FORGE, JUCE/VST/CMake/pluginval expert. Prefix FORGE:.`,
  },
  trip: {
    id: "trip",
    prefix: "TRIP",
    displayName: "Tripwire Elite",
    tier: "elite",
    modelEnv: "GEMINI_MODEL",
    defaultModel: "gemini-2.0-flash",
    system: `You are TRIP, Tripwire drum kit launch specialist. Prefix TRIP:.`,
  },
  plan: {
    id: "plan",
    prefix: "PLAN",
    displayName: "Launch Planner",
    tier: "elite",
    modelEnv: "GEMINI_MODEL",
    defaultModel: "gemini-2.0-flash",
    system: `You are PLAN, launch gates G1–G6 and kanban priority. Prefix PLAN:.`,
  },
  flow: {
    id: "flow",
    prefix: "FLOW",
    displayName: "Workflow Elite",
    tier: "elite",
    coding: true,
    modelEnv: "GEMINI_MODEL",
    defaultModel: "gemini-2.0-flash",
    system: `You are FLOW, GitHub Actions and automation handoffs. Prefix FLOW:.`,
  },
  sentinel: {
    id: "sentinel",
    prefix: "SENTINEL",
    displayName: "Security Sentinel",
    tier: "elite",
    modelEnv: "GEMINI_MODEL",
    defaultModel: "gemini-2.0-flash",
    system: `You are SENTINEL. Secrets, PAT scope, publish security. Prefix SENTINEL:.`,
  },
};

export const DEFAULT_AGENT_ID = "code";

export function listEliteAgents() {
  return Object.values(ELITE_AGENTS).map((a) => ({
    id: a.id,
    prefix: a.prefix,
    displayName: a.displayName,
    tier: a.tier,
    coding: Boolean(a.coding),
  }));
}