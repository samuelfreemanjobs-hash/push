export type AgentAssignmentStatus =
  | "assigned"
  | "running"
  | "completed"
  | "failed";

export type AgentAssignment = {
  id: string;
  title: string;
  instructions: string;
  agentId: string;
  lane?: string;
  kanbanCardId?: string;
  briefActionId?: string;
  status: AgentAssignmentStatus;
  createdAt: string;
  startedAt?: string;
  completedAt?: string;
  /** Agent deliverable (markdown text) */
  output?: string;
  error?: string;
};