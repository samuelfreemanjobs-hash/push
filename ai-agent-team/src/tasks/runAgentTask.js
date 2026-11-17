const TASK_SYSTEM_SUFFIX = `
You are executing an operator-assigned task for DiskLordz.
Produce actionable output: steps, checklists, merge plan, or copy-paste text.
Do not claim you merged PRs or ran commands unless the user already did.
End with: DONE: <one sentence summary>
`;

export function formatTaskUserMessage({
  title,
  instructions,
  kanbanCardId,
}) {
  const lines = [
    `ASSIGNED TASK: ${title}`,
    ...(kanbanCardId ? [`Kanban: ${kanbanCardId}`] : []),
    "",
    instructions,
  ];
  return lines.join("\n");
}

export function withTaskSystem(agentSystem) {
  return agentSystem + TASK_SYSTEM_SUFFIX;
}