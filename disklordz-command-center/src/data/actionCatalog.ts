export type ActionKind =
  | "workflow_dispatch"
  | "copy_command"
  | "open_url"
  | "agent_prompt"
  | "kanban_hint";

export type OpsAction = {
  id: string;
  title: string;
  lane: string;
  kind: ActionKind;
  description: string;
  /** Links daily-brief automation ids to runnable actions */
  briefIds?: string[];
  workflowFile?: string;
  workflowRef?: string;
  command?: string;
  url?: string;
  agentId?: string;
  agentMessage?: string;
  kanbanCardId?: string;
  kanbanColumn?: "ready" | "in_progress" | "blocked" | "done" | "backlog";
};

const PUSH_REPO = "samuelfreemanjobs-hash/push";
const VAULT_KANBAN =
  "https://github.com/samuelfreemanjobs-hash/Revenue-Systems-Engineer-Master-Vault/blob/main/agents/disklordz/kanban.yaml";
const OPS_WORKFLOW_URL = `https://github.com/${PUSH_REPO}/actions/workflows/disklordz-ops-sync.yml`;

export const OPS_ACTIONS: OpsAction[] = [
  {
    id: "refresh-ops-snapshot",
    title: "Refresh ops snapshot (GitHub Action)",
    lane: "FLOW",
    kind: "workflow_dispatch",
    description: "Runs vault/Instruments ingest → commits ops-snapshot.json",
    briefIds: [],
    workflowFile: "disklordz-ops-sync.yml",
    workflowRef: "cursor/disklordz-command-center-634f",
  },
  {
    id: "flow-merge-vault-126",
    title: "FLOW: prep merge vault PR #126",
    lane: "FLOW",
    kind: "agent_prompt",
    description: "Delegate conflict resolution + merge checklist for tripwire bundle",
    briefIds: ["merge-126", "merge-126-ready"],
    agentId: "flow",
    agentMessage:
      "FLOW: Review vault PR #126 (tripwire Instruments bundle). List conflict files, merge order, and post-merge INSTRUMENTS_PUSH_TOKEN steps. No auto-merge.",
  },
  {
    id: "drum-tripwire-qc",
    title: "DRUM: founder QC tripwire pack",
    lane: "DRUM",
    kind: "agent_prompt",
    description: "Run tripwire QC checklist and MANIFEST approval steps",
    briefIds: ["tripwire-qc"],
    agentId: "trip",
    agentMessage:
      "TRIP: Walk me through founder ears QC for tripwire-sample-v1 — MANIFEST.yaml fields, package zip gate, deploy blockers.",
  },
  {
    id: "saas-tripwire-env",
    title: "SAAS: tripwire Vercel/Supabase/Stripe gap list",
    lane: "SAAS",
    kind: "agent_prompt",
    description: "Launch gate env audit for live checkout",
    agentId: "plan",
    agentMessage:
      "PLAN: From SAAS-LAUNCH-GAP-AUDIT, list only env vars and deploy steps needed for tripwire checkout on Instruments. P0 order.",
  },
  {
    id: "review-nightly-qa",
    title: "QA: triage Instruments nightly QA",
    lane: "QA",
    kind: "agent_prompt",
    description: "Summarize failing nightly workflow and next fix",
    briefIds: ["nightly-qa"],
    agentId: "review",
    agentMessage:
      "REVIEW: Instruments nightly-qa failed on main. What lanes to check first and smallest safe fix path? No speculative refactors.",
  },
  {
    id: "sync-local-ops",
    title: "Copy: local snapshot sync",
    lane: "FLOW",
    kind: "copy_command",
    description: "Run from push repo with DISKDLORDZ_OPS_TOKEN for live vault",
    command:
      "cd disklordz-command-center && npm run sync:ops && npm run build",
  },
  {
    id: "gh-workflow-run-ops",
    title: "Copy: gh workflow dispatch CLI",
    lane: "FLOW",
    kind: "copy_command",
    description: "Manual trigger without API backend",
    command:
      "gh workflow run disklordz-ops-sync.yml --repo samuelfreemanjobs-hash/push --ref cursor/disklordz-command-center-634f",
  },
  {
    id: "open-ops-workflow-ui",
    title: "Open GitHub Actions (ops sync)",
    lane: "FLOW",
    kind: "open_url",
    description: "Run workflow_dispatch from browser",
    url: OPS_WORKFLOW_URL,
  },
  {
    id: "open-vault-kanban",
    title: "Open vault kanban (source of truth)",
    lane: "PM",
    kind: "open_url",
    description: "Edit kanban.yaml in vault when local overrides are ready to commit",
    url: VAULT_KANBAN,
  },
  {
    id: "sage-daily-standup",
    title: "SAGE: 15-min solo standup script",
    lane: "SAGE",
    kind: "agent_prompt",
    description: "Chief of staff daily ritual from business-os",
    agentId: "sage",
    agentMessage:
      "SAGE: Run solo 15-minute standup — north star paid_tripwire_count_7d, top P0 ready card, one blocked item I can clear today. Bullet output only.",
  },
  {
    id: "code-command-center-patch",
    title: "CODE: patch command center",
    lane: "CODE",
    kind: "agent_prompt",
    description: "Mobile coding lane for this PWA",
    agentId: "code",
    agentMessage:
      "CODE: I need a small React change in disklordz-command-center — suggest file, minimal diff approach.",
  },
  {
    id: "kanban-k012-progress",
    title: "Mark K-012 in progress (local board)",
    lane: "DRUM",
    kind: "kanban_hint",
    description: "Updates column override in this browser (sync vault separately)",
    kanbanCardId: "K-012",
    kanbanColumn: "in_progress",
  },
];

export function actionForBriefId(briefId: string): OpsAction | undefined {
  return OPS_ACTIONS.find((a) => a.briefIds?.includes(briefId));
}

export function actionById(id: string): OpsAction | undefined {
  return OPS_ACTIONS.find((a) => a.id === id);
}