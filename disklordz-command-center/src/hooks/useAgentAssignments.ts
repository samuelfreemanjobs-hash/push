import { useCallback, useEffect, useState } from "react";
import type { AgentAssignment } from "../data/agentTaskTypes";

const STORAGE_KEY = "dlz-command-center-agent-assignments-v1";

function loadAssignments(): AgentAssignment[] {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) return [];
    return JSON.parse(raw) as AgentAssignment[];
  } catch {
    return [];
  }
}

export function useAgentAssignments() {
  const [assignments, setAssignments] = useState<AgentAssignment[]>(
    () => loadAssignments(),
  );

  useEffect(() => {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(assignments));
  }, [assignments]);

  const create = useCallback(
    (
      input: Omit<
        AgentAssignment,
        "id" | "status" | "createdAt"
      >,
    ): AgentAssignment => {
      const row: AgentAssignment = {
        ...input,
        id: crypto.randomUUID(),
        status: "assigned",
        createdAt: new Date().toISOString(),
      };
      setAssignments((prev) => [row, ...prev].slice(0, 100));
      return row;
    },
    [],
  );

  const patch = useCallback((id: string, patch: Partial<AgentAssignment>) => {
    setAssignments((prev) =>
      prev.map((a) => (a.id === id ? { ...a, ...patch } : a)),
    );
  }, []);

  const remove = useCallback((id: string) => {
    setAssignments((prev) => prev.filter((a) => a.id !== id));
  }, []);

  const clearCompleted = useCallback(() => {
    setAssignments((prev) =>
      prev.filter((a) => a.status !== "completed"),
    );
  }, []);

  return { assignments, create, patch, remove, clearCompleted };
}