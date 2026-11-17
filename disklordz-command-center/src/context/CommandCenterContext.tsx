import {
  createContext,
  useCallback,
  useContext,
  useMemo,
  useState,
  type ReactNode,
} from "react";
import type { SectionId } from "../components/Sidebar.types";
import type { AgentAssignment } from "../data/agentTaskTypes";
import { useAgentAssignments } from "../hooks/useAgentAssignments";
import { useTaskQueue } from "../hooks/useTaskQueue";
import {
  executeAgentTask,
  offlineAgentHint,
} from "../lib/executeAgentTask";
import { runOpsAction } from "../lib/runAction";

type AgentPrefill = {
  agentId?: string;
  message: string;
};

type AssignInput = Omit<
  AgentAssignment,
  "id" | "status" | "createdAt"
>;

type CommandCenterContextValue = {
  section: SectionId;
  setSection: (id: SectionId) => void;
  agentPrefill: AgentPrefill | null;
  clearAgentPrefill: () => void;
  kanbanMoveRequest:
    | { cardId: string; column: string; nonce: number }
    | null;
  clearKanbanMoveRequest: () => void;
  triggerAction: (
    actionId: string,
    meta?: { title: string; lane: string },
  ) => Promise<{ ok: boolean; message: string }>;
  tasks: ReturnType<typeof useTaskQueue>["tasks"];
  clearDoneTasks: () => void;
  assignments: AgentAssignment[];
  assignTask: (input: AssignInput, options?: { runNow?: boolean }) => string;
  runAssignment: (id: string) => Promise<void>;
  runAllAssigned: () => Promise<void>;
  removeAssignment: (id: string) => void;
  clearCompletedAssignments: () => void;
  toast: string | null;
  clearToast: () => void;
};

const CommandCenterContext = createContext<CommandCenterContextValue | null>(
  null,
);

export function CommandCenterProvider({ children }: { children: ReactNode }) {
  const [section, setSection] = useState<SectionId>("overview");
  const [agentPrefill, setAgentPrefill] = useState<AgentPrefill | null>(null);
  const [kanbanMoveRequest, setKanbanMoveRequest] = useState<{
    cardId: string;
    column: string;
    nonce: number;
  } | null>(null);
  const [toast, setToast] = useState<string | null>(null);
  const { tasks, enqueue, patch, clearDone } = useTaskQueue();
  const {
    assignments,
    create,
    patch: patchAssignment,
    remove,
    clearCompleted,
  } = useAgentAssignments();

  const runAssignmentFor = useCallback(
    async (task: AgentAssignment) => {
      const id = task.id;
      patchAssignment(id, {
        status: "running",
        startedAt: new Date().toISOString(),
        error: undefined,
      });
      try {
        const { output, prefix } = await executeAgentTask(task);
        patchAssignment(id, {
          status: "completed",
          completedAt: new Date().toISOString(),
          output: `[${prefix}]\n\n${output}`,
        });
        setToast(`Task completed — ${task.title}`);
      } catch (e) {
        const message = e instanceof Error ? e.message : "Run failed";
        const hint = message === "offline" ? offlineAgentHint() : message;
        patchAssignment(id, {
          status: "failed",
          error: hint,
        });
        setToast(hint);
      }
    },
    [patchAssignment],
  );

  const runAssignment = useCallback(
    async (id: string) => {
      const task = assignments.find((a) => a.id === id);
      if (!task) return;
      await runAssignmentFor(task);
    },
    [assignments, runAssignmentFor],
  );

  const assignTask = useCallback(
    (input: AssignInput, options?: { runNow?: boolean }) => {
      const row = create(input);
      setToast(`Assigned to ${input.agentId}: ${input.title}`);
      if (options?.runNow) {
        void runAssignmentFor(row);
      }
      return row.id;
    },
    [create, runAssignmentFor],
  );

  const runAllAssigned = useCallback(async () => {
    for (const a of assignments.filter((x) => x.status === "assigned")) {
      await runAssignment(a.id);
    }
  }, [assignments, runAssignment]);

  const triggerAction = useCallback(
    async (actionId: string, meta?: { title: string; lane: string }) => {
      const taskId = enqueue({
        actionId,
        title: meta?.title ?? actionId,
        lane: meta?.lane ?? "OPS",
      });
      patch(taskId, { status: "running" });
      try {
        const result = await runOpsAction(actionId);
        if (result.agentPrefill) {
          setAgentPrefill(result.agentPrefill);
          setSection("mobile");
        }
        if (result.kanbanMove) {
          setKanbanMoveRequest({
            ...result.kanbanMove,
            nonce: Date.now(),
          });
          setSection("kanban");
        }
        patch(taskId, {
          status: result.ok ? "done" : "failed",
          finishedAt: new Date().toISOString(),
          result: result.message,
        });
        setToast(result.message);
        return { ok: result.ok, message: result.message };
      } catch (e) {
        const message = e instanceof Error ? e.message : "Action failed";
        patch(taskId, {
          status: "failed",
          finishedAt: new Date().toISOString(),
          result: message,
        });
        setToast(message);
        return { ok: false, message };
      }
    },
    [enqueue, patch],
  );

  const value = useMemo(
    () => ({
      section,
      setSection,
      agentPrefill,
      clearAgentPrefill: () => setAgentPrefill(null),
      kanbanMoveRequest,
      clearKanbanMoveRequest: () => setKanbanMoveRequest(null),
      triggerAction,
      tasks,
      clearDoneTasks: clearDone,
      assignments,
      assignTask,
      runAssignment,
      runAllAssigned,
      removeAssignment: remove,
      clearCompletedAssignments: clearCompleted,
      toast,
      clearToast: () => setToast(null),
    }),
    [
      section,
      agentPrefill,
      kanbanMoveRequest,
      triggerAction,
      tasks,
      clearDone,
      assignments,
      assignTask,
      runAssignment,
      runAllAssigned,
      remove,
      clearCompleted,
      toast,
    ],
  );

  return (
    <CommandCenterContext.Provider value={value}>
      {children}
    </CommandCenterContext.Provider>
  );
}

export function useCommandCenter() {
  const ctx = useContext(CommandCenterContext);
  if (!ctx) {
    throw new Error("useCommandCenter must be used within CommandCenterProvider");
  }
  return ctx;
}