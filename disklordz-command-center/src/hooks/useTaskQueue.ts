import { useCallback, useEffect, useState } from "react";

export type TaskStatus = "queued" | "running" | "done" | "failed";

export type QueuedTask = {
  id: string;
  actionId: string;
  title: string;
  lane: string;
  status: TaskStatus;
  createdAt: string;
  finishedAt?: string;
  result?: string;
};

const STORAGE_KEY = "dlz-command-center-task-queue-v1";

function loadQueue(): QueuedTask[] {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) return [];
    return JSON.parse(raw) as QueuedTask[];
  } catch {
    return [];
  }
}

export function useTaskQueue() {
  const [tasks, setTasks] = useState<QueuedTask[]>(() => loadQueue());

  useEffect(() => {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(tasks));
  }, [tasks]);

  const enqueue = useCallback(
    (input: Omit<QueuedTask, "id" | "status" | "createdAt">) => {
      const task: QueuedTask = {
        ...input,
        id: crypto.randomUUID(),
        status: "queued",
        createdAt: new Date().toISOString(),
      };
      setTasks((prev) => [task, ...prev].slice(0, 50));
      return task.id;
    },
    [],
  );

  const patch = useCallback((id: string, patch: Partial<QueuedTask>) => {
    setTasks((prev) =>
      prev.map((t) => (t.id === id ? { ...t, ...patch } : t)),
    );
  }, []);

  const clearDone = useCallback(() => {
    setTasks((prev) => prev.filter((t) => t.status !== "done"));
  }, []);

  return { tasks, enqueue, patch, clearDone };
}