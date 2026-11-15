import { ELITE_AGENTS, DEFAULT_AGENT_ID } from "./eliteRoster.js";

const PREFIX_RE = /^([A-Z][A-Z0-9]{1,7}):\s*/i;

export function resolveAgent(message, agentId) {
  if (agentId && ELITE_AGENTS[agentId]) {
    const agent = ELITE_AGENTS[agentId];
    const stripped = message.replace(PREFIX_RE, "").trim() || message;
    return { agent, message: stripped, matchedBy: "agentId" };
  }

  const match = message.match(PREFIX_RE);
  if (match) {
    const prefix = match[1].toUpperCase();
    const found = Object.values(ELITE_AGENTS).find((a) => a.prefix === prefix);
    if (found) {
      return {
        agent: found,
        message: message.slice(match[0].length).trim() || message,
        matchedBy: "prefix",
      };
    }
  }

  const fallback = ELITE_AGENTS[DEFAULT_AGENT_ID];
  return { agent: fallback, message, matchedBy: "default" };
}

export function modelForAgent(agent) {
  const envKey = agent.modelEnv || "GEMINI_MODEL";
  return process.env[envKey] || process.env.GEMINI_MODEL || agent.defaultModel;
}