import { useCallback, useEffect, useState } from "react";
import { useOpsData } from "../context/OpsDataContext";
import type { KanbanCard, KanbanColumnId } from "../data/types";

const STORAGE_KEY = "dlz-command-center-kanban-overrides-v1";

type Overrides = Record<string, KanbanColumnId>;

function loadOverrides(): Overrides {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) return {};
    return JSON.parse(raw) as Overrides;
  } catch {
    return {};
  }
}

function applyOverrides(cards: KanbanCard[], overrides: Overrides): KanbanCard[] {
  return cards.map((c) =>
    overrides[c.id] ? { ...c, column: overrides[c.id] } : c,
  );
}

export function useKanbanStorage() {
  const { snapshot } = useOpsData();
  const baseline = snapshot.kanban.cards;
  const wipLimit = snapshot.kanban.wipLimit;

  const [overrides, setOverrides] = useState<Overrides>(() => loadOverrides());

  useEffect(() => {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(overrides));
  }, [overrides]);

  const cards = applyOverrides(baseline, overrides);

  const moveCard = useCallback(
    (cardId: string, column: KanbanColumnId) => {
      setOverrides((prev) => {
        const merged = applyOverrides(baseline, prev);
        const inProgress = merged.filter((c) => c.column === "in_progress").length;
        const target = merged.find((c) => c.id === cardId);
        if (
          column === "in_progress" &&
          target?.column !== "in_progress" &&
          inProgress >= wipLimit
        ) {
          return prev;
        }
        return { ...prev, [cardId]: column };
      });
    },
    [baseline, wipLimit],
  );

  const resetKanban = useCallback(() => {
    setOverrides({});
    localStorage.removeItem(STORAGE_KEY);
  }, []);

  return { cards, moveCard, resetKanban, wipLimit };
}