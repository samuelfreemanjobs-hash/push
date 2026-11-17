import {
  createContext,
  useCallback,
  useContext,
  useMemo,
  useState,
  type ReactNode,
} from "react";
import type { SectionId } from "../components/Sidebar.types";
import { runOpsAction } from "../lib/runAction";
import { useTaskQueue } from "../hooks/useTaskQueue";

type AgentPrefill = {
  agentId?: string;
  message: string;
};

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