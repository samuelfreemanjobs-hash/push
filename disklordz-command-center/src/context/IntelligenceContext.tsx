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
  FALLBACK_INTELLIGENCE,
  fetchIntelligenceSnapshot,
  type IntelligenceSnapshot,
} from "../data/intelligenceSnapshot";

type IntelligenceContextValue = {
  intelligence: IntelligenceSnapshot;
  loading: boolean;
  refresh: () => void;
};

const IntelligenceContext = createContext<IntelligenceContextValue | null>(
  null,
);

export function IntelligenceProvider({ children }: { children: ReactNode }) {
  const [intelligence, setIntelligence] = useState<IntelligenceSnapshot>(
    FALLBACK_INTELLIGENCE,
  );
  const [loading, setLoading] = useState(true);

  const refresh = useCallback(() => {
    setLoading(true);
    void fetchIntelligenceSnapshot()
      .then(setIntelligence)
      .catch(() => setIntelligence(FALLBACK_INTELLIGENCE))
      .finally(() => setLoading(false));
  }, []);

  useEffect(() => {
    refresh();
  }, [refresh]);

  const value = useMemo(
    () => ({ intelligence, loading, refresh }),
    [intelligence, loading, refresh],
  );

  return (
    <IntelligenceContext.Provider value={value}>
      {children}
    </IntelligenceContext.Provider>
  );
}

export function useIntelligence() {
  const ctx = useContext(IntelligenceContext);
  if (!ctx) {
    throw new Error("useIntelligence must be used within IntelligenceProvider");
  }
  return ctx;
}