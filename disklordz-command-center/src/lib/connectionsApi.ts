import type { OpsSnapshot } from "../data/opsSnapshot";
import { getConfiguredAgentApiUrl } from "./agentApiUrl";

export type ConnectionStatus = {
  agent: { reachable: boolean; gemini: boolean; ollama: boolean };
  vault: { connected: boolean; reason?: string; cardCount?: number };
  github: { workflowDispatch: boolean; reason?: string };
  checkedAt: string;
};

export type VaultOpsFragment = Pick<
  OpsSnapshot,
  "kanban" | "vaultPrs" | "launchGates" | "tripwireSteps" | "syncErrors" | "generatedAt"
>;

export async function fetchConnectionStatus(
  baseUrl?: string,
): Promise<ConnectionStatus> {
  const base = baseUrl ?? getConfiguredAgentApiUrl();
  if (!base) {
    return {
      agent: { reachable: false, gemini: false, ollama: false },
      vault: { connected: false, reason: "Agent API URL not set" },
      github: { workflowDispatch: false, reason: "Agent API URL not set" },
      checkedAt: new Date().toISOString(),
    };
  }
  const res = await fetch(`${base}/api/connections/status`);
  const data = (await res.json()) as ConnectionStatus & { error?: string };
  if (!res.ok) {
    throw new Error(data.error ?? res.statusText);
  }
  return data;
}

export async function pullVaultOpsFragment(
  baseUrl?: string,
): Promise<VaultOpsFragment> {
  const base = baseUrl ?? getConfiguredAgentApiUrl();
  if (!base) throw new Error("Agent API URL not set");
  const res = await fetch(`${base}/api/vault/ops-fragment`);
  const data = (await res.json()) as VaultOpsFragment & {
    error?: string;
    details?: string;
  };
  if (!res.ok) {
    throw new Error(data.details ?? data.error ?? res.statusText);
  }
  return data;
}

export async function probeAgentHealth(baseUrl: string): Promise<boolean> {
  try {
    const res = await fetch(`${normalizeUrl(baseUrl)}/health`, {
      signal: AbortSignal.timeout(8000),
    });
    return res.ok;
  } catch {
    return false;
  }
}

function normalizeUrl(url: string): string {
  return url.replace(/\/$/, "");
}