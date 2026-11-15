import { useCallback, useEffect, useState } from "react";
import type { KanbanCard, KanbanColumnId } from "../data/types";
import { KANBAN_CARDS, KANBAN_WIP } from "../data/seed";

const STORAGE_KEY = "dlz-command-center-kanban-v1";

function loadCards(): KanbanCard[] {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) return KANBAN_CARDS;
    const parsed = JSON.parse(raw) as KanbanCard[];
    const seedIds = new Set(KANBAN_CARDS.map((c) => c.id));
    const merged = [...parsed.filter((c) => seedIds.has(c.id))];
    for (const seed of KANBAN_CARDS) {
      if (!merged.some((c) => c.id === seed.id)) merged.push(seed);
    }
    return merged;
  } catch {
    return KANBAN_CARDS;
  }
}

export function useKanbanStorage() {
  const [cards, setCards] = useState<KanbanCard[]>(() => loadCards());

  useEffect(() => {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(cards));
  }, [cards]);

  const moveCard = useCallback((cardId: string, column: KanbanColumnId) => {
    setCards((prev) => {
      const inProgress = prev.filter((c) => c.column === "in_progress").length;
      const target = prev.find((c) => c.id === cardId);
      if (
        column === "in_progress" &&
        target?.column !== "in_progress" &&
        inProgress >= KANBAN_WIP
      ) {
        return prev;
      }
      return prev.map((c) => (c.id === cardId ? { ...c, column } : c));
    });
  }, []);

  const resetKanban = useCallback(() => {
    setCards(KANBAN_CARDS);
    localStorage.removeItem(STORAGE_KEY);
  }, []);

  return { cards, moveCard, resetKanban, wipLimit: KANBAN_WIP };
}