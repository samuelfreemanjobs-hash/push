const STORAGE_KEY = "dlz-agent-api-url-v1";

function normalize(url: string): string {
  return url.trim().replace(/\/$/, "");
}

export function getConfiguredAgentApiUrl(): string | undefined {
  const fromEnv = import.meta.env.VITE_AGENT_API_URL as string | undefined;
  if (fromEnv?.trim()) {
    return normalize(fromEnv);
  }
  try {
    const stored = localStorage.getItem(STORAGE_KEY);
    if (stored?.trim()) return normalize(stored);
  } catch {
    /* ignore */
  }
  return undefined;
}

export function saveAgentApiUrl(url: string): void {
  localStorage.setItem(STORAGE_KEY, normalize(url));
}

export function clearAgentApiUrl(): void {
  localStorage.removeItem(STORAGE_KEY);
}

export const AUTO_VAULT_SYNC_KEY = "dlz-auto-vault-sync-v1";

export function isAutoVaultSyncEnabled(): boolean {
  try {
    const raw = localStorage.getItem(AUTO_VAULT_SYNC_KEY);
    if (raw === null) return true;
    return raw === "1";
  } catch {
    return true;
  }
}

export function setAutoVaultSyncEnabled(on: boolean): void {
  localStorage.setItem(AUTO_VAULT_SYNC_KEY, on ? "1" : "0");
}