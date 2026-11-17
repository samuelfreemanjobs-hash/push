import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useState,
  type ReactNode,
} from "react";
import {
  getConfiguredAgentApiUrl,
  isAutoVaultSyncEnabled,
  saveAgentApiUrl,
  setAutoVaultSyncEnabled,
} from "../lib/agentApiUrl";
import {
  fetchConnectionStatus,
  probeAgentHealth,
  pullVaultOpsFragment,
  type ConnectionStatus,
} from "../lib/connectionsApi";
import { useOpsData } from "./OpsDataContext";

const VAULT_SYNC_INTERVAL_MS = 10 * 60 * 1000;

type ConnectionsContextValue = {
  agentApiUrl: string | undefined;
  setAgentApiUrl: (url: string) => void;
  autoVaultSync: boolean;
  setAutoVaultSync: (on: boolean) => void;
  status: ConnectionStatus | null;
  probing: boolean;
  lastVaultPullAt: string | null;
  lastVaultError: string | null;
  probeNow: () => Promise<void>;
  pullVaultNow: () => Promise<void>;
};

const ConnectionsContext = createContext<ConnectionsContextValue | null>(null);

export function ConnectionsProvider({ children }: { children: ReactNode }) {
  const { mergeLiveSnapshot } = useOpsData();
  const [agentApiUrl, setAgentApiUrlState] = useState(getConfiguredAgentApiUrl);
  const [autoVaultSync, setAutoVaultSyncState] = useState(isAutoVaultSyncEnabled);
  const [status, setStatus] = useState<ConnectionStatus | null>(null);
  const [probing, setProbing] = useState(false);
  const [lastVaultPullAt, setLastVaultPullAt] = useState<string | null>(null);
  const [lastVaultError, setLastVaultError] = useState<string | null>(null);

  const setAgentApiUrl = useCallback((url: string) => {
    saveAgentApiUrl(url);
    setAgentApiUrlState(getConfiguredAgentApiUrl());
  }, []);

  const setAutoVaultSync = useCallback((on: boolean) => {
    setAutoVaultSyncEnabled(on);
    setAutoVaultSyncState(on);
  }, []);

  const pullVaultNow = useCallback(async () => {
    const base = getConfiguredAgentApiUrl();
    if (!base) {
      setLastVaultError("Agent API URL not set");
      return;
    }
    try {
      const fragment = await pullVaultOpsFragment(base);
      mergeLiveSnapshot(fragment);
      setLastVaultPullAt(fragment.generatedAt);
      setLastVaultError(null);
    } catch (e) {
      setLastVaultError(e instanceof Error ? e.message : String(e));
    }
  }, [mergeLiveSnapshot]);

  const probeNow = useCallback(async () => {
    const base = getConfiguredAgentApiUrl();
    setProbing(true);
    try {
      if (base) {
        const ok = await probeAgentHealth(base);
        if (!ok) {
          setStatus({
            agent: { reachable: false, gemini: false, ollama: false },
            vault: { connected: false, reason: "Agent API unreachable" },
            github: { workflowDispatch: false, reason: "Agent API unreachable" },
            checkedAt: new Date().toISOString(),
          });
          return;
        }
      }
      const next = await fetchConnectionStatus(base);
      setStatus(next);
      if (next.vault.connected && isAutoVaultSyncEnabled()) {
        await pullVaultNow();
      }
    } catch (e) {
      setLastVaultError(e instanceof Error ? e.message : String(e));
    } finally {
      setProbing(false);
    }
  }, [pullVaultNow]);

  useEffect(() => {
    void probeNow();
    const id = window.setInterval(() => {
      void probeNow();
    }, VAULT_SYNC_INTERVAL_MS);
    return () => window.clearInterval(id);
  }, [agentApiUrl, autoVaultSync, probeNow]);

  const value = useMemo(
    () => ({
      agentApiUrl,
      setAgentApiUrl,
      autoVaultSync,
      setAutoVaultSync,
      status,
      probing,
      lastVaultPullAt,
      lastVaultError,
      probeNow,
      pullVaultNow,
    }),
    [
      agentApiUrl,
      setAgentApiUrl,
      autoVaultSync,
      setAutoVaultSync,
      status,
      probing,
      lastVaultPullAt,
      lastVaultError,
      probeNow,
      pullVaultNow,
    ],
  );

  return (
    <ConnectionsContext.Provider value={value}>
      {children}
    </ConnectionsContext.Provider>
  );
}

export function useConnections() {
  const ctx = useContext(ConnectionsContext);
  if (!ctx) {
    throw new Error("useConnections must be used within ConnectionsProvider");
  }
  return ctx;
}
