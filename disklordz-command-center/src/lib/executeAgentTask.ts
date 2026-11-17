import { agentApiBase, sendToEliteAgent } from "../data/eliteAgents";
import type { AgentAssignment } from "../data/agentTaskTypes";

function buildTaskMessage(task: AgentAssignment): string {
  const parts = [
    `ASSIGNED TASK: ${task.title}`,
    "",
    task.instructions,
    "",
    "Deliverable: concrete operator output (numbered steps, checklist, or draft text).",
    "If blocked, say what secret or human decision is needed.",
    'End with a line: DONE: <one sentence summary>',
  ];
  if (task.kanbanCardId) {
    parts.splice(2, 0, `Kanban card: ${task.kanbanCardId}`);
  }
  return parts.join("\n");
}

export async function executeAgentTask(
  task: AgentAssignment,
): Promise<{ output: string; prefix: string }> {
  const message = buildTaskMessage(task);
  const base = agentApiBase();

  if (base) {
    const res = await fetch(`${base}/api/tasks/run`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        taskId: task.id,
        title: task.title,
        instructions: task.instructions,
        agentId: task.agentId,
        kanbanCardId: task.kanbanCardId,
      }),
    });
    const data = (await res.json()) as {
      aiResponse?: string;
      output?: string;
      error?: string;
      details?: string;
      agent?: { prefix: string };
    };
    if (!res.ok) {
      throw new Error(data.details || data.error || res.statusText);
    }
    return {
      output: data.output ?? data.aiResponse ?? "",
      prefix: data.agent?.prefix ?? task.agentId.toUpperCase(),
    };
  }

  const { aiResponse, prefix } = await sendToEliteAgent(
    message,
    task.agentId,
  );
  return { output: aiResponse, prefix };
}

export function offlineAgentHint(): string {
  return "Set VITE_AGENT_API_URL to ai-agent-team for assignments from the published app, or use Mobile crew after assigning.";
}