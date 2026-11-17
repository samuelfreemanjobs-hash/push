import { agentApiBase } from "../data/eliteAgents";
import {
  actionById,
  type OpsAction,
} from "../data/actionCatalog";

export type RunActionResult = {
  ok: boolean;
  message: string;
  /** Navigate UI: open mobile crew with this prompt */
  agentPrefill?: { agentId?: string; message: string };
  /** Kanban card move */
  kanbanMove?: { cardId: string; column: NonNullable<OpsAction["kanbanColumn"]> };
};

async function copyText(text: string): Promise<void> {
  await navigator.clipboard.writeText(text);
}

async function dispatchWorkflow(
  workflowFile: string,
  ref: string,
): Promise<RunActionResult> {
  const base = agentApiBase();
  if (!base) {
    return {
      ok: false,
      message:
        "Set VITE_AGENT_API_URL to your ai-agent-team host for one-click GitHub dispatch (or use Copy CLI).",
    };
  }
  const res = await fetch(`${base}/api/github/workflow-dispatch`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ workflowFile, ref }),
  });
  const data = (await res.json()) as { error?: string; details?: string };
  if (!res.ok) {
    return {
      ok: false,
      message: data.details || data.error || res.statusText,
    };
  }
  return { ok: true, message: `Workflow ${workflowFile} dispatched on ${ref}.` };
}

export async function runOpsAction(actionId: string): Promise<RunActionResult> {
  const action = actionById(actionId);
  if (!action) {
    return { ok: false, message: "Unknown action." };
  }

  switch (action.kind) {
    case "workflow_dispatch": {
      if (!action.workflowFile || !action.workflowRef) {
        return { ok: false, message: "Workflow not configured." };
      }
      return dispatchWorkflow(action.workflowFile, action.workflowRef);
    }
    case "copy_command": {
      if (!action.command) {
        return { ok: false, message: "No command." };
      }
      await copyText(action.command);
      return { ok: true, message: "Command copied to clipboard." };
    }
    case "open_url": {
      if (!action.url) {
        return { ok: false, message: "No URL." };
      }
      window.open(action.url, "_blank", "noopener,noreferrer");
      return { ok: true, message: "Opened in new tab." };
    }
    case "agent_prompt": {
      if (!action.agentMessage) {
        return { ok: false, message: "No agent prompt." };
      }
      return {
        ok: true,
        message: "Opening Mobile crew with prompt.",
        agentPrefill: {
          agentId: action.agentId,
          message: action.agentMessage,
        },
      };
    }
    case "kanban_hint": {
      if (!action.kanbanCardId || !action.kanbanColumn) {
        return { ok: false, message: "Kanban target missing." };
      }
      return {
        ok: true,
        message: `Local board: move ${action.kanbanCardId} → ${action.kanbanColumn}.`,
        kanbanMove: {
          cardId: action.kanbanCardId,
          column: action.kanbanColumn,
        },
      };
    }
    default: {
      const _exhaustive: never = action.kind;
      void _exhaustive;
      return { ok: false, message: "Unsupported action kind." };
    }
  }
}