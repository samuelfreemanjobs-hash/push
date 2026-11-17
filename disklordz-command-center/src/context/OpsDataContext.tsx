import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useState,
  type ReactNode,
} from "react";
import { buildDailyBrief, type DailyBrief } from "../automation/engine";
import { mergeOpsSnapshot } from "../data/mergeSnapshot";
import {
  FALLBACK_SNAPSHOT,
  STATIC_AGENT_LANES,
  STATIC_REPOS,
  type OpsSnapshot,
} from "../data/opsSnapshot";
import type { VaultOpsFragment } from "../lib/connectionsApi";

type OpsDataContextValue = {
  snapshot: OpsSnapshot;
  loading: boolean;
  refresh: () => void;
  mergeLiveSnapshot: (fragment: VaultOpsFragment) => void;
  brief: DailyBrief;
  repos: typeof STATIC_REPOS;
  agentLanes: typeof STATIC_AGENT_LANES;
};

const OpsDataContext = createContext<OpsDataContextValue | null>(null);

async function fetchSnapshot(): Promise<OpsSnapshot> {
  const res = await fetch(`/ops-snapshot.json?ts=${Date.now()}`);
  if (!res.ok) throw new Error(`snapshot ${res.status}`);
  return (await res.json()) as OpsSnapshot;
}

export function OpsDataProvider({ children }: { children: ReactNode }) {
  const [snapshot, setSnapshot] = useState<OpsSnapshot>(FALLBACK_SNAPSHOT);
  const [loading, setLoading] = useState(true);

  const refresh = useCallback(() => {
    setLoading(true);
    void fetchSnapshot()
      .then(setSnapshot)
      .catch(() => setSnapshot(FALLBACK_SNAPSHOT))
      .finally(() => setLoading(false));
  }, []);

  const mergeLiveSnapshot = useCallback((fragment: VaultOpsFragment) => {
    setSnapshot((prev) => mergeOpsSnapshot(prev, fragment));
  }, []);

  useEffect(() => {
    refresh();
  }, [refresh]);

  const brief = useMemo(
    () =>
      buildDailyBrief({
        cards: snapshot.kanban.cards,
        gates: snapshot.launchGates,
        vaultPrs: snapshot.vaultPrs,
        vstFleet: snapshot.vstFleet,
        wipLimit: snapshot.kanban.wipLimit,
      }),
    [snapshot],
  );

  const value = useMemo(
    () => ({
      snapshot,
      loading,
      refresh,
      mergeLiveSnapshot,
      brief,
      repos: STATIC_REPOS,
      agentLanes: STATIC_AGENT_LANES,
    }),
    [snapshot, loading, refresh, mergeLiveSnapshot, brief],
  );

  return (
    <OpsDataContext.Provider value={value}>{children}</OpsDataContext.Provider>
  );
}

export function useOpsData() {
  const ctx = useContext(OpsDataContext);
  if (!ctx) throw new Error("useOpsData must be used within OpsDataProvider");
  return ctx;
}