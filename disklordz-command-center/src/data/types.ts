export type KanbanColumnId =
  | "backlog"
  | "ready"
  | "in_progress"
  | "blocked"
  | "done";

export type KanbanCard = {
  id: string;
  column: KanbanColumnId;
  title: string;
  lane: string;
  priority: "P0" | "P1" | "P2";
  owner?: string;
  next?: string;
  blocker?: string;
  link?: string;
  note?: string;
  completed?: string;
};

export type LaunchGate = {
  id: string;
  title: string;
  doc?: string;
  status: "done" | "ready" | "blocked" | "pending";
};

export type AgentLane = {
  id: string;
  prefix: string;
  displayName: string;
  group: string;
  iAm: string;
};

export type VstSku = {
  name: string;
  path: string;
  workflow: string;
  ciStatus: "success" | "failure" | "stale" | "unknown";
  ciDate?: string;
};

export type VaultPr = {
  number: number;
  title: string;
  phase: string;
  mergeable?: string;
  url: string;
};

export type TripwireStep = {
  id: string;
  title: string;
  owner: string;
  status: "done" | "active" | "queued" | "blocked";
  detail?: string;
};

export type RepoLink = {
  label: string;
  href: string;
  role: string;
};